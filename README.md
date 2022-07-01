# LoraAPRSGW
2022-07-01, hb9pae

## General 
LoRa (from "long range") is a physical proprietary radio modulation technique.
It is based on spread-spectrum modulation techniques derived from chirp spread spectrum (CSS) technology.
(c) Wikipedia

This application receives a LoRa modulated APRS Beacon from any LoRa-APRS Tracker and transmits the demodulated APRS datapacket to the APRS-IS server (Automatic Packet Reporting System-Internet Service).
In addition, local position and sensor datas like temperature, humidity and air pressure are periodically transmitted.

The website http://www.iot4pi.com/de/hardware/lora-gateway/ served as the basis for this project.

We have developed a plug-on board for the Raspberry Pi. Interested parties contact us at info@swiss-artg.ch.
----------------------------------

## code structure 
The application LoraAPRSGW consists of:
- Main program LoraAPRS
- Lora_APRS_gateway_7.py	(APRS_IS Gateway) 
- APRS.conf			(APRS Setup) 
- Position.conf			(APRS position data) 
- loraprs.service		(Programm control Systemd)

## Requirements
- Raspberry Pi 2,3 oder 3+
- Raspberry Pi OS Lite (Legacy), 32 bit, Debian 10 (Buster)

## Libraries
- sudo apt install libi2c-dev 
- sudo apt install screen 

## OLED Display Driver 
- git clone https://github.com/hallard/ArduiPi_OLED 
- cd ArduinoPI_OLED
- sudo make

## Aktiviere I2C und SPI Interface
- Enable I2C und SPI  (sudo raspi-config, Interface Options)
- Reboot RPI (sudo reboot)

## Source code 
- git clone https://github.com/hb9pae/LoraAPRSGW.git
- cd LoraAPRSGW
- make clean
- make 

## Copy control script 
- sudo cp Systemd/loraaprs.service /etc/systemd/system

## Enable and start control script	  
- sudo systemctl enable loraaprs.service
- sudo systemctl start loraaprs.service

--------------------------------------
# History
## 2022-07-01 hb9pae 
- additional remarks 

## 2022-06-22 hb9pae
- High CPU load if  operated without BME280. Sleep command added.
- Python script changed to Version 0.7
- Startup script changed
- Installation instruction extended

## 2022-06-16 hb9pae
- Installation instruction extended

## 2022-06-15 hb9pae
- Correction pressure  to absolute air pressure at sea level.
- New parameter "ALTUTUDE=xxx" in Position.conf
- Start process via Systemd adapted.
- Output version string at startup

## 2022-06-15 hb9pae
Tagged as V 0.7
Slightly changed code
- HTML statusfile disabled (main.cpp)
- changed filename position.conf to "Position.conf"
- new startscripts: loraaprs.service and LoraAPRS_Star.sh
- Logfile /var/log/LoraAPRS.log

## 2022-06-13 hb9pae
Latest sources added by Andreas OE5PON
Tagged as V 0.6

## 2022-06 hb9pae
Code forked from OE5PON/LoraAPRSGW
Tagged as V0.1

## LoRa APRS Gateway (Sascha's iot4pi version) 
Many thanks Sascha for providing us a copy of the LoRa-APRS Gateway code!  
I have uploaded the Code 'as-it-is'.
73, Bernd/OE1ACM
