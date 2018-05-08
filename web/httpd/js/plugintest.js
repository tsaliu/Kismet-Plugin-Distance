(
  typeof define === "function" ? function (m) { define("plugintest-js", m); } :
  typeof exports === "object" ? function (m) { module.exports = m(); } :
  function(m){ this.Approx_Dist_Web = m(); }
)(function () {

"use strict";

var exports = {};

exports.load_complete = 0;

console.log("Loaded Web plugin");


kismet_ui.AddDeviceColumn('approx_dist', {
    sTitle: 'Approx. Dist',
    field: 'kismet.common.signal.last_signal_dbm',
    
    renderfunc: function(d, t, r, m) { //data, type, row, meta
	return parseFloat(Math.pow(10,(20+4-r['kismet.common.signal.last_signal_dbm'])/20)*r['kismet.device.base.frequency']/(4*Math.PI*3e8)).toFixed(2);
    },

});

kismet_ui.AddDeviceColumn('test_dis',{
	sTitle: 'Approx. Dist (m)',
	field: 'kismet.device.base.distance_map/kismet.device.base.los_distance',
	
	renderfunc: function(d,t,r,m){
		return d;
	},

});

kismet_ui.AddDeviceColumn('test_signal',{
	sTitle: 'Signal Str (dbm)',
	field: 'kismet.device.base.distance_map/kismet.device.base.distance.signal',
	
	renderfunc: function(d,t,r,m){
		return d;
	},
	

});

kismet_ui.AddDeviceColumn('test_mac',{
	sTitle: 'Source MAC',
	field: 'kismet.device.base.distance_map/kismet.device.base.distance.mac',
	
	renderfunc: function(d,t,r,m){
		return d;
	},

});

kismet_ui.AddDeviceColumn('test_freq',{
	sTitle: 'Freq (kHz)',
	field: 'kismet.device.base.distance_map/kismet.device.base.distance.freq',
	
	renderfunc: function(d,t,r,m){
		return d;
	},

});

exports.load_complete = 1;

return exports;
});

