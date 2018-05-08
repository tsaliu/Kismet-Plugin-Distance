#!/bin/bash
set -e

#installing web ui
cd /home/pi/kismet/plugin-test/web
sudo make install

#installing track
cd /home/pi/kismet/plugin-test/track
sudo make install


cd /home/pi/kismet
gdb ./kismet 
sleep 5
gdb run --debug -f /home/pi/kismet/conf/kismet.conf

