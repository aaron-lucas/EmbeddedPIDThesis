// QEIControl.c
//
// Written By: Aaron Lucas
// Date Created: 2020/08/02
//
// Quadrature encoder interface and control module for the TM4C123GH6PM
// microcontroller.
//
// Written for the Off-World Robotics Team

#include "QEIControl.h"

#include "common.h"
#include "units.h"
#include "driverlib/qei.h"
#include "driverlib/gpio.h"
#include <math.h>

// Default base configuration for all QEI modules
#define QEI_CONFIG          QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_QUADRATURE

#define EDGES_PER_PULSE     4   // Number of transition edges per pulse (for both phases)

#define NUM_QEI_MODULES     2

#define NUM_QEI_PINS        3   // Number of pins used by a QEI module
#define OPTIONS_PER_PIN     3   // Number of GPIO options per QEI pin type

// Total number of pins which have QEI functionality
#define NUM_PIN_OPTIONS     NUM_QEI_PINS * OPTIONS_PER_PIN

#define NUM_VEL_DIVIDERS   8   // Number of velocity divider options

static const uint32_t  secondsPerMin = 60;
static const degrees   degreesPerRev = 360.0f;

// Data storage entity containing information required for each QEI module to
// function.
//
// Contains encoder information (pulses per revolution), operational information
// (sample frequency, velocity divider and input filter) and pin configuration.
struct QEIModuleData {
    uint32_t            pulsesPerRev;

    bool                measureVelocity;
    kilohertz           sampleFrequency;
    enum QEIDivider     divider;
    enum QEIFilter      filter;

    enum QEIIndexPin    idxPin;
    enum QEIPhaseAPin   phAPin;
    enum QEIPhaseBPin   phBPin;
};

// Define default module data. This configuration cannot be used as-is as
// information about the encoder is required. This global data is modified as
// the modules are configured.
//
// Default settings are as follows:
//   * Velocity divider and input filters are disabled
//   * Velocity measurement is disabled
//   * GPIO pins for QEI0 are not the NMI pins
//   * GPIO pins for QEI1 are the only available options
static struct QEIModuleData qeiModuleData[NUM_QEI_MODULES] = {
    { 0, false, 0.0f, QEI_DIVIDE_1, QEI_NO_FILTER, 
                QEI0_IDX_D3, QEI0_PHA_D6, QEI0_PHB_F1 },

    { 0, false, 0.0f, QEI_DIVIDE_1, QEI_NO_FILTER, 
                QEI1_IDX_C4, QEI1_PHA_C5, QEI1_PHB_C6 }
};

// =============================================================================
// The following arrays behave as lookup tables which map QEI constants defined
// in the header file to constants used by the Tivaware peripheral library.
// =============================================================================

// Maps QEI module to its base address in memory
static const uint32_t qeiBaseAddrs[NUM_QEI_MODULES] = {
    QEI0_BASE,
    QEI1_BASE
};

// Maps velocity dividers to their tivaware representations
static const uint32_t qeiVelDividers[NUM_VEL_DIVIDERS] = {
    QEI_VELDIV_1,
    QEI_VELDIV_2,
    QEI_VELDIV_4,
    QEI_VELDIV_8,
    QEI_VELDIV_16,
    QEI_VELDIV_32,
    QEI_VELDIV_64,
    QEI_VELDIV_128
};

// Maps QEI module to its system peripheral
static const uint32_t qeiPeripherals[NUM_QEI_MODULES] = {
    SYSCTL_PERIPH_QEI0,
    SYSCTL_PERIPH_QEI1
};

// Maps QEI pins to their QEI configuration setting
// Enums QEIIndexPin, QEIPhaseAPin and QEIPhaseBPin can index this array
static const uint32_t gpioPinConfigs[NUM_PIN_OPTIONS] = {
    GPIO_PD3_IDX0,
    GPIO_PF4_IDX0,
    GPIO_PC4_IDX1,

    GPIO_PD6_PHA0,
    GPIO_PF0_PHA0,
    GPIO_PC5_PHA1,

    GPIO_PD7_PHB0,
    GPIO_PF1_PHB0,
    GPIO_PC6_PHB1
};

// Maps QEI pins to their GPIO system peripheral
// Order of GPIO ports for each pin type is the same (D, F, C), so these can be
// mapped with a modulo 3 operator to save memory space.
static const uint32_t gpioPeripherals[OPTIONS_PER_PIN] = {
    SYSCTL_PERIPH_GPIOD,
    SYSCTL_PERIPH_GPIOF,
    SYSCTL_PERIPH_GPIOC,
};

// Maps QEI pins to their GPIO port base address in memory
// Order of GPIO ports for each pin type is the same (D, F, C), so these can be
// mapped with a modulo 3 operator to save memory space.
static const uint32_t gpioBaseAddrs[OPTIONS_PER_PIN] = {
    GPIO_PORTD_BASE,
    GPIO_PORTF_BASE,
    GPIO_PORTC_BASE,
};

// Maps QEI pins to their GPIO pin number in a bit-packed representation
static const uint8_t gpioPins[NUM_PIN_OPTIONS] = {
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_4,

    GPIO_PIN_6,
    GPIO_PIN_0,
    GPIO_PIN_5,

    GPIO_PIN_7,
    GPIO_PIN_1,
    GPIO_PIN_6
};

// Macros for lookup tables
// Some lookup tables have strange indexing to save space so these macros should
// be used instead of indexing the arrays manually.
#define QEI_DATA(qei)               qeiModuleData[qei]

#define QEI_BASE(qei)               qeiBaseAddrs[qei]
#define QEI_DIVIDER(qei)            qeiVelDividers[qei]
#define QEI_PERIPH(qei)             qeiPeripherals[qei]

#define GPIO_PIN_CONFIG(pin)        gpioPinConfigs[pin]
#define GPIO_PERIPH(pin)            gpioPeripherals[pin % OPTIONS_PER_PIN]
#define GPIO_BASE(pin)              gpioBaseAddrs[pin % OPTIONS_PER_PIN]
#define GPIO_PIN(pin)               gpioPins[pin]

// Configure a GPIO pin for use by a QEI module.
// Enable the GPIO peripheral and configure the pin to enable its QEI
// functionality, unlocking it if necessary.
static void configureGPIOPin(uint32_t pin);

// Set the input pins for QEI module 0. Module 1 pins cannot be reconfigured.
//
// Does not configure any pins or peripherals, which is done in
// qeiConfigureForEncoder.
void qeiConfigureModule0Pins(enum QEIIndexPin idx, enum QEIPhaseAPin phA, enum QEIPhaseBPin phB) {
    QEI_DATA(QEI0).idxPin = idx;
    QEI_DATA(QEI0).phAPin = phA;
    QEI_DATA(QEI0).phBPin = phB;
}

// Configure the QEI module for the given encoder specifcations. Configures the
// module for position measurement only.
//
// Enables all required peripherals and configures all pins. This will override
// the behaviour of locked NMI pins if any locked pins are specified for use.
//
// This function should be called before enabling the QEI module.
void qeiConfigureForEncoder(enum QEIModule qei, struct Encoder encoder) {
    struct QEIModuleData *data = &QEI_DATA(qei);

    uint32_t config = QEI_CONFIG;
    config |= encoder.hasIndexSignal ? QEI_CONFIG_RESET_IDX : QEI_CONFIG_NO_RESET;
    config |= encoder.swapPhases ? QEI_CONFIG_SWAP : QEI_CONFIG_NO_SWAP;

    data->pulsesPerRev = encoder.pulsesPerRev;

    // Enable peripherals and setup gpio pins
    enablePeripheral(QEI_PERIPH(qei));

    configureGPIOPin(data->idxPin);
    configureGPIOPin(data->phAPin);
    configureGPIOPin(data->phBPin);

    // Counter should load 1 less than number of edges when moving in the
    // reverse direction from position 0.
    QEIConfigure(QEI_BASE(qei), config, data->pulsesPerRev * EDGES_PER_PULSE - 1);
}

// Allows the QEI module to take velocity measurements.
//
// How often the velocity is measured is set by sampleFreq, which is given in
// kilohertz. This value must be set carefully as setting it too high may
// overflow the 32-bit edge counter but setting it too low will limit the
// resolution of the velocity measurement, especially if the encoder has a small
// number of pulses per revolution.
//
// For encoders with a high pulse count or a low sample frequency, the
// pre-divider can be used to reduce the number of edges which are measured.
//
// This function should be called before enabling the QEI module (if velocity
// functionality is required).
//
// NOTE: The timer which controls the velocity sampling rate is clocked from the
// system clock and this source cannot be changed. The minimum allowable sample
// frequency is the system clock frequency divided by (2^32 - 1) (in Hz). The
// largest sampling frequency is one quarter of the system clock frequency.
void qeiConfigureVelocityCapture(enum QEIModule qei, enum QEIDivider div, kilohertz sampleFreq) {
    uint32_t edges = SYS_CLOCK_FREQ_KHZ / sampleFreq;
    QEIVelocityConfigure(QEI_BASE(qei), QEI_DIVIDER(qei), edges);

    QEI_DATA(qei).measureVelocity = true;
    QEI_DATA(qei).divider = div;
    QEI_DATA(qei).sampleFrequency = sampleFreq;
}

// Apply a filter to the input signals by requiring a change in the phase inputs
// to be stable for a given number of clock cycles before being counted in the
// position or velocity measurements.
//
// The input filter is disabled by default and can also be disabled manually by
// specifying QEI_NO_FILTER.
void qeiConfigureInputFilter(enum QEIModule qei, enum QEIFilter filter) {
    if (filter == QEI_NO_FILTER)
        QEIFilterDisable(QEI_BASE(qei));
    else
        QEIFilterConfigure(QEI_BASE(qei), filter);

    QEI_DATA(qei).filter = filter;
}

// Enable an interrupt whenever the velocity measurement is recalculated, and
// link an interrupt handler. Allows velocity measurement to be processed with
// minimal delay.
void qeiInterruptVelocity(enum QEIModule qei, void (*handler)(void)) {
    QEIIntEnable(QEI_BASE(qei), QEI_INTTIMER);
    QEIIntRegister(QEI_BASE(qei), handler);
}

// Set the current position of the encoder in degrees.
//
// The encoder can only provide a fixed number of possible angles depending on
// the number of pulses per revolution so care should be take to set the angle
// to one of these values or else the angle measurements will be offset.
void qeiCalibratePosition(enum QEIModule qei, degrees angle) {
    uint32_t edges = angle / degreesPerRev * (QEI_DATA(qei).pulsesPerRev * EDGES_PER_PULSE);
    QEIPositionSet(QEI_BASE(qei), edges);
}

// Obtain the current angular position of the encoder in degrees.
//
// This will be a multiple of 360 / (4 * pulses per revolution) as pulse edges
// on each phase will contribute to the angle measurement.
degrees qeiGetPosition(enum QEIModule qei) {
    uint32_t edges = QEIPositionGet(QEI_BASE(qei));
    return (float)edges * degreesPerRev / (QEI_DATA(qei).pulsesPerRev * EDGES_PER_PULSE);
}

// Obtain the most recently measured velocity of the encoder (in rpm) and its
// direction of rotation. This may not represent the current speed or direction
// but that which was measured in the last sample.
struct AngularVel qeiGetVelocity(enum QEIModule qei) {
    struct QEIModuleData data = QEI_DATA(qei);

    struct AngularVel velocity = {
        .direction = NO_ROTATION,
        .speed = 0.0f
    };

    // Return safe value if velocity capture is not configured
    if (!data.measureVelocity) {
        return velocity;
    }

    // QEIDirectionGet returns 1 for 'forward', -1 for 'backward' motion, which
    // converts to CLOCKWISE and ANTICLOCKWISE respectively
    velocity.direction = (enum RotateDir)QEIDirectionGet(QEI_BASE(qei));
    
    // Calculate speed in two steps, the first of which involves integer
    // calculations only.
    uint32_t ticks = QEIVelocityGet(QEI_BASE(qei));
    uint32_t intCalc = ticks * secondsPerMin;

    rpm speed = (float)intCalc * khzToHz(data.sampleFrequency) / (2.0 * (float)data.pulsesPerRev);
    velocity.speed = speed;

    // Don't compare float directly to 0
    if (speed < 1E-6) {
        velocity.direction = NO_ROTATION;
    }

    return velocity;
}   

// Enable or disable a QEI module. This must be called after configuring the
// module for an encoder (required) and for velocity capture (optional).
//
// This will enable/disable the velocity capture functionality if applicable.
void qeiEnableModule(enum QEIModule qei, bool enable) {
    if (enable) {
        QEIEnable(QEI_BASE(qei));
        if (QEI_DATA(qei).measureVelocity) {
            QEIVelocityEnable(QEI_BASE(qei));
        }
    } else {
        QEIDisable(QEI_BASE(qei));
        if (QEI_DATA(qei).measureVelocity) {
            QEIVelocityDisable(QEI_BASE(qei));
        }
    }
}

// Configure a GPIO pin for use by a QEI module.
// Enable the GPIO peripheral and configure the pin to enable its QEI
// functionality, unlocking it if necessary.
static void configureGPIOPin(uint32_t pin) {
    enablePeripheral(GPIO_PERIPH(pin));

    // Unlock NMI pin if required
    if (pin == QEI0_PHA_F0 || pin == QEI0_PHB_D7)
        GPIOUnlockPin(GPIO_BASE(pin), GPIO_PIN(pin));

    GPIOPinConfigure(GPIO_PIN_CONFIG(pin));
    GPIOPinTypeQEI(GPIO_BASE(pin), GPIO_PIN(pin));
}
