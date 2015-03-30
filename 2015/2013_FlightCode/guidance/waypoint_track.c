/*! \file waypoint_track.c
 *	\brief Waypoint tracker get_guidance law
 *
 *	\details Designed by Garrison Hoe. Waypoints specified for TriValley RC field.
 *	\ingroup guidance_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: waypoint_track.c 757 2012-01-04 21:57:48Z murch $
 */

#include <stdlib.h>
#include <math.h>

#include "../globaldefs.h"
#include "../system_id/systemid_interface.h"
#include "guidance_interface.h"

static float base_pitch_cmd= 0.0872664; // 5deg

extern void get_guidance(double time, struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr){
	
	
	double phi_cmd = 0;
	
	// waypoint tracker variable decleration
	double waypoint[4][2] = { { 44.732913, -93.0594 },
			{ 44.734464, -93.057697 }, { 44.734959, -93.0629 }, { 44.732928,
					-93.0625 } }; //waypoint lat/lon
	static int xx = 1; //start towards first waypoint
	double lat1, lat2, lon1, lon2, dlat, dlon, dist_lat, dist_lon, D;
	double V_e_x, V_e_y, mag_bearing, mag_heading, theta, cross_sign, delta,delta_p=0;


	//*************************************************
	//*************************************************
	// WAYPOINT TRACKER
	//*************************************************
	//*************************************************
	lat1 = navData_ptr->lat; // lat;
	lat2 = waypoint[xx][0];
	lon1 = navData_ptr->lon; // lon;
	lon2 = waypoint[xx][1];

	dlat = (lat2 - lat1);
	dlon = (lon2 - lon1);

	dist_lat = dlat * 111112; //Convert degrees to meters
	dist_lon = dlon * 80206; //Convert degrees to meters

	D = sqrt(dist_lat * dist_lat + dist_lon * dist_lon);
	if (D < 45) { //Switch to next waypoint once you are within 30 meters
		xx = xx + 1;
	}
	if (xx > 3) {
		xx = 0;
	}

	V_e_x = navData_ptr->vn; // vnorth
	V_e_y = navData_ptr->ve; // veast

	mag_bearing = sqrt(V_e_x * V_e_x + V_e_y * V_e_y);
	mag_heading = sqrt(dlat * dlat + dlon * dlon);
	theta = acos((dlat * V_e_x + dlon * V_e_y) / (mag_bearing
			* mag_heading));
	cross_sign = (dlat * V_e_y - dlon * V_e_x);
	if (cross_sign < 0) {
		theta = -theta;
	}
	delta = 0.9048 * delta_p + 0.09516 * (theta);
	delta_p = delta;
	phi_cmd = delta * -1.2;
	if (phi_cmd < -0.7) {
		phi_cmd = -0.7;
	}
	if (phi_cmd > 0.7) {
		phi_cmd = 0.7;
	}
	// end waypoint tracker code
	
	// Assign doublet sequences to references vector
	controlData_ptr->phi_cmd= phi_cmd; // Roll angle command
	controlData_ptr->theta_cmd = base_pitch_cmd; // Pitch angle command

}

