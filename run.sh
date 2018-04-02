#!/bin/bash
set -e

if [ "$EUID" == 0 ]
	then echo "ALERT:	Please do NOT run as sudo"
		echo "INFO:	Kismet was suidinstall, and browser cannot be sudo"
		echo "Terminating ..."
	exit
fi

if pgrep -f "chromium-browser" >/dev/null 2>&1
	then 
		echo "Chromium-Browser Running"
		echo "Killing Chromium-Browser ..."
		sudo pkill chromium
fi

#installing web ui
cd /home/pi/kismet/plugin-test/web
sudo make install

#installing track
cd /home/pi/kismet/plugin-test/track
sudo make install -j4


python -m webbrowser http://localhost:2501
#python -m webbrowser http://localhost:2501/plugin/dashboard/
#python -m webbrowser http://localhost:2501/plugin/plugtest/

#python openweb.py

#cd /home/pi/kismet/plugin-test/track
#sudo rm *.kismet
#sudo rm *.kismet-journal
sudo kismet -f /home/pi/kismet/conf/kismet.conf







