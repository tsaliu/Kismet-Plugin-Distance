#ifndef __LOS_H__
#define __LOS_H__


#include <math.h>


double los_distance(int signal_dbm, double freq_khz){
	double Pt_dbm = 20;
	double Gt_dbi = 4;
	double Gr_dbi = 5;
	double Pr_dbm = signal_dbm;
	double freq = freq_khz * 1e3 ;
	double c = 3*pow(10,8);
	double wavelength = c / freq;
	double tmp_v = pow(10, ((Pr_dbm - Gr_dbi - Pt_dbm - Gt_dbi) / 20));
	double tmp_vs = sqrt(tmp_v);
	double approx_distance = wavelength / (4 * M_PI * tmp_vs);
	//double approx_distance = pow(10,((-(Pr_dbm-Pt_dbm-Gt_dbi))/20));
	return approx_distance;
}

#endif
