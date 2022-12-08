#!/usr/bin/python3

# Copyright 2012 Leigh L. Klotz, Jr. WA5ZNU Leigh@WA5ZNU.org
# MIT License: see LICENSE.txt

#Edit by IoT4pi <office@iot4pi.com> under Apache License Version 2.0 (APLv2)
#Upgrade to Python V3.7
import socket
import select
import configparser, os
import time
import datetime
import traceback

import pdb

#Path to config File
strPath="APRS.conf"
strPosPath="Position.conf"
bDebug=True

# Configure this part with your call and APRS-IS passcode
APRS_IS_CALL=""

#APRS_IS_PASSCODE = "99999"		# Edit this to be your passcode
APRS_IS_PASSCODE=""

#Gateway can transmitt
TRANSMIT="TRUE"

#Server-side Filter Commands
FILTER=""

# Configure this part with your location, PHG (antenna description), and brief info
LATITUDE=""
LONGITUDE=""
PHG=""
INFO=""

# You can change APRS_IS_HOST to a server geographically close to you.
# See http://www.aprs2.net/
#http://185.18.149.99:14501/
APRS_IS_HOST_1=""
APRS_IS_PORT_1=0
APRS_IS_HOST_2=""
APRS_IS_PORT_2=0
APRS_IS_HOST_3=""
APRS_IS_PORT_3=0
APRS_IS_HOST_4=""
APRS_IS_PORT_4=0

# Edit these if you need to change where this UDP server listens.
LISTEN_IP=""			# listen on all IP addresses this server has
LISTEN_UDP_PORT=8080		# Listen on UDP port 8080
addr=None                   #Adrress

REPLACE_PATH=False		# If true, add ",qAR,APRS_IS_CALL" to path
#VERSION="LoRa-APRS_iGate"
VERSION=""
udp_sock = None
aprs_is_sock = None

#counter of received packets
numPackets=0
#time of last received packet from aprs Server
LTime=0
#timeout in seconds between new authetication
Auth_Timeout =0
#BME280
BME280=""
#Temperature Huminity and Pressure Storage
TempHumPress=""

def read_config():
    global APRS_IS_CALL
    global APRS_IS_PASSCODE
    global TRANSMIT
    global FILTER
    global LATITUDE
    global LONGITUDE
    global PHG
    global INFO
    global APRS_IS_HOST_1
    global APRS_IS_PORT_1
    global APRS_IS_HOST_2
    global APRS_IS_PORT_2
    global APRS_IS_HOST_3
    global APRS_IS_PORT_3
    global APRS_IS_HOST_4
    global APRS_IS_PORT_4
    global LISTEN_IP
    global LISTEN_UDP_PORT
    global REPLACE_PATH
    global SNR
    global Version
    global Auth_Timeout
    global BME280

    #open parser file
    config = configparser.ConfigParser()

    try:
        config.read(strPath)
    except:
        if bDebug:
            print(getTime()+ ": Error open config file !")
        return False

    pos_config = configparser.ConfigParser()

    try:
        pos_config.read(strPosPath)
    except:
        if bDebug:
            print(getTime()+ ": Error open config file !")
        return False

    #parse SETUP Variales
    try:
        if(config.get("SETUP", "REPLACE_PATH")=='TRUE'):
	        REPLACE_PATH=True
        else:
            REPLACE_PATH=False

        APRS_IS_CALL = config.get("SETUP", "APRS_IS_CALL")
        APRS_IS_PASSCODE = config.get("SETUP", "APRS_IS_PASSCODE")
        TRANSMIT=config.get("SETUP", "TRANSMIT")
        FILTER=config.get("SETUP", "Filter")
        LATITUDE = pos_config.get("SETUP", "LATITUDE")
        LONGITUDE = pos_config.get("SETUP", "LONGITUDE")
        PHG = config.get("SETUP", "PHG")
        INFO = config.get("SETUP", "INFO")
        APRS_IS_HOST_1 = config.get("SETUP", "APRS_IS_HOST_1")
        APRS_IS_PORT_1 = int(config.get("SETUP", "APRS_IS_PORT_1"))
        APRS_IS_HOST_2 = config.get("SETUP", "APRS_IS_HOST_2")
        APRS_IS_PORT_2 = int(config.get("SETUP", "APRS_IS_PORT_2"))
        APRS_IS_HOST_3 = config.get("SETUP", "APRS_IS_HOST_3")
        APRS_IS_PORT_3 = int(config.get("SETUP", "APRS_IS_PORT_3"))
        APRS_IS_HOST_4 = config.get("SETUP", "APRS_IS_HOST_4")
        APRS_IS_PORT_4 = int(config.get("SETUP", "APRS_IS_PORT_4"))
        LISTEN_IP = config.get("SETUP", "LISTEN_IP")
        LISTEN_UDP_PORT = int(config.get("SETUP", "LISTEN_UDP_PORT"))
        Version = config.get("SETUP", "Version")
        Auth_Timeout=int(config.get("SETUP", "Auth_Timeout"))
        BME280=config.get("SETUP", "BME280")
        SNR=config.get("SETUP", "SNR")

    except:
        if bDebug:
            print(getTime()+ ": Error reading SETUP Variable from Config File !")
        return False

    if bDebug:
        print(getTime()+ ": Version = " +Version)
        print(getTime()+ ": APRS_IS_CALL = " +APRS_IS_CALL)
        print(getTime()+ ": APRS_IS_PASSCODE = " +APRS_IS_PASSCODE)
        print(getTime()+ ": TRANSMIT = " +TRANSMIT)
        print(getTime()+ ": FILTER = " +FILTER)
        print(getTime()+ ": LATITUDE = " +LATITUDE)
        print(getTime()+ ": LONGITUDE = " +LONGITUDE)
        print(getTime()+ ": PHG = " +PHG)
        print(getTime()+ ": INFO = " +INFO)
        print(getTime()+ ": APRS_IS_HOST_1 = " +APRS_IS_HOST_1)
        print(getTime()+ ": APRS_IS_PORT_1 = " +str(APRS_IS_PORT_1))
        print(getTime()+ ": APRS_IS_HOST_2 = " +APRS_IS_HOST_2)
        print(getTime()+ ": APRS_IS_PORT_2 = " +str(APRS_IS_PORT_2))
        print(getTime()+ ": APRS_IS_HOST_3 = " +APRS_IS_HOST_3)
        print(getTime()+ ": APRS_IS_PORT_3 = " +str(APRS_IS_PORT_3))
        print(getTime()+ ": APRS_IS_HOST_4 = " +APRS_IS_HOST_4)
        print(getTime()+ ": APRS_IS_PORT_4 = " +str(APRS_IS_PORT_4))
        print(getTime()+ ": LISTEN_IP = " +LISTEN_IP)
        print(getTime()+ ": LISTEN_UDP_PORT = " +str(LISTEN_UDP_PORT))
        print(getTime()+ ": REPLACE_PATH = " +str(REPLACE_PATH))
        print(getTime()+ ": Auth_Timeout = " +str(Auth_Timeout))
        print(getTime()+ ": BME280 = " +BME280)
        print(getTime()+ ": SNR = " +SNR)

    return True

#######################################
#Helper function
# Function to get present Time as String
def getTime():
    ts = time.time()
    strTime = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
    return strTime
###### End Function



def open_connections():
    global udp_sock, aprs_is_sock
    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    udp_sock.bind((LISTEN_IP, LISTEN_UDP_PORT))
    aprs_is_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # TCP
    aprs_is_sock.settimeout( 5.0)
    aprs_is_sock.setsockopt( socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
    try:
        aprs_is_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # TCP
        aprs_is_sock.settimeout( 5.0)
        aprs_is_sock.setsockopt( socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        print(getTime()+ ': Open connection APRS_IS_1 ...')
        aprs_is_sock.connect((APRS_IS_HOST_1, APRS_IS_PORT_1))
        print(getTime()+ ': Connected to %s' % APRS_IS_HOST_1)
        return True
    except socket.timeout:	#no connect
        print(getTime()+ ': APRS_IS_1 timeout')
        traceback.print_exc()
        time.sleep( 5.0)
    except socket.error:
        print(getTime()+ ': APRS_IS_1 error')
        traceback.print_exc()
        time.sleep( 5.0)
    except:
        print(getTime()+ ': APRS_IS_1 other')
        traceback.print_exc()
        time.sleep( 5.0)

    try:
        aprs_is_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # TCP
        aprs_is_sock.settimeout( 5.0)
        aprs_is_sock.setsockopt( socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        print(getTime()+ ': Open connection APRS_IS_2 ...')
        aprs_is_sock.connect((APRS_IS_HOST_2, APRS_IS_PORT_2))
        print(getTime()+ ': Connected to %s' % APRS_IS_HOST_2)
        return True
    except socket.timeout:
        print(getTime()+ ': APRS_IS_2 timeout')
        traceback.print_exc()
        time.sleep( 5.0)
    except socket.error:
        print(getTime()+ ': APRS_IS_2 error')
        traceback.print_exc()
        time.sleep( 5.0)
    except:
        print(getTime()+ ': APRS_IS_2 other')
        traceback.print_exc()
        time.sleep( 5.0)

    try:
        aprs_is_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # TCP
        aprs_is_sock.settimeout( 5.0)
        aprs_is_sock.setsockopt( socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        print(getTime()+ ': Open connection APRS_IS_3 ...')
        aprs_is_sock.connect((APRS_IS_HOST_3, APRS_IS_PORT_3))
        print(getTime()+ ': Connected to %s' % APRS_IS_HOST_3)
        return True
    except socket.timeout:
        print(getTime()+ ': APRS_IS_3 timeout')
        traceback.print_exc()
        time.sleep( 5.0)
    except socket.error:
        print(getTime()+ ': APRS_IS_3 error')
        traceback.print_exc()
        time.sleep( 5.0)
    except:
        print(getTime()+ ': APRS_IS_3 other')
        traceback.print_exc()
        time.sleep( 5.0)

    try:
        aprs_is_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # TCP
        aprs_is_sock.settimeout( 5.0)
        aprs_is_sock.setsockopt( socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        print(getTime()+ ': Open connection APRS_IS_4 ...')
        aprs_is_sock.connect((APRS_IS_HOST_4, APRS_IS_PORT_4))
        print(getTime()+ ': Connected to %s' % APRS_IS_HOST_4)
        return True
    except socket.timeout:
        print(getTime()+ ': APRS_IS_4 timeout')
        traceback.print_exc()
        time.sleep( 5.0)
    except socket.error:
        print(getTime()+ ': APRS_IS_4 error')
        traceback.print_exc()
        time.sleep( 5.0)
    except:
        print(getTime()+ ': APRS_IS_4 other')
        traceback.print_exc()
        time.sleep( 5.0)

    print(getTime()+ ': Could not establish connection!')
    return False


def auth_packet():
    global Version
    global APRS_IS_CALL
    global APRS_IS_PASSCODE
    global FILTER
    packetstr = "user %s pass %s vers %s filter %s\r\n" % (APRS_IS_CALL, APRS_IS_PASSCODE, Version, FILTER)
    packet = packetstr.encode(errors='ignore')
    return(packet)


def position_packet():
    packetstr = "%s>APOTW1,TCPIP*:!%sL%s&%s%s" % (APRS_IS_CALL, LATITUDE.zfill(8), LONGITUDE.zfill(9), PHG, INFO+" "+TempHumPress)
    packet = packetstr.encode(errors='ignore')
    return(packet)


def send_packet(packet):
    global udp_sock, aprs_is_sock
    strpacket = packet.decode(errors='ignore')
    print(getTime()+ ": ->", strpacket)
    message = "%s\r\n" % (strpacket)
    bmessage = message.encode(errors='ignore')
    aprs_is_sock.sendall(bmessage)


def process_packets():
    global udp_sock, aprs_is_sock,numPackets,LTime,addr
    global TempHumPress
	# Await a read event for 5 seconds
    rlist, wlist, elist = select.select([udp_sock, aprs_is_sock], [], [], 5)
    for sock in rlist:
        if sock == udp_sock:
            binpacket, addr = udp_sock.recvfrom(1024) # buffer size is 1024 bytes
            inpacket = binpacket.decode(errors='ignore')
            inpacket = inpacket.strip()
            if (inpacket != ""):
#                print(getTime()+ ': received from APRS GW  packet '+ inpacket + '\n')
#                print(getTime()+ ': RX-UDP %s' % inpacket)
                if(inpacket=="Test"):
                    message=str(numPackets)+";"+getTime()+'\x00\r'
					#print(message)
                    bmessage = message.encode(errors='ignore')
                    udp_sock.sendto(bmessage, addr)
                elif inpacket.startswith("Temp:") :
                    print('Temperature Packet received by APRS GW')
                    TempHumPress=inpacket
                else:
#                    print('APRS GW received packet and sends to APRS Server :')
                    message=str(numPackets)+";"+getTime()+'\x00\r'
					#print(message)
					#udp_sock.sendto(message, addr)
                    binpacket = inpacket.encode(errors='ignore')
                    send_packet(replace_path(inpacket))
                    numPackets=1+numPackets
        elif sock == aprs_is_sock:
                #Meassages from APRS Server
                breply = aprs_is_sock.recv(4096)
                tcpframe = breply.decode(errors='ignore')
                if not tcpframe.startswith('#'):
                  aprs_is_receive = tcpframe.splitlines(True) #if more then one APRS Frame are in a TCP Packet
                  for reply in aprs_is_receive:
                   pos1 = reply.find(':')
                   if pos1 != -1:                  #End of Adress
                    pos2=reply.find('::');        #if a Message Type
                    if pos1 == pos2:
                     try:
                      if reply[pos2+11] == ':':   #check end of Destination
                        print('\n' +getTime()+ ': Message Type received:')
                        print(reply[:-1])
                        temp1=reply.find('>')
                        Call=reply[0:temp1]
                        print('Call %s' % Call)
                        temp2=reply.find('::')
                        if temp2:
                            Via = reply[:temp2]
                            Via = Via[Via.rfind(',')+1:]
                            print('message from %s' % Via)
                            To=reply[temp2+2:temp2+2+9]
                            print('message to %s' % To)
                            Text=reply[temp2+2+9+1:]
                            Text=Text.rstrip()
                            print('message : %s' % Text)
#                            message='M|'+Call+'|'+Via +'|'+To+'|'+Text
                            message='M|%s|%s|%s|%s' % (Call,Via,To,Text)
#                            messageSEND=Call+'>LORA::'+To+':'+Text
                            messageSEND='%s>LORA::%s:%s' % (Call,To,Text)
                            if To == APRS_IS_CALL:  #if Message for me
                                temp3=reply.rfind('{')
                                MesNo=''
                                if temp3>0 :
                                    print('Message to %s with ACK requested !' % To)
                                    MesNo=reply[temp3+1:]
                                    print('message') # ' + MesNo
                                    temp3=message.rfind('{')
                                    message=message[:temp3]
                                    message='%s|A|%s' % (message,MesNo)
                                    ack_message = '1%s' % message
                                    messageSEND = '1%s' % messageSEND
                                    while(len(Call)<9):
                                        Call+=' '
                                    resp=To.strip().upper()+'>APRS::'+Call.upper()+':ack'+MesNo
                                    print('send to APRS Server ',resp)
                                    send_packet(replace_path(resp))
                            else:                         #Message not for me
                                if (TRANSMIT=="TRUE"):    #if TRUE send to radio
#                                    temp4=messageSEND.rfind('{')
                                    if Call != To.rstrip():  #if a Broadcastmessage Call=Destination, then don't send to radio
                                        bmessageSEND = messageSEND.encode(errors='ignore') 
                                        udp_sock.sendto(bmessageSEND, addr)  #send only addressed message to radio (ToDo: better only to heard Stations)
                                        print('send to c++ ',message)
                                        print('messageSEND =', messageSEND)
#                                else:
#                                    print('GW is not allowed to send message!')
                     except:
                      print(getTime()+ ': Not a Message, no ::123456789: in Message %s Len=%d' % (reply, len(reply)))
                LTime=time.time()
                #print  reply


def replace_path(packet):
    if REPLACE_PATH:
        if ":" in packet:
            (path,data) = packet.split(':', 1)
            packet = path + ",qAO," + APRS_IS_CALL + ':' + data
        else:
            print("no : in packet")
        bpacket = packet.encode(errors='ignore')
    return bpacket

def process():
    print(getTime()+ ": Starting Lora_APRS_gateway_7.py")
    if(read_config()):
        if(open_connections()):
            send_packet(auth_packet())
            time.sleep(2)
            send_packet(position_packet())
            LTime=time.time()
            while True:
                process_packets()
                if(time.time()-LTime)>Auth_Timeout:
                    send_packet(position_packet())
                    LTime=time.time()
        else:
            print(getTime()+ ': Restart Service!')
            return False

process()
