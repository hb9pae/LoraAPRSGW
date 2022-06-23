#!/bin/sh

LOGFILE="/var/log/LoraAPRS.log"
WorkingDir="/home/pi/LoraAPRSGW"

cd $WorkingDir

while true
do 
	Date=$(date '+%Y-%m-%d %H:%M:%S')
	echo "$Date Startup ready, starte Lora iGate" >> $LOGFILE
	/usr/bin/screen -S LoraAPRS -D -m -h 2000 -L -Logfile $LOGFILE $WorkingDir/LoraAPRS
done
