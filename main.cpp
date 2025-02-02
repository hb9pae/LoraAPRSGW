/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: paulalexander66
 *
 * Created on 10. Mai 2017, 14:54

# Code modified hb9pae@gmail.com
# 2022-06-15

*/

#include <cstdlib>
#include <stdio.h>
#include <string>
#include <sstream>
#include <time.h>

#include "ParamList.h"
#include "APRS_Connector.h"
#include "HopeRF.h"
#include "HMI.h"
//#include "Temp.h"
extern "C"
{
  #include "TempPressHum.h"
}

using namespace std;

string sVersion="V0.7, hb9pae"; 
string sConfig="./APRS.conf";
string sPosconf="./Position.conf";
string sPythonApp="Lora_APRS_gateway_7.py";
double OLED_timeout =60*5;  //OLED Timeout in seconds
double TempSend=60*5;      //when to send Temp
HopeRF o_HopeRF;
ParamList myList,posList;
APRS_Connector o_APRS;
HMI o_HMI;
string sError="";
string APRS_IS_CALL;
string LATITUDE;
string LONGITUDE;
string PARM,UNIT,EQNS,BITS,SNR,MENUE; //Telemetry settings
string eth0;
string wlan0;
int ConfigPage=1; //which page is showing
int StatisticPage=1; //which page is showing
string showErrorAtStart="TRUE";
int ALTITUDE;

//Hf Parameter
uint32_t RxFrequ;
uint32_t TxFrequ;
int SF;
double BW;
int CR;

//Display
int iWasSleeping=0;
time_t seconds; //Get Starting Time

//index which Menue to show
// 0 = Main
// 1 = Config
// 2 = Statistic
// 3 = Packets 
int Menue=0; //wird von 
//Packet Statistic
int pkt_receive_GW_OK = 0;
int pkt_receive_GW_NOK = 0;
int pkt_send_GW_NOK = 0;
int pkt_send_GW_OK = 0;
int APRS_packet = 0;
string sLTime; //Last Time APRS Server send somthing to GW

//##################################################
//BME280
string strBME280="";
//Temp o_Temp;
//##################################################



//Helper function 
//Int/Doudle to String conversion
string double2string(float iNumber){
    std::ostringstream ss;
    ss << iNumber;
    std::string s(ss.str());
    return s;
}


int SetupGW(){

    
    //###########################
    //Setting up HMI
    if(o_HMI.setupHMI()!=0){
        printf("Error setting up HMI !");
        return 1;
    }
    
    o_HMI.drawWelcome();
    //###########################
    //Getting ans Reading Config File
    //Read Config File 
    if(myList.openConfig(sConfig.c_str())==1){
        printf("Error Reading Config File!");
        sError="Error Reading Conf File!";
        return 1;
    }
    if(posList.openConfig(sPosconf.c_str())==1){
        printf("Error Reading Position Config File!");
        sError="Error Reading Position Conf File!";
        return 1;
    }

    
    APRS_IS_CALL=myList.getValue("APRS_IS_CALL");

    showErrorAtStart=myList.getValue("ShowErrorAtStart");
    LATITUDE=posList.getValue("LATITUDE");
    LONGITUDE=posList.getValue("LONGITUDE");
    if(LONGITUDE.length() == 8) {
        string temp = "0";
        temp.append(LONGITUDE);
        LONGITUDE = temp;
    }

    string Temp_ALT=posList.getValue("ALTITUDE");
    ALTITUDE=atoi(Temp_ALT.c_str());

    PARM=myList.getValue("PARM");
    UNIT=myList.getValue("UNIT");
    EQNS=myList.getValue("EQNS");
    BITS=myList.getValue("BITS");
    SNR=myList.getValue("SNR");
    MENUE=myList.getValue("Display"); Menue=atoi(MENUE.c_str());
    
    //get own IP adresses
    eth0= o_APRS.getIP_eth0();
    wlan0= o_APRS.getIP_wlan0();
    

    //###########################
    //Setting up HopeRF Receiver
    //Reading HopeRF Values
    string Temp_RxFrequ=myList.getValue("RxFrequency");
    string Temp_SF=myList.getValue("SF");
    string Temp_BW=myList.getValue("BW");
    string Temp_CR=myList.getValue("CR");
    string Temp_TxFrequ=myList.getValue("TxFrequency");
    /* 
    printf("RxFrequ %s | ",Temp_RxFrequ.c_str());
    printf("SF %s | ",Temp_SF.c_str());
    printf("BW %s | ",Temp_BW.c_str());
    printf("CR %s | \n",Temp_CR.c_str());
    printf("TxFrequ = %s\n",Temp_TxFrequ.c_str());
    */

    try{
        RxFrequ =strtoul (Temp_RxFrequ.c_str(), NULL, 0);
        TxFrequ =strtoul (Temp_TxFrequ.c_str(), NULL, 0);
        SF=atoi(Temp_SF.c_str());
        BW =atof(Temp_BW.c_str());
        CR=atoi(Temp_CR.c_str());
        int iSNR;
        if(SNR.compare("TRUE")==0) iSNR=1; else iSNR=0;
        printf("Setup Parameter: %u | %u | %d | %f | %d | %d\n",RxFrequ, TxFrequ, SF, BW,CR,iSNR );
        if(o_HopeRF.setParam(RxFrequ,TxFrequ,SF,BW,CR,iSNR)!=0){
            printf("HopeRf could not read Parameter \n");
            sError="HopeRf could not  read Parameter";
            return 1;
        }
    }catch(...){
        printf("Error Reading HopeRF Values !\n");
        sError="Error Reading HopeRF Values";
        return 1;
    }
    
    if(o_HopeRF.setupHopeRF()!=0){
        printf("Error setting up HopeRF !\n");
        sError="Error setting up  HopeRF";
        return 1;
    }
    
    //###########################
    //Setting up APRS Connector
    int iPort=atoi(myList.getValue("LISTEN_UDP_PORT").c_str());
    string APPRS_Host_1=myList.getValue("APRS_IS_HOST_1");
    string APPRS_Host_2=myList.getValue("APRS_IS_HOST_2");
    string APPRS_Host_3=myList.getValue("APRS_IS_HOST_3");
    string APPRS_Host_4=myList.getValue("APRS_IS_HOST_4");
    printf("APRS_IS_HOST_1 is %s \nAPRS_IS_HOST_2 is %s \nAPRS_IS_HOST_3 is %s \nAPRS_IS_HOST_4 is %s \n LISTEN_UDP_PORT is %d \n",APPRS_Host_1.c_str(),APPRS_Host_2.c_str(),APPRS_Host_3.c_str(),APPRS_Host_4.c_str(),iPort);
    //Set the Port of Python App
    o_APRS.setPort(iPort);
    //ping the server  
    /*
    if(o_APRS.APRS_ping(APPRS_Host)!=0){
        printf("APPRS could not be pinged !\n");
        sError="APPRS could not be pinged";
        return 1;
    } */
    printf("Path to python app %s on Port %d \n", sPythonApp.c_str(),iPort);
    //Connect to Server and start the Python App
    if(o_APRS.startAPRS_Connector(sPythonApp.c_str())!=0){
        printf("Error could not start APRS Connector !\n");
        sError="Error could not start APRS Connector";
        return 1;
    }else{
        printf("APRS Connector started\n");
    }

    //################    
    //give app time to start up 
    //and proot if it is still running
    usleep(2500000); //Slow CPU's need Delay for wait to run Python
    //################   
    
    //is python app running
    if(o_APRS.process_activ()!=0){
        printf("APRS Connector not running !\n");
        sError="APRS Connector not running";
        return 1;
    }
    
    //Get first message from UDP Server
    char c_Antw[]="Test";
    int i_AntwLen=strlen(c_Antw);
    if(o_APRS.sendudp(c_Antw,i_AntwLen)!=0){
        printf("Couldn't send to APRS Connector !\n");
        sError="Couldn't send to APRS Connector";
        return 1;

    }
     //Read Msg from UDP Server
    int BufferSize =256;
    char c_Receiv[BufferSize];
    //Somtimes I do not receive packet in first run 
    //so try again
    int iCount =0;
    int iReturn=1;
    while(iCount <150){
        usleep(1000000);
        iReturn=o_APRS.receiveUDP(c_Receiv,BufferSize);
        printf("Return in main from UDP: %d Try# %d:\n",iReturn,iCount+1);
    
            if(iReturn==0){
                string sMessage(c_Receiv);
                printf("Message as string %s \n",sMessage.c_str());
                int iFound =sMessage.find(";");
                if(iFound){
                    string sNumPackets = sMessage.substr(0,iFound);
                    string sLTime=sMessage.substr(iFound+1,sMessage.length());
                    
                    APRS_packet=atoi(sNumPackets.c_str());
                    printf("Nummber of Packets = %d\n",APRS_packet);
                    printf("Time last Info from APRS = %s\n",sLTime.c_str());
                    break;
                    
                }
            }else if(iReturn==1){
                sError="received NULL packet from APRS Connector";
                
                //return 1;
            }else{
                printf("Couldn't receive from APRS Connector !\n");
                sError="Couldn't receive from APRS Connector";
                return 1;
            }
        
        iCount++;            
    }
    
    //##########################
    //BME280
    strBME280=myList.getValue("BME280");
    //printf("BME280 %s\n",strBME280.c_str());
    if (strBME280.compare("TRUE") == 0){
        //if (o_Temp.setupBME280()!=0){
        if (setupBME280()!=0){
            strBME280="FALSE";
            printf("Error BME280 Init!\n");
            sError="Error BME280 Init!";
            printf("BME280 %s\n",strBME280.c_str());
            return 1;
        }
    }
    //printf("Setup finished\n");
    return iReturn;
}

void process_packets(){
	char stat_timestamp[24];
    time_t t;
    string receiveString="";
    //Receive packets and send to APRS
    if(o_HopeRF.receivepacket()==0){
        receiveString=o_HopeRF.getReceivePkt();
        t = time(NULL);
        strftime(stat_timestamp, sizeof stat_timestamp, "Rx: %F %T %Z", gmtime(&t));
        printf("%s %s \n",stat_timestamp, receiveString.c_str());
//        printf("Received %s \n",receiveString.c_str());
        //show in Menue if Packets is active
	o_HMI.setbufferPacket(receiveString.c_str());
        if(Menue==3){
            seconds = time(NULL)+OLED_timeout;
            iWasSleeping=0;
            o_HMI.printPacket(receiveString.c_str());
        }
        //if received, send it to APRS GW
        if(o_APRS.sendudp((char*)receiveString.c_str(), receiveString.length())==0){
            pkt_send_GW_OK++;
            //printf("main send packet OK\n");
            usleep(10000);
            int BufferSize =256;
            char c_Receiv[BufferSize];
            int iReturn=o_APRS.receiveUDP(c_Receiv,BufferSize);
            if(iReturn==0){
                string sMessage(c_Receiv);
                //printf("Message as string %s \n",sMessage.c_str());
                int iFound =sMessage.find(";");
                if(iFound){
                    string sNumPackets = sMessage.substr(0,iFound);
                    sLTime=sMessage.substr(iFound+1,sMessage.length());
                    printf("Nummber of APRS Packets send  = %s\n",sNumPackets.c_str());
                    printf("Time last Info from APRS = %s\n",sLTime.c_str());
                    //TODO
                    //Number Packet & Time
                    APRS_packet=atoi(sNumPackets.c_str()); 
                }
                    pkt_receive_GW_OK++;
                }else{
                    pkt_receive_GW_NOK++;
                }                             
            }else{
                pkt_send_GW_NOK++;
            }//end if sendudp != 0
            
        }//end if receivepacket !=0
}

void showMenue(){
    o_HMI.printMenue(APRS_IS_CALL);
    o_HMI.drawMenue(10,28,"Conf");
    o_HMI.drawMenue(45,26,"Stat");
    o_HMI.drawMenue(78,26,"Pkt");
}

void showConfig(){
    int Line=0;             //running Line Number
    int Secondline=0;       //Line longer then MaxLineLen
    int NumbePages;         //Total Number of Pages      
    int preamble= 2;        //how many Lines before Config
    int NumLinesperPage=3;  //Lines to sho per Page

    //calc how many pages 
    NumbePages =(myList.size()+preamble)/NumLinesperPage;
    int iRest=(myList.size()+preamble)%NumLinesperPage;
    if(iRest)NumbePages++;
    //printf("pages %d | size %d | Line %d \n", NumbePages,myList.size(),Line);

    

    string Result = "";
    o_HMI.showConfig();
    o_HMI.drawMenue(10,28,"Back");
    
    if(ConfigPage >=NumbePages){
        ConfigPage=NumbePages;
        o_HMI.drawTriangle_up();    
    } else if(ConfigPage <= 1){
        ConfigPage=1;
        o_HMI.drawTriangle_down();
    }else{
        o_HMI.drawTriangle_up();
        o_HMI.drawTriangle_down();
    }    

    if(ConfigPage==1){
        if(eth0 ==""){
            //printf("NO eth0 \n");
            o_HMI.showLine(Line, "NO eth0");
            Line++;
        }else{
            //printf("eth0 = %s\n",eth0.c_str());
            Result.append("eth0 ");
            Result.append(eth0);
            o_HMI.showLine(Line,Result.c_str());
            Line++;
        }
   
        if(wlan0 ==""){
            //printf("NO wlan0 \n");
            o_HMI.showLine(Line, "NO wlan0");
            Line++;
        }else{
            //printf("wlan0 = %s\n",wlan0.c_str());
            Result="wlan0 ";
            Result.append(wlan0);
            o_HMI.showLine(Line,Result.c_str());
            Line++;
        }

        Result=LATITUDE;
        Result.append(" ");
        Result.append(LONGITUDE);
        o_HMI.showLine(Line,Result.c_str());
        Line++;
    }
    
    
    for(int i = 0; i< myList.size();i++){
        if(ConfigPage*NumLinesperPage>i+ preamble && (ConfigPage-1)*NumLinesperPage<=i+ preamble){  
            ConfigParam test=myList.getConfigAt(i);
            int LenLine =test.get_Param().length()+test.get_Value().length()+1;
            printf("%s | %s | %d\n",test.get_Param().c_str(),test.get_Value().c_str(),LenLine);
            //calc how many char to cut
            int diff= LenLine -20;
            if(diff >0){
                if(diff < 5) {
                    Result=test.get_Param().substr(0, test.get_Param().length() - diff);
                } else {
                    Result=test.get_Param();
                    Secondline=1;
                }
            }else{
                Result=test.get_Param();
            }
            if(diff <= 20) {
                Result.append(" ");
                Result.append(test.get_Value());
                o_HMI.showLine(Line,Result.c_str());
                Line++;
                if(Secondline){
                    Line++;Secondline=0;
                }
            }
        }//end if
    }//end for   
}

void showStatistic(){
    int Line=0;             //running Line Number
    int NumbePages=5;       //Total Number of Pages
    //int NumLinesperPage=2;  //Lines to sho per Page
    o_HMI.showStatistic();
    o_HMI.drawMenue(10,28,"Back");
    o_HMI.drawMenue(68,35,"Update");
    
    
    if(StatisticPage >=NumbePages){
        StatisticPage=NumbePages;
        o_HMI.drawTriangle_up();    
    } else if(StatisticPage <= 1){
        StatisticPage=1;
        o_HMI.drawTriangle_down();
    }else{
        o_HMI.drawTriangle_up();
        o_HMI.drawTriangle_down();
    }

    if(StatisticPage==1){
        string Result="pkt_rec    =";
        Result.append(double2string((float)o_HopeRF.getNumReceivedPkt()));
        o_HMI.showLine(Line,Result.c_str());
        Line++;
        Result="pkt_rec_OK =";
        Result.append(double2string((float)o_HopeRF.getNumReveivedPktOK()));
        o_HMI.showLine(Line,Result.c_str());
        Line++;
        Result="pkt_rec_NOK=";
        Result.append(double2string((float)o_HopeRF.getNumReveivedPktNOK()));
        o_HMI.showLine(Line,Result.c_str());

    }else if (StatisticPage==2){
        string Result="pkt_trans_GW_OK =";
        Result.append(double2string((float)pkt_send_GW_OK));
        o_HMI.showLine(Line,Result.c_str());
        Line++;
        Result="pkt_trans_GW_NOK=";
        Result.append(double2string((float)pkt_send_GW_NOK));
        o_HMI.showLine(Line,Result.c_str());

    }else if (StatisticPage==3){
        string Result="pkt_rec_GW_OK  =";
        Result.append(double2string((float)pkt_receive_GW_OK));
        o_HMI.showLine(Line,Result.c_str());
        Line++;
        Result="pkt_rec_GW_NOK =";
        Result.append(double2string((float)pkt_receive_GW_NOK));
        o_HMI.showLine(Line,Result.c_str());
    }else if (StatisticPage==4){
        string Result="APRS_packet    = ";
        Result.append(double2string((float)APRS_packet));
        o_HMI.showLine(Line,Result.c_str());
        Line++;
        Result="APRS_IS_HOST";
        o_HMI.showLine(Line,Result.c_str());
        Line++;

        string s_socketip = "./check_APRS_IS_connection";
        const string socketip_Result = o_APRS.exec(s_socketip.c_str());
        if(socketip_Result.length() >= 7) {
            printf("APRS_IS = %s\n", socketip_Result.c_str());
            o_HMI.showLine(Line,socketip_Result.c_str());
        } else {
            printf("APRS_IS = Not Connected\n");
            o_HMI.showLine(Line,"Not Connected");
        }


    }else if (StatisticPage==5){
        char sTemp[100];
        sprintf(sTemp,"Temp:= %0.1fC \n",getTemp());
        o_HMI.showLine(Line,sTemp);
        Line++;
        char sPressure[100];
        sprintf(sPressure,"Pressure:= %0.1fhPA \n",(getPressure()/100));
        o_HMI.showLine(Line,sPressure);
        Line++;
        char sHumidity[100];
        sprintf(sHumidity,"Humidity:= %0.1f%% \n",getHuminity());
        o_HMI.showLine(Line,sHumidity);    
    }
}


void showPackets(){
//    o_HMI.showPackets();
    o_HMI.showbufferPacket();
}

void showHMI(int Button){
    //if(Button != 0){
        switch(Menue){
            case 0:
                // 0 = Main
                // Button 1 = Config
                // Button 2 = Statistic
                // Button 3 = Packets
                if(Button==1){
                    showConfig();
                    Menue=1;
                }else if(Button==2){
                    showStatistic();
                    Menue=2;
                }else if(Button==3){
                    showPackets();
                    Menue=3;
                }else{
                    showMenue();
                }
                break;
            case 1:
                // 1 = Config
                if(Button==1){
                    showMenue();
                    Menue=0;
                }else if(Button ==5){
                    ConfigPage++;
                    showConfig();
                    Menue=1;
                }else if(Button ==4){
                    ConfigPage--;
                    showConfig();
                    Menue=1;
                }else{
                    showConfig();
                    Menue=1;
                }
                break;
            case 2:
                // 2 = Statistic
                if(Button==1){
                    showMenue();
                    Menue=0;
                }else if(Button ==5){
                    StatisticPage++;
                    showStatistic();
                    Menue=2;
                }else if(Button ==4){
                    StatisticPage--;
                    showStatistic();
                    Menue=2;
                }else if(Button ==3){
                    showStatistic();
                    Menue=2;
                }else{
                    showStatistic();
                    Menue=2;
                }
                break;
            case 3:
                // 3 = Packets
                if(Button==3){
                    showMenue();
                    Menue=0;
                }
                else if(Button ==5){
///////////////////////////////////////////
                    //Test CAD
                    ///////////////////////////////////////////
                    printf("Test Procedere !\n");
                    if (o_HopeRF.TxCarrierSense()!=0){
                        printf("Channel activ! Can't transmit !\n");
                        //Channel is not able to send 
                        // so set to Rx Channel again
                        if(o_HopeRF.setupHopeRF()!=0){
                            printf("Error setting up HopeRF !\n");
                            sError="Error setting up  HopeRF";
                        }                  
                   }else{
                        //Configure HopeRf in Mode TX & Send Message
                        //Set to Rx Channel again

                        //type Message without Ack:
                        // <�M|OE1KEB|NINTH|IOT4PI3  |APRS Message3
                        //type Message with Ack
                        // <�M|OE1KEB|NINTH|IOT4PI3  |APRS Message3|A|003
                        /*
                        char m_Msg[100];
                        memset(m_Msg, '\0', sizeof(m_Msg));
                        m_Msg[0]=0x3C; //Pkt Type
                        m_Msg[1]=0xFF; //Destination
                        m_Msg[2]=0x01; //Source
                        */
                        //char temp[]="1M|OE1KEB|NINTH|IOT4PI3  |APRS Message3|A|003";
                        //char temp[]="OE1KEB>LORA::IOT4PI3  :Test internal";
                        char temp[]="IoT4Pi4>APRS:!/6403RXWy>?{Q";
                        //strcat(m_Msg,temp);
                        //printf("Send Packet %s \n",m_Msg); 
                        
                        
                        int Size = strlen(temp);
                        if(o_APRS.proofACK(temp, Size)){
                            int i= o_HopeRF.TXSendPacket(temp,Size,1);
                            if (i!=0) printf("Error sending payload !\n");
                            if(o_HopeRF.setupHopeRF()!=0){
                                printf("Error setting up HopeRF !\n");
                                sError="Error setting up  HopeRF";
                            }
                        
                        }else{

                        
                            int i= o_HopeRF.TXSendPacket(temp,Size);
                            if (i!=0) printf("Error sending payload !\n");
                            if(o_HopeRF.setupHopeRF()!=0){
                                printf("Error setting up HopeRF !\n");
                                sError="Error setting up  HopeRF";
                            }
                        }
                           
                    }
                    /////////////////////////////////////// 
/////////////////////////////////////////////////////////////////                    
                }
                else{
                    showPackets();
                }
                break;
            default:
                break;
        }
    //} //end if
}


int main(int argc, char** argv) {
    int iButton=0;
    int iTempButton=0; //holder of Last pressed Button
    int iTempMenue=0;  //holder of Last pressed Menue
    int not_sent=0;    //Telemetry settings
    int count_tele=0;   //Telemetry Counter
//    int iWasSleeping=0;
    printf("Hallo, iot4pi LoRa APRS Gateway starting...\n");
    printf("Version: %s\n",sVersion.c_str());

    string last_socketip = "";

//    time_t seconds; //Get Starting Time
    time_t watch_stat_sec;    //Refresh Timer f�r Statisticseite 
    time_t watch_python_sec;  //Timer for python check intervall
    time_t watch_restart_sec; //Timer f�r CGI Befehle
    time_t watch_connect_sec; //Timer f�r Verbindungsstatusanzeige
    time_t SecTempSend;
    time_t t;

/* hb9pae no statisticfile
    char sResultc[1024];

    int fdc_out=open("/var/www/html/status.html", O_RDONLY,0644);
    read(fdc_out,sResultc, sizeof(sResultc));
//    fgets(sResultc,1024,fdc_out);
//    printf("%s\n",sResultc);
    close(fdc_out);
    if(strstr(sResultc,"Not") == NULL) {
        int fdc_out=open("/var/www/html/status.html", O_WRONLY|O_CREAT|O_TRUNC,0644);
        strcpy(sResultc,"<HTML><HEAD><title>Lora-APRS I-Gate</title><meta http-equiv=\"refresh\" content=\"60; status.html\"></HEAD><BODY bgcolor=#A0A0A0><h3>Connection</h3> <p style=\"font-family:courier;\">");
        write(fdc_out,sResultc,strlen(sResultc) );
        strcpy(sResultc,"Not Connected</p><a href=\"status.html\">Refresh</a><br><a href=\"readuser.php\">Last Heard User</a><br><a href=\"index.html\">Mainpage</a></BODY></HTML>");
        write(fdc_out,sResultc,strlen(sResultc) );
        close(fdc_out);
    }
*/

    if(SetupGW()!=0){
        printf("Show Error at Startup= %s\n",showErrorAtStart.c_str());
        if(showErrorAtStart.compare("TRUE") == 0){
            printf("Waiting for User input\n");
            o_HMI.printError(sError);
            //################
            //Draw Menue Exit or Resume Buton
            o_HMI.drawMenue(10,28,"Exit");
            o_HMI.drawMenue(70,40,"Resume");
            while(true){
                if(Menue != -1) iButton =o_HMI.readButton();
                if(iButton==1){
                    o_HMI.printGoodby();
                    return 1;
                }else if(iButton==3){
                    break;
                }
            }
        }
        //return 1; nicht abbrechen bei Startup Error
    }
    watch_python_sec=time(NULL)+300;
    watch_restart_sec=time(NULL)+15;
    watch_connect_sec=time(NULL);
    o_HMI.printInfo("LoRa APRS GW         starting....");
    usleep(1000000);
    switch(Menue) {
        case 0:showMenue();break;
        case 1:showConfig();break;
        case 2:showStatistic();break;
        case 3:showPackets();break;
        default:showMenue();
    }
    iTempMenue=Menue;
    seconds = time(NULL)+OLED_timeout;
    SecTempSend= time(NULL)+TempSend;
    //Buffer for receiving
    //from UDP
    int iReturn,onetimerun=0;
    int BufferSize =4096;
    char c_Receiv[BufferSize];
    char c_Framebuf[260];
    char stat_timestamp[24];
  
   
    while(1){
        process_packets();
	usleep(1000000);
        if(Menue != -1) iButton =o_HMI.readButton();
        /////////////////////////////////////////
        //Read UDP if there are messages form APRS Server
        memset(c_Receiv, '\0', sizeof(c_Receiv));
        iReturn=o_APRS.receiveUDP(c_Receiv,BufferSize);
        if(iReturn==0){
            string sMessage(c_Receiv);
            printf("Received from UDP %d %s\n",sMessage.length(),sMessage.c_str());
            
            int x=0,y=0;
            for(;;) {
                while(c_Receiv[x] != 0 && c_Receiv[x] != '\r' && c_Receiv[x] != '\n') {
                    c_Framebuf[y]=c_Receiv[x];
                    x++;y++;
                    c_Framebuf[y]=0;
                }
                //printf("Framelen %d %d\n",x,y);
                if(y != 0 && y <= 256){  // Lora can max 256 Byte sending, max Message Length = 253 + 3 Byte Header
            
                    //Test CAD
                    ///////////////////////////////////////////
                    if (o_HopeRF.TxCarrierSense()!=0){
                        printf("Channel activ! Can't transmit !\n");
                        //Channel is not able to send 
                        // so set to Rx Channel again
                        if(o_HopeRF.setupHopeRF()!=0){
                            printf("Error setting up HopeRF !\n");
                            sError="Error setting up  HopeRF";
                        }                  
                   }else{
                        //printf("Framesend\n");
                        if(o_APRS.proofACK(c_Framebuf, strlen(c_Framebuf))){
                            //With ACK
                            //Configure HopeRf in Mode TX & Send Message
                            //Set to Rx Channel again
                            int ilen=strlen(c_Framebuf);
                            printf("Sending paket with Ack %s \n",c_Framebuf);
                            int i= o_HopeRF.TXSendPacket(c_Framebuf,ilen,1);
                            if (i!=0) printf("Error sending payload !\n");
                            if(o_HopeRF.setupHopeRF()!=0){
                                printf("Error setting up HopeRF !\n");
                                sError="Error setting up  HopeRF";
                            }

                        }else{
                            //No ACK
                            //Configure HopeRf in Mode TX & Send Message
                            //Set to Rx Channel again
                            int ilen=strlen(c_Framebuf);
                            printf("Sending paket without Ack %s \n",c_Framebuf);
                            int i= o_HopeRF.TXSendPacket(c_Framebuf,ilen);
                            if (i!=0) printf("Error sending payload !\n");
                            if(o_HopeRF.setupHopeRF()!=0){
                                printf("Error setting up HopeRF !\n");
                                sError="Error setting up  HopeRF";
                            }
                        }   
                    }//end If CarrierSense
            /////////////////////////////////////// 
                } //End if < 265
                y=0;
                if(c_Receiv[x] == 0) break;
                x++;
            } //End For
        
        }//end if UDP Received

        ////////////////////////////////
        if(iButton!=0){
            //Read Time Button is pressed for 
            //OLED Timeout
            seconds = time(NULL)+OLED_timeout;
            if(iWasSleeping==1){
                //if awake from sleeping 
                //show last menue
                printf("Calling Menue with Menue: %d\n",iTempMenue);
                printf("Calling Menue with Button: %d\n",iTempButton);
                showHMI(iTempButton);
            }else{
                showHMI(iButton);
                iTempMenue=Menue;  //storing Menue temp
                printf("Saving Menue: %d\n",Menue);
                if(iButton>3 && iButton<=5){               
                    iTempButton=0;
                    printf("Saving Button: %d\n",iTempButton);
                    
                }
            }
            iWasSleeping=0;
        }else{ 
            if(seconds<=time(NULL) && !iWasSleeping){
                o_HMI.clearDisplay();
                iWasSleeping=1;
                //printf("Timer elapsed...\n");
            }
        }

        if(Menue==2) {
            if(watch_stat_sec<=time(NULL)){
                watch_stat_sec=time(NULL)+OLED_timeout-1;
                seconds = time(NULL)+OLED_timeout;
                iWasSleeping=0;
                showStatistic();
            }
        }

        if(watch_python_sec<=time(NULL)){
            watch_python_sec=time(NULL)+300;
            //is python app running
            if(o_APRS.process_activ()!=0){
                printf("APRS Connector not running !\n");
                sError="APRS Connector not running";
                return 1;  // Programm abbrechen systemd macht restart
            }
            //is socket activ APRS_IS TCP
/*            if(o_APRS.socket_activ()!=0){
                printf("APRS Socket not established!\n");
                sError="APRS Socket not established";
                return 1;  // Programm abbrechen systemd macht restart
            } */
        }

        if(watch_restart_sec<=time(NULL)){
               watch_restart_sec=time(NULL)+15;
               if(o_APRS.restart_request()!=0){
                   printf("Restart from CGI!\n");
                   sError="Restart from CGI";
                   return 1;  // Programm abbrechen systemd macht restart
               }
               if(o_APRS.reboot_request()!=0){
                   printf("Reboot from CGI!\n");
                   sError="Reboot from CGI";
                   return 1;  // Programm abbrechen Reboot
               }
        }

        if(watch_connect_sec<=time(NULL)){
               watch_connect_sec=time(NULL)+60;
                string s_socketip = "./check_APRS_IS_connection";
                const string socketip_Result = o_APRS.exec(s_socketip.c_str());

                char aprs_state[100];
                char ipResult[100];
                char sResultc[1000];
                t = time(NULL);
                strftime(stat_timestamp, sizeof stat_timestamp, "%F %R", gmtime(&t));
//                printf("APRS_IS: %s Length %d\n",socketip_Result.c_str(),socketip_Result.length());
                if(socketip_Result.compare(last_socketip) || socketip_Result.length() != last_socketip.length() || socketip_Result.length() == 0 ) {    //only changing write to file
                    int notconnected = 0;
                    last_socketip = socketip_Result;
                    if(socketip_Result.length() >= 7) {
    //                    printf("APRS_IS = %s\n", socketip_Result.c_str());
                        sprintf(ipResult,"Connected since %s UTC to APRS_IS %s",stat_timestamp,socketip_Result.c_str());
                        strcpy(aprs_state,socketip_Result.c_str());
                        strtok(aprs_state,":");
                    } else {
                        sprintf(ipResult,"Not Connected since %s UTC",stat_timestamp);
                        strcpy(aprs_state,"");
                        notconnected = 1;
                    }
                    int fdc_out=open("/var/www/html/status.html", O_WRONLY|O_CREAT|O_TRUNC,0644);
                    strcpy(sResultc,"<HTML><HEAD><title>Lora-APRS I-Gate</title><meta http-equiv=\"refresh\" content=\"60; status.html\"></HEAD><BODY bgcolor=#A0A0A0><h3>Connection</h3> <p style=\"font-family:courier;\">");
                    write(fdc_out,sResultc,strlen(sResultc) );
                    if(strlen(aprs_state)) 
                        sprintf(sResultc,"%s<a href=\"http://%s:14501\" target=\"_blank\" >Show Dashboard</a><BR>",ipResult,aprs_state);
                    else
                        sprintf(sResultc,"%s<BR>",ipResult);
                    write(fdc_out,sResultc,strlen(sResultc) );
                    strcpy(sResultc,"</p><a href=\"status.html\">Refresh</a><br><a href=\"readuser.php\">Last Heard User</a><br><a href=\"index.html\">Mainpage</a></BODY></HTML>");
                    write(fdc_out,sResultc,strlen(sResultc) );
                    close(fdc_out);
                    if(notconnected) {
                        printf("Restart Service, because not connected!\n");
                        return 1;
                    }
                }
        }


        //#####################################
        //BME280 
        if (strBME280.compare("TRUE") == 0){
            readBME280();
              ///send Temp Pressure Huminity to Python Script
            if (SecTempSend<=time(NULL)){
                printf("Send TempHuminityPressure\n");
                SecTempSend= time(NULL)+TempSend;
                char sResult[1200];
                sprintf(sResult,"Temp:%0.1fC Pressure:%0.1fhPa Humidity:%0.1f%%\n",getTemp(),(getPressure()/100),getHuminity());
                o_APRS.sendudp(sResult,strlen(sResult));
                t = time(NULL);
                strftime(stat_timestamp, sizeof stat_timestamp, "%02d%02H%02M", gmtime(&t));
                sprintf(sResult,"%s>APOTW1,TCPIP*:@%sz%s/%s_.../...g...t%03dr...p...P...h%02db%05dBME280",APRS_IS_CALL.c_str(),stat_timestamp,LATITUDE.c_str(),LONGITUDE.c_str(),getTempWx(),getHuminityWx(),getPressureWx());
                o_APRS.sendudp(sResult,strlen(sResult));
                if(!not_sent) {
                    not_sent = 1;
                    sprintf(sResult,"%s>APOTW1,TCPIP*::%-9s:%s",APRS_IS_CALL.c_str(),APRS_IS_CALL.c_str(),PARM.c_str());
                    o_APRS.sendudp(sResult,strlen(sResult));
                    sprintf(sResult,"%s>APOTW1,TCPIP*::%-9s:%s",APRS_IS_CALL.c_str(),APRS_IS_CALL.c_str(),UNIT.c_str());
                    o_APRS.sendudp(sResult,strlen(sResult));
                    sprintf(sResult,"%s>APOTW1,TCPIP*::%-9s:%s",APRS_IS_CALL.c_str(),APRS_IS_CALL.c_str(),EQNS.c_str());
                    o_APRS.sendudp(sResult,strlen(sResult));
                    sprintf(sResult,"%s>APOTW1,TCPIP*::%-9s:%s",APRS_IS_CALL.c_str(),APRS_IS_CALL.c_str(),BITS.c_str());
                    o_APRS.sendudp(sResult,strlen(sResult));
                }
                sprintf(sResult,"%s>APOTW1,TCPIP*:T#%03d,%03d,%03d,%03d,000,000,00000000",APRS_IS_CALL.c_str(),count_tele,getTempTele(),getPressureTele(),getHuminityTele());
                if(++count_tele > 999) count_tele=0;
                o_APRS.sendudp(sResult,strlen(sResult));
                strftime(stat_timestamp, sizeof stat_timestamp, "%F %R", gmtime(&t));
                sprintf(sResult,"<HTML><HEAD><title>Lora-APRS I-Gate</title><meta http-equiv=\"refresh\" content=\"60; weather.html\"></HEAD><BODY bgcolor=#A0A0A0><h3>BME280 Weather (5min)</h3> <p style=\"font-family:courier;\"> UTC %s<BR>Temp:%0.1fC<BR>Pressure:%0.1fhPa<BR>Humidity:%0.1f%%<BR><br></p><a href=\"weather.html\">Refresh</a><br><a href=\"readuser.php\">Last Heard User</a><br><a href=\"index.html\">Mainpage</a></BODY></HTML>",stat_timestamp,getTemp(),(getPressure()/100),getHuminity());

                int fd_out=open("/var/www/html/weather.html", O_WRONLY|O_CREAT|O_TRUNC,0644);
                write(fd_out,sResult,strlen(sResult) );
                close(fd_out);

            }
        } else {
            if(!onetimerun) {
                onetimerun = 1;
                char sResult[1200];
                //strftime(stat_timestamp, sizeof stat_timestamp, "%02H:%02M", gmtime(&t));
                sprintf(sResult,"<HTML><HEAD><title>Lora-APRS I-Gate</title><meta http-equiv=\"refresh\" content=\"60; weather.html\"></HEAD><BODY bgcolor=#A0A0A0><h3>BME280 disabled</h3> <p style=\"font-family:courier;\"><BR></p><a href=\"weather.html\">Refresh</a><br><a href=\"readuser.php\">Last Heard User</a><br><a href=\"index.html\">Mainpage</a></BODY></HTML>");

                int fd_out=open("/var/www/html/weather.html", O_WRONLY|O_CREAT|O_TRUNC,0644);
                write(fd_out,sResult,strlen(sResult) );
                close(fd_out);
            }
        
        }
        

    }//while
    o_HMI.printGoodby();
    return 0;
}

