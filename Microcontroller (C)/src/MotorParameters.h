#ifndef MOTOR_PARAMETERS_H
#define MOTOR_PARAMETERS_H

#include "ControllerParameters.h"

// Constants obtained from parameters in ELEC3114 Lab 2
#define DC_GAIN             23.8095238095f
#define TIME_CONSTANT        0.2293332714f

#define COEFF_V             TS * DC_GAIN / (TS + TIME_CONSTANT)
#define COEFF_W             TIME_CONSTANT / (TS + TIME_CONSTANT)

#endif
