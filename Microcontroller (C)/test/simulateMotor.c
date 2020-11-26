#include "common.h"

#include "driverlib/timer.h"
#include "driverlib/gpio.h"

#include "PIDController.h"
#include "Motor.h"
#include "PWMControl.h"

#include "units.h"
#include "MotorParameters.h"
#include "ControllerParameters.h"

#define ZERO 0.0f

static void setupGPIO(void);
static void setupPWM(void);
static void setupTimer(void);

static void timer_isr(void);

volatile float setpointReg, feedbackReg, controlReg;

struct pidController *pid;
struct motor *motor;

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

    // Motor simulator initialisation
    struct motor _motor = {
        .dcGain = DC_GAIN,
        .timeConstant = TIME_CONSTANT,

        .angularVelocity = ZERO,

        .coeffV = COEFF_V,
        .coeffW = COEFF_W
    };

    pid = &_pid;
    motor = &_motor;
        
    setupGPIO();
    setupPWM();
    setupTimer();

    setpointReg = 100.0f;

    while (true);
    
    return 0;
}

static void setupTimer(void) {
    enablePeripheral(SYSCTL_PERIPH_TIMER0);   

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); //| TIMER_CFG_A_ACT_TOINTD);
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);

    TimerLoadSet(TIMER0_BASE, TIMER_A, 50000);  // 50000 clock cycles @ 50MHz = 1ms

    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer_isr);

    TimerEnable(TIMER0_BASE, TIMER_A);
}

static void setupGPIO(void) {
    enablePeripheral(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);
}

static void setupPWM(void) {
    pwmSetClockDivider(PWM_CLKDIV_1);
    pwmConfigureOutput(PWM00_B6);
    pwmSetFrequency(PWM00_B6, 10.0f);
    pwmSetDutyCycle(PWM00_B6, 0.0f);
    pwmEnableOutput(PWM00_B6, true);
}


static void timer_isr(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_PIN_7);
    feedbackReg = calculateAngularVelocity(motor, controlReg);

    runControlAlgorithm(pid);

    percent duty = controlReg / pid->outputMax * 100.0f;
    pwmSetDutyCycle(PWM00_B6, duty);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7, 0x00);

}
