#include "config.h"

#include "errno.h"
#include "syslog.h"

#include "util.h"
#include "messagebus.h"
#include "version.h"

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <map>
#include <memory>
#include <time.h>
#include <list>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "kis_mutex.h"
#include <macaddr.h>
#include <uuid.h>
#include "trackedelement.h"
#include <devicetracker.h>
#include <devicetracker_component.h>
#include "entrytracker.h"
#include <packet.h>
#include <globalregistry.h>
#include <tracked_location.h>
#include <tracked_rrd.h>
#include <packinfo_signal.h>

#include <packetchain.h>
#include <timetracker.h>

#include "kis_net_microhttpd.h"

#include "kismet_algorithm.h"

#include <sstream>
#include <math.h>

#include "configfile.h"
#include "messagebus.h"
#include "gpstracker.h"
#include "alertracker.h"
#include "manuf.h"
//#include "dumpfile_netxml.h"

#include "simple_datasource_proto.h"
#include "endian_magic.h"
#include "msgpack_adapter.h"
#include "datasourcetracker.h"

#include "json_adapter.h"
#include "structured.h"
#include "kismet_json.h"
#include "storageloader.h"
#include "base64.h"
#include "kis_datasource.h"
#include "kis_databaselogfile.h"
#include "phy_80211.h"
#include "plugintracker.h"
#include "kis_ppilogfile.h"
#include "kis_ppi.h"

#include "algo/los.h"

GlobalRegistry *globalreg = NULL;

class dis_tracked_device : public tracker_component{
public:

    dis_tracked_device(GlobalRegistry *in_globalreg, int in_id) : tracker_component(in_globalreg, in_id) {
        register_fields();
        reserve_fields(NULL);
    }


    dis_tracked_device(GlobalRegistry *in_globalreg, int in_id, SharedTrackerElement e) : tracker_component(in_globalreg, in_id) {
        register_fields();
        reserve_fields(e);
    }

    virtual SharedTrackerElement clone_type(){
		return SharedTrackerElement(new dis_tracked_device(globalreg, get_id()));
	}

	
    virtual ~dis_tracked_device(){
    	
    };
	
	__Proxy(los_distance, double ,double ,double, los_distance);
	__Proxy(signal, int, int, int, signal);
	__Proxy(mac, std::string, std::string, std::string, mac);
	__Proxy(freq, double,double, double, freq);
	
	static int chainhandler(CHAINCALL_PARMS);
protected:
	//GlobalRegistry *globalreg;

	virtual void register_fields() {
        tracker_component::register_fields();

		RegisterField("kismet.device.base.los_distance", TrackerDouble, "los distance", &los_distance);
		RegisterField("kismet.device.base.distance.signal", TrackerInt32, "distance_signal strength", &signal);
		RegisterField("kismet.device.base.distance.freq", TrackerDouble, "distance_frequency", &freq);
		RegisterField("kismet.device.base.distance.mac", TrackerString, "distance_mac address", &mac);

    	}
	virtual void reserve_fields(SharedTrackerElement e){
		tracker_component::reserve_fields(e);
	};

	
	SharedTrackerElement los_distance;
	SharedTrackerElement signal;
	SharedTrackerElement mac;
	SharedTrackerElement freq;
};

class Track_Algo : public LifetimeGlobal, public tracker_component {
public:
	static std::shared_ptr<Track_Algo> create_track(GlobalRegistry *in_globalreg){
		std::shared_ptr<Track_Algo> mon(new Track_Algo(in_globalreg));
		in_globalreg->RegisterLifetimeGlobal(mon);
		in_globalreg->InsertGlobal("TRACK_ALGO", mon);
		return mon;
	}
private:
	Track_Algo(GlobalRegistry *in_globalreg);


public:
	virtual ~Track_Algo(){
		local_locker lock(&track_mutex);
		globalreg->RemoveGlobal("TRACK_ALGO");
		
		std::shared_ptr<Packetchain> packetchain = std::static_pointer_cast<Packetchain>(globalreg->FetchGlobal(std::string("PACKETCHAIN")));
		if(packetchain != NULL){
			packetchain->RemoveHandler(&chainhandler, CHAINPOS_TRACKER);
		}
			
	}	
	static int chainhandler(CHAINCALL_PARMS);
	

		
protected:
	int distance_map_id;
	SharedTrackerElement distance_map;
		
	int distance_entry_id;
	
	
	virtual void register_fields(){
		tracker_component::register_fields();
		
		distance_map_id = RegisterField("kismet.device.base.distance_map", TrackerMap, "Distance Map", &distance_map);
		
		std::shared_ptr<dis_tracked_device> track_builder(new dis_tracked_device(globalreg, 0));
		
		distance_entry_id = RegisterComplexField("kismet.device.base.track_bulder", track_builder, "Distance Entry");
			
	}
	
	GlobalRegistry *globalreg;
	
	kis_recursive_timed_mutex track_mutex;
	
	int pack_comp_80211, pack_comp_device, pack_comp_l1data, pack_comp_sinfo;

};

Track_Algo::Track_Algo(GlobalRegistry *in_globalreg) : tracker_component(in_globalreg, 0){
	globalreg = in_globalreg;	
	register_fields();
	reserve_fields(NULL);
	
	
	fprintf(stderr, "DEBUG: Create LifeTimeGlobal\n");

	
	std::shared_ptr<Packetchain> packetchain = std::static_pointer_cast<Packetchain>(in_globalreg->FetchGlobal(std::string("PACKETCHAIN")));
	
	pack_comp_80211 = packetchain->RegisterPacketComponent("PHY80211");
	pack_comp_device = packetchain->RegisterPacketComponent("DEVICE");
	pack_comp_l1data = packetchain->RegisterPacketComponent("RADIODATA");
	pack_comp_sinfo = packetchain->RegisterPacketComponent("STRINGDATA");
	
	packetchain->RegisterHandler(&chainhandler, this, CHAINPOS_TRACKER,0);
	
}

int Track_Algo::chainhandler(CHAINCALL_PARMS){
	
	//fprintf(stderr, "DEBUG: CHAINCALL_PARMS\n");
	Track_Algo *ta = (Track_Algo *) auxdata;
	//local_locker locker(&(ta->track_mutex));
	
	//packetchain_comp_datasource *pack_datasrc = (packetchain_comp_datasource *) in_pack->fetch(globalreg->packetchain->RegisterPacketComponent("KISDATASRC"));		
	//KisDatasource *source = pack_datasrc->ref_source;
	
	
	if (in_pack->error){
		_MSG("in_pack is error 1"   ,MSGFLAG_INFO);	
		//return 0;	
	}
	
	dot11_packinfo *packinfo = (dot11_packinfo *) in_pack->fetch(ta->pack_comp_80211);
	kis_layer1_packinfo *radioinfo = (kis_layer1_packinfo *) in_pack->fetch(ta->pack_comp_l1data);
	kis_tracked_device_info *devinfo = (kis_tracked_device_info *) in_pack->fetch(ta->pack_comp_device);
	kis_string_info *radiostring = (kis_string_info *) in_pack->fetch(ta->pack_comp_sinfo);

	if (devinfo == NULL)
    	return 1;
    	

    // Try to pull the existing basedev, we don't want to re-parse
    	for (auto di : devinfo->devrefs) {
       		std::shared_ptr<kis_tracked_device_base> basedev = di.second;

       		if (basedev == NULL)
           		return 1;
           		
           	if(radioinfo ==NULL)
           		return 1;

      	
      	TrackerElement::map_iterator imi = basedev -> find(ta->distance_map_id);
			
      	std::shared_ptr<dis_tracked_device> my_distance;
    		if(imi == basedev->end()){
        		my_distance.reset(new dis_tracked_device(globalreg, ta->distance_map_id));
       			basedev->add_map(my_distance);
    		}
    		else{
    			my_distance = std::static_pointer_cast<dis_tracked_device>(imi->second);
    		}
   		my_distance->set_signal((int) radioinfo->signal_dbm);
   		my_distance->set_freq((double) radioinfo->freq_khz);
   		my_distance->set_mac((std::string) packinfo->source_mac.Mac2String().c_str());
   		my_distance->set_los_distance((double) los_distance(radioinfo->signal_dbm, radioinfo->freq_khz));
		}
	
	return 1;
}



int testfunction(GlobalRegistry *in_globalreg){
	globalreg=in_globalreg;
	
	Track_Algo::create_track(globalreg);
	
	
	_MSG("Done loading Plugin"  ,MSGFLAG_INFO);

	return 1;

}


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


