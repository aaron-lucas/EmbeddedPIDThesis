// pwmTest.c
//
// Written By: Aaron Lucas
// Date Created: 2020/30/07
//
// Test program for PWMControl.
//
// Written for the Off-World Robotics Team

#include "PWMControl.h"

#include "common.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"

int main(void) {
    setSystemClock();
    pwmSetClockDivider(PWM_CLKDIV_1);

    // Configure outputs from module 0
    pwmConfigureOutput(PWM06_D0);
    pwmConfigureOutput(PWM07_D1);

    // Sets frequency for PWM07_D1 since they run from the same generator 
    // (10 kHz)
    pwmSetFrequency(PWM06_D0, 10);
    pwmSetDutyCycle(PWM06_D0, 25.0f);                           // 25% duty
    pwmSetDutyCycle(PWM07_D1, pwmGetDutyCycle(PWM06_D0)+35.0f); // 60% duty

    pwmEnableOutput(PWM06_D0, true);
    pwmEnableOutput(PWM07_D1, true);



    // Configure outputs from module 1
    pwmConfigureOutput(PWM14_F0);
    pwmConfigureOutput(PWM15_F1);

    // Sets period for PWM15_F1 since they run from the same generator (1 kHz)
    pwmSetPeriod(PWM14_F0, 1.0f);
    pwmSetDutyCycle(PWM14_F0, 0.0f);      // 0% duty (always off)
    pwmSetDutyCycle(PWM15_F1, 100.0f);    // 100% duty (always on)

    pwmEnableOutput(PWM14_F0, true);
    pwmEnableOutput(PWM15_F1, true);

    while (true);
}

