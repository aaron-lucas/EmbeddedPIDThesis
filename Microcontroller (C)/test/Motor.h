#ifndef MOTOR_SIM_H
#define MOTOR_SIM_H

struct motor {
    const float dcGain;
    const float timeConstant;

    float angularVelocity;

    const float coeffV;
    const float coeffW;
};

float calculateAngularVelocity(struct motor *motor, float voltage);

#endif
