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

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

struct pidController {
    const float kp, ki, kd;
    const float setWeightB, setWeightC;
    const float filterCoeff;
    const float sampleTime, sampleFreq;
    const float outputMin, outputMax;

    const float intCoeff;
    const float derCoeff1, derCoeff2;

    volatile float *const setpoint;
    volatile float *const feedback;
    volatile float *const controlSignal;

    float integrator;
    float differentiator;
    float prevError;
};

float runControlAlgorithm(struct pidController *pid);

#endif
