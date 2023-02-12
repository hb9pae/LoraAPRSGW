#!/bin/sh

WorkingDir="/home/ubuntu/src/LoraAPRSGW"

cd $WorkingDir

while true
do 
	echo "Starting Lora iGate"
	/usr/bin/screen -S LoraAPRS -h 2000 $WorkingDir/LoraAPRS
	sleep 0.5
done

