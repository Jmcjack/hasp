/*! \file doublet_theta3.c
 *	\brief Three doublet commands on pitch angle
 *
 *	\details
 *	\ingroup guidance_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: doublet_theta3.c 757 2012-01-04 21:57:48Z murch $
 */

#include <stdlib.h>
#include <math.h>

#include "../globaldefs.h"
#include "../system_id/systemid_interface.h"
#include "guidance_interface.h"

static double base_pitch_cmd= 0.0872664; // 5deg

extern void get_guidance(double time, struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr){
	
				controlData_ptr->theta_cmd = base_pitch_cmd + doublet(2, time, 5, 5*D2R) + \
								doublet(9, time, 5, 5*D2R) + \
								doublet(16, time, 5, 5*D2R); // Pitch angle command;

}

