/*! \file straight_level.c
 *	\brief Straight and level pitch/roll commands
 *
 *	\details
 *	\ingroup guidance_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: straight_level.c 777 2012-02-23 16:58:44Z murch $
 */

#include <stdlib.h>
#include <math.h>

#include "../globaldefs.h"
#include "guidance_interface.h"

static float base_pitch_cmd= 0.0872664; // 5deg

extern void get_guidance(double time, struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr){
	if (time < 25.0){
		controlData_ptr->phi_cmd = 0;
		controlData_ptr->theta_cmd = base_pitch_cmd;
	}else{
		controlData_ptr->phi_cmd = 15*D2R;
		controlData_ptr->theta_cmd = base_pitch_cmd;
	}
	}

