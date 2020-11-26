// QEIControl.h
//
// Written By: Aaron Lucas
// Date Created: 2020/08/02
//
// Quadrature encoder interface and control module for the TM4C123GH6PM
// microcontroller.
//
// Written for the Off-World Robotics Team

// IMPORTANT DEFINITIONS:
// ----------------------------------------------------------------------------- 
// A 'pulse' refers to a pair of rising and falling edges representing a
// square-wave-like signal on one of the phase signals. For two phase signals,
// one pulse refers to one of these waveforms on each signal.
//
// An 'edge' refers to a single rising or falling transition. On a single
// phase signal, two edges represent one pulse. When using two phase signals,
// four edges represent one pulse.
// ----------------------------------------------------------------------------- 

#ifndef QEI_CONTROL_H
#define QEI_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#include "units.h"

// Representation of a QEI module, of which the TM4C123GH6PM microcontroller has
// two. Each module uses three input pins: one for an index signal and two for
// the quadrature-encoded phases.
//
// QEI module 0 has two valid pin choices for each of the inputs, while module 1
// has only a single option. Thus, QEI0 input pins are configurable but QEI1
// pins are not.
//
// NOTE:The two QEI modules are clocked from the system clock and this cannot be
// pre-divided!
//
// WARNING: Two of the pin options for QEI0 are protected NMI pins which, if
// reconfigured, may impact critical parts of the overall system. As such the
// default GPIO pins are the ones which are not used as an NMI.
enum QEIModule {
    QEI0,
    QEI1
};

// The following enums represent the options for the QEI input pins, index,
// phase A and phase B respectively. Each pin type is contained in its own enum
// space for encapsulation however the numerical values continue as if they were
// in a single enum since this makes translating each value to its GPIO pin
// constants much simpler.
//
// QEI0 has two options for each pin while QEI1 only has a single configuration.
enum QEIIndexPin {
    QEI0_IDX_D3 = 0,    // QEI0 Default
    QEI0_IDX_F4,
    QEI1_IDX_C4         // Only pin for QEI1
};

enum QEIPhaseAPin {
    QEI0_PHA_D6 = 3,    // QEI0 Default
    QEI0_PHA_F0,        // Protected NMI Pin
    QEI1_PHA_C5         // Only pin for QEI1
};

enum QEIPhaseBPin {
    QEI0_PHB_D7 = 6,    // Protected NMI Pin
    QEI0_PHB_F1,        // QEI0 Default
    QEI1_PHB_C6         // Only pin for QEI1
};

// Representation of the encoder connected to the QEI interface which is used to
// set up a module.
struct Encoder {
    uint32_t pulsesPerRev;    // Full pulses per revolution on one phase
    bool     hasIndexSignal;  // Encoder has an index signal (and use it)
    bool     swapPhases;      // Swap phase pins to correct for miswiring
};

// Filter values for the QEI phase inputs to guard against the effects of noise.
//
// The filter number is the number of system clock cycles for which the input
// must be unchanged for the module to accept it as a valid input.
enum QEIFilter {
    QEI_NO_FILTER = 0,
    QEI_FILTER_2  = 2, 
    QEI_FILTER_3, 
    QEI_FILTER_4, 
    QEI_FILTER_5, 
    QEI_FILTER_6, 
    QEI_FILTER_7, 
    QEI_FILTER_8, 
    QEI_FILTER_9, 
    QEI_FILTER_10, 
    QEI_FILTER_11, 
    QEI_FILTER_12, 
    QEI_FILTER_13, 
    QEI_FILTER_14, 
    QEI_FILTER_15, 
    QEI_FILTER_16, 
    QEI_FILTER_17,
};

// Input divider values for velocity measurement. This has no effect on position
// measurements.
//
// Given QEI_DIVIDE_n, every n'th edge will be counted in the velocity
// measurement, allowing for very high velocities to be measured and not
// overflow the 32-bit count register.
//
// QEI_DIVIDE_1 will count every phase edge (i.e. no pre-division).
enum QEIDivider {
    QEI_DIVIDE_1 = 0,
    QEI_DIVIDE_2,
    QEI_DIVIDE_4,
    QEI_DIVIDE_8,
    QEI_DIVIDE_16,
    QEI_DIVIDE_32,
    QEI_DIVIDE_64,
    QEI_DIVIDE_128
};

// Set the input pins for QEI module 0. Module 1 pins cannot be reconfigured.
//
// Does not configure any pins or peripherals, which is done in
// qeiConfigureForEncoder.
void qeiConfigureModule0Pins(enum QEIIndexPin idx, enum QEIPhaseAPin phA, enum QEIPhaseBPin phB);

// Configure the QEI module for the given encoder specifcations. Configures the
// module for position measurement only.
//
// Enables all required peripherals and configures all pins. This will override
// the behaviour of locked NMI pins if any locked pins are specified for use.
//
// This function should be called before enabling the QEI module.
void qeiConfigureForEncoder(enum QEIModule qei, struct Encoder encoder);

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
void qeiConfigureVelocityCapture(enum QEIModule qei, enum QEIDivider div, kilohertz sampleFreq);

// Apply a filter to the input signals by requiring a change in the phase inputs
// to be stable for a given number of clock cycles before being counted in the
// position or velocity measurements.
//
// The input filter is disabled by default and can also be disabled manually by
// specifying QEI_NO_FILTER.
void qeiConfigureInputFilter(enum QEIModule qei, enum QEIFilter filter);

// Enable an interrupt whenever the velocity measurement is recalculated, and
// link an interrupt handler. Allows velocity measurement to be processed with
// minimal delay.
void qeiInterruptVelocity(enum QEIModule qei, void (*handler)(void));

// Set the current position of the encoder in degrees.
//
// The encoder can only provide a fixed number of possible angles depending on
// the number of pulses per revolution so care should be take to set the angle
// to one of these values or else the angle measurements will be offset.
void qeiCalibratePosition(enum QEIModule qei, degrees angle);

// Obtain the current angular position of the encoder in degrees.
//
// This will be a multiple of 360 / (4 * pulses per revolution) as pulse edges
// on each phase will contribute to the angle measurement.
degrees qeiGetPosition(enum QEIModule qei);

// Obtain the most recently measured velocity of the encoder (in rpm) and its
// direction of rotation. This may not represent the current speed or direction
// but that which was measured in the last sample.
struct AngularVel qeiGetVelocity(enum QEIModule qei);

// Enable or disable a QEI module. This must be called after configuring the
// module for an encoder (required) and for velocity capture (optional).
//
// This will enable/disable the velocity capture functionality if applicable.
void qeiEnableModule(enum QEIModule qei, bool enable);

#endif
