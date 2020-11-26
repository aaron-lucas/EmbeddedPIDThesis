/*
 * PIDController.c
 *
 * An implementation of a generic PID controller algorithm meant for embedded
 * use in a microcontroller.
 *
 * Features:
 *      - Constant time and memory complexity
 *      - No dynamic memory allocation
 *      - Independent of the standard library
 *      - No floating point arithmetic or division
 *      - Optimised for cycle count
 *
 * Author: Aaron Lucas
 * Date Created: 2020/04/03
 *
 * Written for the Off-World Robotics Team
 */

#include <stddef.h>

#include "PIDController.h"

float runControlAlgorithm(struct pidController *pid) {
    if (pid == NULL)
        return 0;

    float pTerm = pid->kp * (pid->setWeightB * *(pid->setpoint) - *(pid->feedback));
    float iTerm = pid->intCoeff * (*(pid->setpoint) - *(pid->feedback)) + pid->integrator;
    float swcError = pid->setWeightC * *(pid->setpoint) - *(pid->feedback);
    float dTerm = pid->derCoeff2 * (pid->differentiator + pid->derCoeff1 * (swcError - pid->prevError));

    float controlSignal = pTerm + iTerm + dTerm;

    // Saturate control signal if required
    if (controlSignal < pid->outputMin)
        controlSignal = pid->outputMin;
    else if (controlSignal > pid->outputMax)
        controlSignal = pid->outputMax;

    // Update pid states
    // -------------------------------------------------------------------------
    pid->integrator = iTerm;
    pid->differentiator = dTerm;
    pid->prevError = swcError;

    *(pid->controlSignal) = controlSignal;

    return controlSignal;
}

