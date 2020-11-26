#include "QEIControl.h"

#include "common.h"
#include "driverlib/qei.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"

// This module can perform two tests, one for position which will output a fixed
// number of pulses, and one for velocity which will continuously output pulses.
// The following definition activates the position test. Comment it out for the
// velocity test.
#define TEST_POSITION

// Interrupt Service Routines
static void qei_isr(void);
static void pwm_isr(void);

// Peripheral Setup Functions
static void setupQEI(void);
static void setupPulseGenerator(void);
static void setupIndexOutput(void);

// Globals
volatile struct AngularVel velocity = { 0 };
volatile degrees position = 0.0f;
volatile uint32_t idxPulseCounter = 0;
volatile uint32_t edgeCounter = 0;

// Constants
#ifdef TEST_POSITION
static const uint32_t maxEdges = 49;
static const uint32_t edgeInterrupts = PWM_INT_CNT_ZERO | PWM_INT_CNT_AU |
                                       PWM_INT_CNT_LOAD | PWM_INT_CNT_AD;
#endif

static const struct Encoder encoder = {
    .pulsesPerRev = 12,
    .hasIndexSignal = true,
    .swapPhases = true
};



int main(void) {
    setSystemClock();

    setupPulseGenerator();
    setupIndexOutput();     // Run after setupPulseGenerator - doesn't enable peripherals
    setupQEI();

    while (true);
}

static void qei_isr(void) {
    QEIIntClear(QEI1_BASE, QEI_INTTIMER);
    velocity = qeiGetVelocity(QEI1);
    position = qeiGetPosition(QEI1);
}

static void pwm_isr(void) {
    // Check if index pulse should be generated first so there is minimal delay
    // between the channel edge and the index pulse
    if (PWM0_0_ISC_R & PWM_INT_CNT_AU) {
        // Start of new pulse, generate index signal if 1 revolution has been
        // performed
        if (idxPulseCounter == encoder.pulsesPerRev) {
            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
            idxPulseCounter = 0;
        } else {
            idxPulseCounter++;
        }
#ifndef TEST_POSITION
        // Interrupt will be cleared later if testing velocity
        PWMGenIntClear(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_AU);
#endif

        for (int i = 0; i < 100; i++);
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);
    } 

#ifdef TEST_POSITION
    if (PWM0_0_ISC_R & edgeInterrupts) {
        PWMGenIntClear(PWM0_BASE, PWM_GEN_0, edgeInterrupts);
        // End of pulse, increment pulse counter and stop PWM output if
        // necessary
        if (++edgeCounter >= maxEdges) {
            PWMGenDisable(PWM0_BASE, PWM_GEN_0);
        }
    }
#endif

}

static void setupQEI(void) {
    qeiConfigureForEncoder(QEI1, encoder);
    qeiConfigureVelocityCapture(QEI1, QEI_DIVIDE_1, 0.01);
    qeiInterruptVelocity(QEI1, qei_isr);
    qeiCalibratePosition(QEI1, 0.0f);
    qeiEnableModule(QEI1, true);

}

static void setupPulseGenerator(void) {
    // Use the PWM hardware to generate pulses which simulate the output of a
    // quadrature encoder.
    SysCtlPWMClockSet(SYSCTL_PWMDIV_4);

    // Enable GPIO port and PWM module
    enablePeripheral(SYSCTL_PERIPH_GPIOB);
    enablePeripheral(SYSCTL_PERIPH_PWM0);

    // Set pin functions to PWM output
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinConfigure(GPIO_PB7_M0PWM1);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    // Run both pulses from the same generator, but have the outputs toggle at
    // different points
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    // Interrupt to count the number of pulses and generate index signal
    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);

#ifdef TEST_POSITION
    // Interrupt on every transition on both channels to count number of edges
    PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, edgeInterrupts);
#else
    // Interrupt on the final transition of a pulse (falling edge of channel B)
    PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_AU);
#endif

    PWMGenIntRegister(PWM0_BASE, PWM_GEN_0, pwm_isr);

    // Prescaler set to 4 so only need a quarter of normal ticks
    const uint32_t periodTicks = (((uint32_t)SYS_CLOCK_FREQ_KHZ * 1000) / 120 / 4);

    // periodTicks will not fit in a 16-bit integer but half the value will
    // Only half of period ticks should be loaded since the counter will count
    // up and down.
    PWM0_0_LOAD_R = (uint16_t)(periodTicks / 2);
    PWM0_0_CMPA_R = periodTicks / 4;  // Allow for events 1/4 way through a period

    // Channel A will toggle on count matches (halfway between 0 and the LOAD
    // value) and channel B will toggle on load matches and zero matches to
    // generate two 50% duty cycle square waves which are 90 degrees out of
    // phase.
    PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO;
    PWM0_0_GENB_R = PWM_0_GENB_ACTLOAD_ONE | PWM_0_GENB_ACTZERO_ZERO;

    // Start the counters and output the signals
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT, true);
}

static void setupIndexOutput(void) {
    // Use GPIO B5 for the index signal as the GPIO port B peripheral is already
    // enabled and pins 6 and 7 are used for the PWM output.
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);

    // Index signal is low by default until a full revolution has occurred
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);
}
