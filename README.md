# LoraAPRSGW
2022-06-20, hb9pae


## Allgemeines
Die Modulationsart «LoRa» (Long Range) erlaubt durch ein Frequenzspreizverfahren (spread-spectrum technique)
die effiziente Übertragung von Daten mit geringer Leistung über grosse Distanzen. 

In dieser Anwendung werden GPS-Positionsdaten, zusammen mit einem APRS-Symbol und Sensordaten (Temperatur, 
Luftdruck und Luftfeuchtigkeit) an den APRS-IS Server (Automatic Packet Reporting System-Internet Service) übermittelt. 

Grundlage dieses Projektes diente die Webseite http://www.iot4pi.com/de/hardware/lora-gateway/.  

Wir haben eine Aufsteckplatine für den RPI-PI entwickelt. Interessenten melden sich unter  info@swiss-artg.ch.
  
----------------------------------

## Code Aufbau
Die Applikation LoraAPRSGW besteht aus folgenden Programmteilen:
- Programm LoraAPRS	Hauptprogramm C++
- Lora_APRS_gateway_7.py	Python Programm, Kommunikation mit dem APRS-IS Server 
- APRS.conf		APRS Konfiguration
- Position.conf		Standortdaten		

- loraprs.service	Systemd Steuerdaten  (/etc/systemd/system/)


Installiere folgende Libs
- sudo apt install libi2c-dev 
- sudo apt install screen 

Installiere den OLED-Driver
- git clone https://github.com/hallard/ArduiPi_OLED 
- cd ArduinoPI_OLED
- sudo make
- cd 

Aktiviere I2C und SPI Interface
- Enable I2C und SPI  (sudo raspi-config, Interface Options)
- Reboot RPI (sudo reboot)

## Installation LoraAPRSGW
Kompiliere und Linke den Code 
- cd /home/pi/LoraAPRSGW
- make clean
- make 

Kopiere die Systemd-Startdatei  
- sudo cp Systemd/loraaprs.service /etc/systemd/system

Enable die Sytemd-Startdatei	  
- sudo systemctl enable loraaprs.service
Starte die Applikation	  
- sudo systemctl start loraaprs.service


# History
## 2022-06-22 hb9pae
-High CPU load if  operated without BME280. Sleep command added.
-Python script changed to Version 0.7
-Startup script changed
-Installation instruction extended

## 2022-06-16 hb9pae
Installation instruction extended

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
73,
Bernd/OE1ACM

