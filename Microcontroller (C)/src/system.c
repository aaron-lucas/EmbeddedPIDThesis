/*
 * system.c
 *
 * The main PID control system which incorporates the PID controller itself,
 * along with the PWM module for output and the quadrature encoder module for
 * input feedback.
 *
 * Author: Aaron Lucas
 * Date Created: 2020/09/19
 */

#include "common.h"

#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/qei.h"

#include "PIDController.h"
#include "PWMControl.h"
#include "QEIControl.h"

#include "units.h"
#include "ControllerParameters.h"

#define ZERO 0.0f

static void setupGPIO(void);
static void setupPWM(void);
static void setupQEI(void);

static void qei_isr(void);

volatile float setpointReg, feedbackReg, controlReg;

struct pidController *pid;
struct Encoder *encoder;

int main(void) {
    setSystemClock();
    enableFPU();

    // PID Controller initialisation
    struct pidController _pid = {
        .kp = KP, .ki = KI, .kd = KD,
        .setWeightB = SW_B, .setWeightC = SW_C,
        .filterCoeff = N,
        .sampleTime = TS, .sampleFreq = FS,
        .outputMin = OUTPUT_MIN, .outputMax = OUTPUT_MAX,

        .intCoeff = INT_COEFF,
        .derCoeff1 = DER_COEFF1, .derCoeff2 = DER_COEFF2,
        
        .setpoint = &setpointReg,
        .feedback = &feedbackReg,
        .controlSignal = &controlReg,

        .integrator = ZERO,
        .differentiator = ZERO,
        .prevError = ZERO
    };

    setpointReg = 10.0f;
    pid = &_pid;

    struct Encoder _encoder = {
        .pulsesPerRev = 1366,
        .hasIndexSignal = false,
        .swapPhases = false
    };

    encoder = &_encoder;
        
    setupGPIO();
    setupPWM();
    setupQEI();


    // Pin 7 on port A will determine the setpoint of the controller which can
    // be used for measuring the step response of the system.
    while (true) {
        if (GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7))
            setpointReg = 20;
        else
            setpointReg = 10;
    }
    
    return 0;
}

static void setupGPIO(void) {
    enablePeripheral(SYSCTL_PERIPH_GPIOA);

    // Pin A6 is a timing pin used to measure the calculation time of the
    // controller
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0);

    // Pin A7 is for setpoint adjustment
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
}

static void setupPWM(void) {
    pwmSetClockDivider(PWM_CLKDIV_8);
    pwmConfigureOutput(PWM00_B6);
    /* pwmSetPeriod(PWM00_B6, 10.0f);          // 10ms period */
    pwmSetFrequency(PWM00_B6, 0.1f);
    pwmSetDutyCycle(PWM00_B6, 15.0f);       // 1.5ms pulse for neutral setting
    pwmEnableOutput(PWM00_B6, true);
}

static void setupQEI(void) {
    qeiConfigureForEncoder(QEI1, *encoder);
    qeiConfigureVelocityCapture(QEI1, QEI_DIVIDE_1, hzToKhz(FS));
    qeiInterruptVelocity(QEI1, qei_isr);
    // QEI1_CTL_R |= QEI_CTL_STALLEN;  // Stop quadrature module when at a
    // breakpoint when debugging
    qeiEnableModule(QEI1, true);
}

static void qei_isr(void) {
    // Clear velocity timer interrupt flag
    QEIIntClear(QEI1_BASE, QEI_INTTIMER);

    // Toggle timing pin to indicate calculation process has started
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);

    // Receive feedback from quadrature module
    struct AngularVel velocity = qeiGetVelocity(QEI1);
    feedbackReg = velocity.speed * velocity.direction;

    // Calculate new PID control output
    runControlAlgorithm(pid);

    // Map output to 1.0-2.0ms pulse length where 1.5ms is neutral
    // Assumes PID output max and min values have the same magnitude
    percent duty = controlReg / pid->outputMax * 100.0f / 20.0f + 15.0f;
    pwmSetDutyCycle(PWM00_B6, duty);

    // Toggle timing pin to indicate end of calculation process
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0x00);

}
