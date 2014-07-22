/*! \file doublet_rates.c
 *	\brief 1-2-1 doublet command on roll rate.
 *
 *	\details
 *	\ingroup guidance_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: doublet_rates.c 757 2012-01-04 21:57:48Z murch $
 */
#include <stdlib.h>
#include <math.h>

#include "../globaldefs.h"
#include "../system_id/systemid_interface.h"
#include "guidance_interface.h"

static float base_pitch_cmd= 0.0872664; // 5deg

extern void get_guidance(double time, struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr){
	
	controlData_ptr->phi_cmd = 0;
	controlData_ptr->theta_cmd = base_pitch_cmd;
	controlData_ptr->p_cmd = doublet121(5, time, 0.2, 0.3, 0.4 , 100*D2R);
	controlData_ptr->r_cmd = 0; // r command

}

