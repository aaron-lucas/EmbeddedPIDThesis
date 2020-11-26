// PWMControl.c
//
// Written By: Aaron Lucas
// Date Created: 2020/07/29
//
// PWM interface and control module for the TM4C123GH6PM microcontroller.
//
// Written for the Off-World Robotics Team

#include "PWMControl.h"

#include "common.h"

#include "driverlib/gpio.h"
#include "driverlib/pwm.h"

// Number of GPIO pin outputs available in each PWM module
// This is more than the number of PWM signals which can be generated per
// module (8) as some signals can be directed to multiple GPIO pins.
#define PWMS_PER_MODULE         10

// Number of distinct PWM modules on the MCU
#define NUM_PWM_MODULES         2

// Number of GPIO pins which can output a PWM signal
#define NUM_PWM_OUTPUTS         PWMS_PER_MODULE * NUM_PWM_MODULES

// Number of different clock divider options for the PWM peripheral
#define NUM_CLOCK_DIVIDERS      7

// Default configuration settings for PWM generator blocks
// Count down for a left-aligned PWM pulse and allow PWM generators to operated
// independently.
#define PWM_GEN_CONFIG          PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC | \
                                PWM_GEN_MODE_GEN_NO_SYNC 

// Mapping between PWMOutput and the respective GPIO peripheral.
static const uint32_t gpioPeripherals[NUM_PWM_OUTPUTS] = {
    SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOB,
    SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOB,
    SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOE,
    SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD, SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD,

    SYSCTL_PERIPH_GPIOD, SYSCTL_PERIPH_GPIOD,
    SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOE,
    SYSCTL_PERIPH_GPIOF, SYSCTL_PERIPH_GPIOF,
    SYSCTL_PERIPH_GPIOF, SYSCTL_PERIPH_GPIOF
};

// Mapping between PWMOutput and the respective PWM peripheral.
static const uint32_t pwmPeripherals[NUM_PWM_MODULES] = {
    SYSCTL_PERIPH_PWM0,
    SYSCTL_PERIPH_PWM1
};

static const uint32_t gpioPinConfigs[NUM_PWM_OUTPUTS] = {
    GPIO_PB6_M0PWM0, GPIO_PB7_M0PWM1,
    GPIO_PB4_M0PWM2, GPIO_PB5_M0PWM3,
    GPIO_PE4_M0PWM4, GPIO_PE5_M0PWM5,
    GPIO_PC4_M0PWM6, GPIO_PD0_M0PWM6, GPIO_PC5_M0PWM7, GPIO_PD1_M0PWM7,

    GPIO_PD0_M1PWM0, GPIO_PD1_M1PWM1,
    GPIO_PA6_M1PWM2, GPIO_PE4_M1PWM2, GPIO_PA7_M1PWM3, GPIO_PE5_M1PWM3,
    GPIO_PF0_M1PWM4, GPIO_PF1_M1PWM5,
    GPIO_PF2_M1PWM6, GPIO_PF3_M1PWM7
};

static const uint32_t gpioBaseAddrs[NUM_PWM_OUTPUTS] = {
    GPIO_PORTB_BASE, GPIO_PORTB_BASE,
    GPIO_PORTB_BASE, GPIO_PORTB_BASE,
    GPIO_PORTE_BASE, GPIO_PORTE_BASE,
    GPIO_PORTC_BASE, GPIO_PORTD_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE,

    GPIO_PORTD_BASE, GPIO_PORTD_BASE,
    GPIO_PORTA_BASE, GPIO_PORTE_BASE, GPIO_PORTA_BASE, GPIO_PORTE_BASE,
    GPIO_PORTF_BASE, GPIO_PORTF_BASE,
    GPIO_PORTF_BASE, GPIO_PORTF_BASE
};

static const uint32_t pwmBaseAddrs[NUM_PWM_MODULES] = {
    PWM0_BASE,
    PWM1_BASE
};

static const uint32_t pwmOuts[NUM_PWM_OUTPUTS] = {
    PWM_OUT_0, PWM_OUT_1,
    PWM_OUT_2, PWM_OUT_3,
    PWM_OUT_4, PWM_OUT_5,
    PWM_OUT_6, PWM_OUT_6, PWM_OUT_7, PWM_OUT_7,

    PWM_OUT_0, PWM_OUT_1,
    PWM_OUT_2, PWM_OUT_2, PWM_OUT_3, PWM_OUT_3,
    PWM_OUT_4, PWM_OUT_5,
    PWM_OUT_6, PWM_OUT_7
};

static const uint32_t pwmOutBits[NUM_PWM_OUTPUTS] = {
    PWM_OUT_0_BIT, PWM_OUT_1_BIT,
    PWM_OUT_2_BIT, PWM_OUT_3_BIT,
    PWM_OUT_4_BIT, PWM_OUT_5_BIT,
    PWM_OUT_6_BIT, PWM_OUT_6_BIT, PWM_OUT_7_BIT, PWM_OUT_7_BIT,

    PWM_OUT_0_BIT, PWM_OUT_1_BIT,
    PWM_OUT_2_BIT, PWM_OUT_2_BIT, PWM_OUT_3_BIT, PWM_OUT_3_BIT,
    PWM_OUT_4_BIT, PWM_OUT_5_BIT,
    PWM_OUT_6_BIT, PWM_OUT_7_BIT
};

static const uint8_t gpioPins[NUM_PWM_OUTPUTS] = {
    GPIO_PIN_6, GPIO_PIN_7,
    GPIO_PIN_4, GPIO_PIN_5,
    GPIO_PIN_4, GPIO_PIN_5,
    GPIO_PIN_4, GPIO_PIN_0, GPIO_PIN_5, GPIO_PIN_1,

    GPIO_PIN_0, GPIO_PIN_1,
    GPIO_PIN_6, GPIO_PIN_4, GPIO_PIN_7, GPIO_PIN_5,
    GPIO_PIN_0, GPIO_PIN_1,
    GPIO_PIN_2, GPIO_PIN_3
};


// PWM generators control ordered pairs of outputs, so only half the number of
// outputs are needed to relate outputs to generators.
static const uint32_t pwmGenerators[NUM_PWM_OUTPUTS / 2] = {
    PWM_GEN_0,
    PWM_GEN_1,
    PWM_GEN_2,
    PWM_GEN_3, PWM_GEN_3,

    PWM_GEN_0,
    PWM_GEN_1, PWM_GEN_1,
    PWM_GEN_2,
    PWM_GEN_3
};

static const uint32_t clockDividers[NUM_CLOCK_DIVIDERS] = {
    SYSCTL_PWMDIV_1, 
    SYSCTL_PWMDIV_2, 
    SYSCTL_PWMDIV_4, 
    SYSCTL_PWMDIV_8, 
    SYSCTL_PWMDIV_16,
    SYSCTL_PWMDIV_32,
    SYSCTL_PWMDIV_64 
};

// Macros for lookup tables
// Some lookup tables have strange indexing to save space so these macros should
// be used instead of indexing the arrays manually.
#define GPIO_PERIPH(pwm)        gpioPeripherals[pwm]
#define PWM_PERIPH(pwm)         pwmPeripherals[(pwm) / PWMS_PER_MODULE]
#define GPIO_PIN_CONFIG(pwm)    gpioPinConfigs[pwm]
#define GPIO_BASE(pwm)          gpioBaseAddrs[pwm]
#define PWM_BASE(pwm)           pwmBaseAddrs[(pwm) / PWMS_PER_MODULE]
#define PWM_OUT(pwm)            pwmOuts[pwm]
#define PWM_OUT_BIT(pwm)        pwmOutBits[pwm]
#define GPIO_PIN(pwm)           gpioPins[pwm]
#define PWM_GEN(pwm)            pwmGenerators[(pwm) / 2]

// Global variable for PWM clock divider.
// All PWM sources run from the same clock source.
// Clock divider is disabled by default.
// This is equivalent to log2(divider value) and is used to convert a time to a
// number of clock ticks.
static enum PWMClockDivider clockDivider = PWM_CLKDIV_1;

// Configure the clock source for all PWM outputs by dividing the system clock
// frequency.
void pwmSetClockDivider(enum PWMClockDivider divider) {
    SysCtlPWMClockSet(clockDividers[divider]);
    clockDivider = divider;
}

// Configure a GPIO pin to become a PWM output.
//
// Enables the correct GPIO port and PWM module peripherals, configures the GPIO
// pin function and configures the PWM generator corresponding to the given PWM
// signal.
void pwmConfigureOutput(enum PWMOutput pwm) {
    // Enable all peripherals used by the PWM output (GPIO port and PWM module)
    enablePeripheral(GPIO_PERIPH(pwm));

    // PWM outputs 0-9 are in module 0 and outputs 10-19 are in module 1.
    enablePeripheral(PWM_PERIPH(pwm));

    // Some GPIO pins have their functionality locked by default.
    // Edit the LOCK and COMMIT registers to allow reconfiguration.
    if (pwm == PWM14_F0) {
        GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;  // Unlock commit register
        GPIO_PORTF_CR_R  |= 1;              // Allow pin F0 to be modified
        GPIO_PORTF_LOCK_R = 0;              // Lock commit register
    }

    // Configure GPIO pin as a PWM output
    GPIOPinConfigure(GPIO_PIN_CONFIG(pwm));
    GPIOPinTypePWM(GPIO_BASE(pwm), GPIO_PIN(pwm));

    // Configure PWM generator
    PWMGenConfigure(PWM_BASE(pwm), PWM_GEN(pwm), PWM_GEN_CONFIG);
}

// Set the frequency of a PWM signal, given in kilohertz.
//
// If the period corresponding to the given frequency is not a multiple of the
// PWM clock period then the actual pwm frequency will not match exactly due to
// quantisation errors.
//
// Since the PWM frequency is set in the PWM generators, this function will also
// change the frequency of the other PWM signal coming from the same generator.
void pwmSetFrequency(enum PWMOutput pwm, kilohertz frequency) {
    pwmSetPeriod(pwm, 1.0f / frequency);  // Set period in milliseconds
}

// Get the pulse frequency of the given PWM output.
//
// This may be slightly
// different to the set value as the actual period is an integer number of clock
// cycles.
kilohertz pwmGetFrequency(enum PWMOutput pwm) {
    return 1.0 / pwmGetPeriod(pwm);  // Get period in milliseconds
}

// Set the period of a PWM signal, given in milliseconds.
//
// If the given period is not a multiple of the PWM clock period then the
// actual period of the PWM signal will not match exactly due to quantisation
// errors.
//
// Since the PWM period is set in the PWM generators, this function will also
// change the period of the other PWM signal coming from the same generator.
void pwmSetPeriod(enum PWMOutput pwm, milliseconds period) {
    uint32_t clockTicks = (uint32_t)(period * SYS_CLOCK_FREQ_KHZ) >> clockDivider;
    PWMGenPeriodSet(PWM_BASE(pwm), PWM_GEN(pwm), clockTicks);
}

// Get the pulse period of the given PWM output.
//
// This may be slightly different to the set value as the actual period is an
// integer number of clock cycles.
milliseconds pwmGetPeriod(enum PWMOutput pwm) {
    uint32_t clockTicks = PWMGenPeriodGet(PWM_BASE(pwm), PWM_GEN(pwm));
    return (clockTicks << clockDivider) / SYS_CLOCK_FREQ_KHZ;
}

// Set the duty cycle of a PWM signal, which must be between 0 and 100
// (inclusive).
//
// Rounding errors may cause the actual duty cycle to slightly differ from the
// given duty cycle, with the duration of the high pulse being rounded to the
// nearest multiple of the PWM clock period.
//
// Each PWM output can have its own independent duty cycle, they are not tied to
// generator blocks.
void pwmSetDutyCycle(enum PWMOutput pwm, percent duty) {
    // A duty cycle of zero acts as a 100% duty cycle so invert the output
    // signal.
    bool invert = (duty == 0);
    PWMOutputInvert(PWM_BASE(pwm), PWM_OUT_BIT(pwm), invert);

    uint32_t clockTicks = PWMGenPeriodGet(PWM_BASE(pwm), PWM_GEN(pwm));
    clockTicks = (uint32_t)((float)clockTicks * duty / 100.0f);

    PWMPulseWidthSet(PWM_BASE(pwm), PWM_OUT(pwm), clockTicks);
}

// Get the duty cycle of the given PWM output, rounded to the nearest 0.1%.
// This may be significantly different to the duty cycle that was previously set
// if the PWM period is very low.
percent pwmGetDutyCycle(enum PWMOutput pwm) {
    /* // If signal is inverted then the duty cycle is 0 */
    /* if (PWM_BASE(pwm) == PWM0_BASE && (PWM0_INVERT_R & PWM_OUT_BIT(pwm)) || */
    /*     PWM_BASE(pwm) == PWM1_BASE && (PWM1_INVERT_R & PWM_OUT_BIT(pwm))) */
    /*     return 0; */

    uint32_t clockTicks = PWMPulseWidthGet(PWM_BASE(pwm), PWM_OUT(pwm));
    return (float)(clockTicks * 100) / (float)PWMGenPeriodGet(PWM_BASE(pwm), PWM_GEN(pwm));
}

// Enable or disable the given PWM output.
// Enables the PWM generator if enabling the PWM output and disables the
// generator if both PWM outputs are disabled.
void pwmEnableOutput(enum PWMOutput pwm, bool enable) {
    // Select the two PWM signals corresponding to the generator block
    uint8_t genPwmA = (pwm & ~1);     // Round down to even number
    uint8_t genPwmB = (genPwmA + 1);  // Next number forms the generator pair
    uint32_t pwmEnableMask = PWM_OUT_BIT(genPwmA) | PWM_OUT_BIT(genPwmB);

    // Enable/disable the PWM output to the GPIO pin
    PWMOutputState(PWM_BASE(pwm), PWM_OUT_BIT(pwm), enable);
    
    // Enable the corresponding generator if enabling an output.
    // Disable the generator if disabling the output and the other output from
    // the generator is also disabled.
    if (enable)
        PWMGenEnable(PWM_BASE(pwm), PWM_GEN(pwm));
    else if ((pwm <  PWMS_PER_MODULE && !(PWM0_ENABLE_R & pwmEnableMask)) ||
             (pwm >= PWMS_PER_MODULE && !(PWM1_ENABLE_R & pwmEnableMask)))
        PWMGenDisable(PWM_BASE(pwm), PWM_GEN(pwm));
}
