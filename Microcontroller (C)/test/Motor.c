#include "Motor.h"

float calculateAngularVelocity(struct motor *motor, float voltage) {
    float angularVelocity = (motor->coeffV * voltage) + (motor->coeffW * motor->angularVelocity);
    motor->angularVelocity = angularVelocity;

    return angularVelocity;
}
