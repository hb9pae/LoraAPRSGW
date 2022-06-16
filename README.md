# LoraAPRSGW
2022-06-15, hb9pae


## Allgemeines
Die Modulationsart «LoRa» (Long Range) erlaubt durch ein Frequenzspreizverfahren (spread-spectrum technique)
die effiziente Übertragung von Daten mit geringer Leistung über grosse Distanzen. 

In dieser Anwendung werden GPS-Positionsdaten, zusammen mit einem APRS-Symbol und Sensordaten (Temperatur, 
Luftdruck und Luftfeuchtigkeit) an den APRS-IS Server (Automatic Packet Reporting System-Internet Service) übermittelt. 

Grundlage dieses Projektes diente die Webseite http://www.iot4pi.com/de/hardware/lora-gateway/.  

----------------------------------

## Code Aufbau
Die Applikation LoraAPRSGW besteht aus folgenden Programmteilen:
- Programm LoraAPRS	Hauptprogramm C++
- Lora_APRS_gateway_6.py	Python Programm, Kommunikation mit dem APRS-IS Server 
- APRS.conf		APRS Konfiguration
- Position.conf		Standortdaten		

- loraprs.service	Systemd Steuerdaten  (/etc/systemd/system/)

## Build-Process
- make clean
- make 

## Installation
- Kompiliere und Linke den Code (Build-Process)
- Kopiere das Systemd-Startdatei  (sudo cp Systemd/loraaprs.service /etc/systemd/system)
- Enable die Sytemd-Startdatei	  (sudo systemctl enable loraaprs.service)
- Starte die Applikation	  (sudo systemctl start loraaprs.service)

# History
## 2022-06-15 hb9pae
- Ergänzung Code zur Korrektur des abs. Luftdruckes auf Normalnull (Meereshöhe) 
- Neuer Parameter "ALTUTUDE=xxx" in Position.conf
- Startprozess via Systemd angepasst.
- Ausgabe Versionsstring beim Start 

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

