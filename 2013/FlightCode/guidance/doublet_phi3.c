/*! \file doublet_phi3.c
 *	\brief Three doublet commands on roll angle
 *
 *	\details
 *	\ingroup guidance_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: doublet_phi3.c 757 2012-01-04 21:57:48Z murch $
 */

#include <stdlib.h>
#include <math.h>

#include "../globaldefs.h"
#include "../system_id/systemid_interface.h"
#include "guidance_interface.h"

static float base_pitch_cmd= 0.0872664; // 5deg

extern void get_guidance(double time, struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr){
	
				controlData_ptr->phi_cmd = doublet(2, time, 4, 20*D2R) + \
								doublet(10, time, 4, 20*D2R) + \
								doublet(16, time, 4, 20*D2R); // Roll angle command; doublet at 2 sec, fault at 8, doublets at 10 and 16
				controlData_ptr->theta_cmd = base_pitch_cmd; // Pitch angle command

}

