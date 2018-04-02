# Kismet Plugin for Device Distance
## Special thanks
- Developer dragorn from kismetwireless 
  - https://www.kismetwireless.net/
  - https://github.com/kismetwireless/kismet
## Requirement:
- Raspberry Pi
- Wireless Card (make sure the wireless card support the correct kernal version and also support monitor mode)
  - populor choices are 
    - TL-WN772N V1.0 or V2.0 (V3.0 and up are not supported)
    - TL-WN822N V1.0 to V4.0
- GPS USB Dongle (Optional, if want to locate Raspberry pi)
  - populor choice BU-353S4
## Steps and Guide
1. Install Raspbian, either using NOOBS or directly Raspbian
   - Backup (Clone) RPi
     - since we are using NOOBS, the traditional way to backup or clone would be merging all partitions and create an image
       - more convient way is to use rpi-clone
		```
		git clone https://github.com/billw2/rpi-clone.git
		cd rpi-clone
		sudo cp rpi-clone rpi-clone-setup /usr/local/sbin
		```
       - test if rpi-clone works
		```
		sudo rpi-clone -t testname
		```
         - check files in /tmp/clone-test to see if lines are added
       - plug in the external sd card using usb sd card reader
	 - check you external card name (IMPORTANT OR OTHER FILES MAY BE OVERWRITE)
	 - should be something like `/dev/sda#	`
	   - `#` is the number showed up
       - clone
		```
		sudo rpi-clone sda#
		```
         - if does not have any other sda, just use
		```
		sudo rpi-clone
		```
	 - follow the rest of the steps

2. Connect to Internet (Wifi or Ethernet)
   - For McMaster Wifi
	```
	sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
	```
	```
	country=CA
	ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
	ap_scan=1
	update_config=1
		
	network={
		ssid="MacSecure"
		scan_ssid=1
		key_mgmt=WPA-EAP
		pairwise=CCMP TKIP
		eap=PEAP
		identity="MacID"
		password="Password"
		#phase1="peapver=0"
		phase2="auth=MSCHAPV2"
	}	
	```
	```
	sudo nano /etc/network/interfaces
	```
	```
	# interfaces(5) file used by ifup(8) and ifdown(8)
	
	# Please note that this file is written to be used with dhcpcd
	# For static IP, consult /etc/dhcpcd.conf and 'man dhcpcd.conf'
	
	# Include files from /etc/network/interfaces.d:
	source-directory /etc/network/interfaces.d
	
	auto lo
	#iface lo inet loopback
	
	iface eth0 inet manual
	#iface eth0 inet dhcp
	
	auto wlan0
	allow-hotplug wlan0
	iface wlan0 inet dhcp
	wpa-driver wext
		wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf

	#Here direct your wpa settings for you wireless card dongle
	auto wlan1
	allow-hotplug wlan1
	iface wlan1 inet manual
	wpa-conf /home/pi/wpa.conf
	```

3. Update and Upgrade system
	```
	sudo apt-get update
	sudo apt-get upgrade
	```

4. Install neccesities
	```
	sudo apt-get install gedit geany android-tools-adb android-tools-fastboot firmware-realtek firmware-atheros wireless-tools usbutils iw wireshark aircrack-ng
	sudo apt-get install libncurses5-dev libpcap-dev build-essential libpcre3-dev libnl-3-dev libnl-genl-3-dev gpsd gpsd-clients python-gps unzip
	sudo apt-get install libmicrohttpd-dev libsqlite3-dev network-manager network-manager-dev libnm-util-dev git python-dev
	sudo apt-get install zlib1g-dev libcap-dev libnm-dev libdw-dev libsqlite3-dev 
	```
  - install wireless card driver (Here we have TL-WN822N V4.0)
    - (for TL-WN822N V1.0 to V2.0 should be plug and play, requires AR-9170)
    - (for V3.0 requires rtl8192cu, V4.0 requires rlt8192eu)
		```
		sudo wget http://www.fars-robotics.net/install-wifi -O /usr/bin/install-wifi
		sudo chmod +x /usr/bin/install-wifi
		sudo install-wifi
	    ```
		```
		sudo wget http://wireless.kernel.org/download/htc_fw/1.3/htc_9271.fw
		sudo cp htc_9271.fw /lib/firmware
		```
  - install serf
  	```
	sudo apt-get install unzip
	wget https://releases.hashicorp.com/serf/0.8.1/serf_0.8.1_linux_arm.zip
	unzip serf_0.8.1_linux_arm.zip 
	sudo mv serf /usr/local/bin
	```
  - install msgpack-c
  	```
	git clone https://github.com/msgpack/msgpack-c.git
	cd msgpack-c
	cmake .
	make
	sudo make install
	```

  - install kismet(do NOT use sudo apt-get to install, may result in mismatch file location and later we need src to compile plugins)
	```
	sudo wget http://www.kismetwireless.net/code/kismet-2016-07-R1.tar.xz
	tar -xvf kismet-2016-07-R1.tar.xz
	cd kismet-2016-07-R1
	sudo ./configure
	make dep
	make
	make install
	sudo groupadd -g 71 kismet
	sudo usermod -a -G kismet pi
	```
	- !!!!NOTE BY USING THIS CANNOT REPLAY LIVE (if use this method, continue to step 5)

  - second method for installing kismet
  	```
	sudo git clone https://github.com/kismetwireless/kismet.git
	sudo ./configure
	sudo make
	sudo make suidinstall
	sudo usermod -a -G kismet pi
	(restart)
	```
  - check if in kismet group
  		```
		groups
		```
	- !!!!!THIS IS RECOMMANDED
	  - (by using this step 7, 9, 11 will not work)

5. Lanch kismet
   - (if install with suidinstall can just do kismet)
	```
	sudo kismet
	```
6. putting wireless card in monitor mode
	```
	sudo airmon-ng check kill
	sudo airmon-ng start wlan1
	```
   - check if put in monitor successfully (if success, wireless card will have bracket saying monitor mode)
	```
	sudo airmon-ng
	```
   - second method for putting into monitor mode (here we define name to be wlan1mon)
    ```
	sudo iw dev wlan1 interface add wlan1mon type monitor
	sudo iw dev wlan1 del
    ```
   - check name for step 7 (look for mon)
    ```
	sudo ifconfig
    ```
7. Configure kismet
	```
	sudo gedit /usr/local/etc/kismet.conf
 	```
   - for replay
		-Go to source
		```
		source=location of pcapdump:realtime=true, retry=true
		```
   - for filtering
	 - go to filter_tracker (Here we filter for "depline")
		```
		filter_tracker=BSSID(9C:A9:E4:AB:5C:F2)
		```
   - for capture
	 - go to ncsource (depending on the name for monitor)
	 	```
		source=mon0
		source=wlan1mon0
		source=wlan1mon
		```
8. Enable GPS (optional)
   - method 1
   	```
	sudo apt-get install gpsd gpsd-clients python-gps
	gpsd /dev/ttyUSB0
	sudo gpsd /dev/ttyUSB0 -F /var/run/gpsd.sock
	cgps
	```

   - method 2
   	```
	sudo systemctl stop gpsd.socket
	sudo systemctl disable gpsd.socket
	sudo gpsd /dev/ttyUSB0 -F /var/run/gpsd.sock
	cgps
	```
   - method 3
   	```
	cat /dev/ttyUSB0
	stty -F /dev/ttyUSB0 4800
	```
	
   - method 4
   	```
	dmesg|grep tty
	gpsd /dev/ttyUSB0
	cgps
	```

9. Compile Plugin (assuming the kismet src is at /home/pi/Downloads/kismet-2016-07-R1)
   - move the plugin foler to src
   	```
	sudo mv pluginfoldername /home/pi/Downloads/kismet-2016-07-R1
	cd /home/pi/Downloads/kismet-2016-07-R1/pluginfoldername
	```
     - configure Makefile
       - the Makefile is already pre-configured, only need to make sure the kismet src location (KIS_SRC_DIR)
	 - Here since it is just before plugin folder KIS_SRC_DIR ?= ../

     - CLIOBJS is the main cc code for client, if have custom headers (class/struct ... etc) input here too
       - for example I have plugtest_main_ui.cc plugtest2_class_ui.cc
	 - CLIOBJS = plugtest_main_ui.o plugtest2_class_ui.o
     - CLIOUT is compiled plugin files client for kismet
       - CLIOUT = plugtest_main_ui.so
     - SRVOBJS is the main cc code for server, same ideal with client
       - SRVOBJS = kismet_plugtest.o plugtest.o
     - SRVOUT is compiled plugin files for server for kismet
       - SRVOUT = kismet_plugtest.so

     - move the compiled .so files into kismet plugin folders
    	```
		sudo rm /usr/lib/kismet_client/plugtest_main_ui.so
		sudo cp plugtest_main_ui.so /usr/lib/kismet_client
		```

		```
		sudo rm /usr/lib/kismet/kismet_plugtest.so
		sudo cp kismet_plugtest.so /usr/lib/kismet
		```
10. Step 7. and 9. can be done by simply run "run.sh" file (optional) (can edit the source, filter and location of plugin folders in run.sh)
	```
	./run.sh
	```
    - if unable to run run.sh
      - make executable
      	```
		sudo chmod +x *.sh
		```
	
11. Enable Plugins in kismet
    - plugins can be pre-enable by editing kismet.conf and kismet_ui.conf
	
    - or launch kismet 
    	```
		sudo kismet 
		```
      - under kismet/plugins/select plugins
 	- use tab to alter between client and server (server plugin is loaded auto)
	- use up down key and enter to change the load status, then close. restart kismet

12. Configuring Kismet
    - since there are 3 versions of kismet, it would be safer to specify .conf used
    - here we use the on that comes with the src files
    	```
		sudo gedit /home/pi/kismet/conf/kismet.conf
		```
	
    - under source specify the interface that is in monitor mode by step 6
      - for replay source=pcapdumpfilelocation:realtime=true,retry=true

13. Lanuch kismet
	```
	sudo kismet -f /home/pi/kismet/conf/kismet.conf
	```
    - when first launch kismet, random password will generate in ~/.kismet/kismet_httpd.conf
      - we can directly input the username and password into /usr/local/etc/kismet/kismet_httpd.conf 
      - here, my password and username is
		```
		password=6xcbbfPBwNsUernV
		httpd_username=kismet
		```

14. Connect to client/UI
	- in web browser put in http://localhost:2501/

15. Web-only plugin for kismet (can access cross-platform anywhere in local network without kismet)
	- requires: Makefile and manifest.conf
	- build with .js .css and index.html
	- to compile `cd /pluginfolder/`, need to define Kismet SRC in Makefile (here we have /home/pi/kismet)
		```
		sudo make install 
		```
	- for example 
		```
		sudo make install 
		```
	  - access at http://localhost:2501/plugin/pluginname/
	  - this way need to write custom .css and index.html (web development)
	    - here we don't need it, so can ignore .css and index.html, the .css and index.html is from example "plugin-dashboard"

	- simplier option (DO NOT run as sudo)
		```	
		./run.sh
		```
	  - will auto
	    - compile plugin
	    - open kismet	 
	    - open default kismet UI, plugin UI
	- it is possible to open multiple UI pages to load different plugins

16. File types
	- Makefile: defines how the plugin is compiled (should only modify the SRC)
	- run.sh: auto open kismet server, kismet UI and compile plugin
	- manifest.conf: plugin static web content
	  - contains basic description of the plugin and the location for URL
	  - example manifest for js (use http://localhost:2501/plugin/plugin_url to access)
		```
		name=PluginName_to_display_on_info_page
		description=description of plugin
		author=name <can put contact here>
		version=the version number of your plugin
		js=plugin_model_name,/plugin/plugin_url/js/plugin_js_filename.js
		```
	    - for c++, need (object=kismet-plugin_name.so)
	- index.html: use to define the layout of the plugin page, uses parameters from /css/xxx.css
	- xxx.css: use to define the properties of each element in the plugin page
	- xxx.js: main plugin code
	- Recommand to seperate your .js plugin and .so plugin

17. Ways to interact with kismet server
    - create client to fetch kismet server (external)
      - can use "kismetclient by PauMcMillan" (old, no update, no support)
      - can use kismetRest (fetching using python scripts)
    - directly use js or c++ (internal)
      - create new plugin page other than the default kismet UI
      - Plugin output data can direct be added in the default kismet UI (this way don't need css and index)
	- the UI uses DataTables plugin for jQuery from https://datatables.net
18. Output
    - if you do everything correctly, and using method of creating custom field in web ui datatable. then we can simply run "run.sh" (do NOT run as sudo)
	```	
	./run.sh 
	```
    - it will auto 
      - compile plugin and put it into plugin folder
      - run kismet server (http://localhost:2501)
      - open kismet UI with plugin colum
19. Algorithm
    - the algorithms are store in the folder "algo" in track folder
    - see detail in algo
    
20. Other capture methods- using Aircrack-ng (some say cannot capture hidden network, benifit of this is will auto setup the wireless interface to monitor if supports)
	```
	sudo airmon-ng start wlan1
	sudo airodump-ng wlan1
	```
	or
	```
	sudo airodump-ng wlan1 -w /some/directory/for/logs.pcap --manufacturer
	```
21. Internal Kismet Plugin
    - there are 3 main portion needed for the plugin (excluding the Web UI)
      - Tracker Components, GlobalRegistry, Activate Plugins
      - IMPORTANT, every class should have a virtual destructor even if we do not need them

    - Tracker Components enable us to map the data we desire to the DataTable on Web UI, and also easy access
      - The following template is a usual usage for tracker_component class
	```
	class some_track_component : public tracker_component {
	public:
    		some_track_component(GlobalRegistry *in_globalreg, int in_id) :
        		register_fields();
        		reserve_fields(NULL);
    		}
    		some_track_component(GlobalRegistry *in_globalreg, int in_id, SharedTrackerElement e) : tracker_component(in_globalreg, in_id) {
        		register_fields();
        		reserve_fields(e);
    		}
    		virtual SharedTrackerElement clone_type(){
			return SharedTrackerElement(new dis_tracked_device(globalreg, get_id()));
		}
    		virtual ~dis_tracked_device(){};

		__Proxy(name,tracker type, input type, return type, variable);
		__ProxyTrackable(example_vec, TrackedElement, example_vec);
	protected:
		virtual void register_fields() {
        		tracker_component::register_fields();
			RegisterField("Some.Name", TrackerType, "Some_Readable_Name", &name);
		}	
		virtual void reserve_fields(SharedTrackerElement e){
			tracker_component::reserve_fields(e);
		}
	SharedTrackerElement name;
	};
	```
		
    - the `__Proxy` and `__ProxyTrackable` creates easy mapping for components
      - also have other `__Proxy` functions, see trackedelement.h
	- allow us to use get_name() and set_name()
	- RegisterField registers the tracker_components into a field corresponding to DataTable
	- SharedTrackerElement defines the variable you want to track.
    - Register in GlobalRegistry, to link the custom field under a map and register the track component into packetchain
      - There are two types of global registry we can add: regular and lifetime
	- to add regular register we can simply use GlobalRegistry::RegisterGlobal(string name), and GlobalRegistry::FetchGlobal(string name)
      - for lifetimeglobal we need to define as follows
	```
	class someclass : public LifetimeGlobal {
	public:
	static shared_ptr<someclass> create_someclass(GlobalRegistry *in_globalreg){
		shared_ptr<someclass> mon(new someclass(in_globalreg));
		in_globalreg->RegisterLifetimeGlobal(mon);
		in_globalreg->InsertGlobal("SOME_NAME", mon);
		return mon;
	}
	pritave:
		someclass(GlobalRegistry *in_globalreg);
	protected:
		GlobalRegistry *globalreg;
		kis_recursive_timed_mutex someclass_mutex;
	```
      - the mutex is need to ensure this process is being use once at a time
	- VERY IMPORTANT, will produce corrupted double-linked otherwise, or Broken PIPE
	- don't need if we know the plugin is access once at a time.	
      - Enables and Activate Plugins
	- similar to the older (legacy) kismet, we need to check to kismet plugin version, activate and finalize plugins.
	- hence something like below is a must for plugins
	```
	extern "C" {
   		int kis_plugin_version_check(struct plugin_server_info *si) {
     			 si->plugin_api_version = KIS_PLUGINTRACKER_VERSION;
     			 si->kismet_major = VERSION_MAJOR;
     			 si->kismet_minor = VERSION_MINOR;
      			 si->kismet_tiny = VERSION_TINY;
      		 	 return 1;
    		}
   		int kis_plugin_activate(GlobalRegistry *in_globalreg) {
      		 	 return 1;
    		}
    		int kis_plugin_finalize(GlobalRegistry *in_globalreg) {
        		return testfunction(in_globalreg);
    		}
	}
	```
		
	- kis_plugin_activate should return negative on failure, non-negative on success
	- kis_plugin_finalize is the same fashion as activate, but MUST return sucess during initial activation or receive the finalization event.
	- Then a function(CHAINCALL_PARMS) would be called to do nesscary process for each packets.
	
22. Debug
    - usually we just 
      - display on kismet UI
      		```
			_MSG("SOMETEXT", MSGFLAG_INFO) 	
			```
      - display on terminal
      		```
			fprintf(stderr, "SOMETET %i", int some_number)
			```
    - if cannot debug with above
      - go to kismet src directory
      		```
			gdb ./kismet
			run --debug
			```
	- supports terminal option like normal kismet
	  - i.e `run --debug -f /home/pi/kismet/conf/kismet.conf`
	  - some options are
	  	```
		(gdb) handle SIGPIPE nostop noprint pass   (to ignore PIPE signal)
		(gdb) set logging on     (log to a file)
		(gdb) bt                 (backtrace, where crash)
		(gdb) info threads 	 (collect threads info)
		(gdb) thread apply all bt full             (collect per-thread backtraces)
		```
	- check kismet README.md file for more detail
	
23. Check Registered Field
    - even if everything compiles, built and runs, we need to make sure variables are registered correctly
    - this can be done by checking the `dev/debug` catagory in deviceinfo
24. Logging
	- if the tracker component field is inserted into device object, it will be auto log into .kismet file.
	  - NOTE: the custom field will only be log in .kismet file, the values will NOT be convert into pcap, since pcap cannot hold dynamic extra data and is for packets. the data can be converted in to JSON.
	  - detail about logging can be found in /docs/dev/log_kismet.md
	    - to writing into pcap, will require custom plugin for wireshark, or parse the signal headers (radiotap or ppi) in an external tool
	  - the .kismet is in sqlite3 format. in order to open/read we need sqlite3
	  	```
		sudo apt-get install sqlite3
		```
	    - to read
	    	```
			sqlite3
			.read FILENAME.kismet
			```
	  - the .kismet file can be converted into pcap or JSON using the log_tools written in Python by dragorn (kismetwireless)
	  - dependencies needed
	  	```
		sudo apt-get install python-dateutil
		```
	- alternativly, user can download the pcapng file from the web ui, but requires login
	  - the username and password for the web ui are recorded in ~/.kismet/kismet_httpd.conf
	    - can either input through web ui or directly configure config file from /usr/local/etc/kismet_httpd.conf
25. This plugin is based on uav-phy in kismet github

