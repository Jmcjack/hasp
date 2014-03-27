/*! \file control_functions.c
 *	\brief Shared control functions source code
 *
 *	\details Shared control specific functions
 *
 *	\ingroup control_fcns
 *
 *	\author University of Minnesota
 *	\author Aerospace Engineering and Mechanics
 *	\copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: control_functions.c 756 2012-01-04 18:51:43Z murch $
 */
#include <stdlib.h>
#include <math.h>

#include "../globaldefs.h"
#include AIRCRAFT_UP1DIR
#include "control_interface.h"


extern void add_trim_bias(struct control * controlData_ptr){
	// Bias controller commands with approximate trim values (set in aircraft/XXX_config.h)				
	controlData_ptr->dthr += THROTTLE_TRIM; // throttle
	controlData_ptr->de   += ELEVATOR_TRIM; // elevator
	controlData_ptr->dr   += RUDDER_TRIM; // rudder
	controlData_ptr->da_l -= AILERON_TRIM; // left aileron
	controlData_ptr->da_r += AILERON_TRIM; // right aileron
}
