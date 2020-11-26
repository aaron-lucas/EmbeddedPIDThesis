#ifndef CONTROLLER_PARAMETERS_H
#define CONTROLLER_PARAMETERS_H

// PID Gains
/* #define KP                  0.324f */
/* #define KI                  11.6096f */
/* #define KD                  -0.0022f */

#define KP                  0.0165f
#define KI                  1.6452f
#define KD                  0.0f

/* #define KP                  0.0268f */
/* #define KI                  1.1415f */
/* #define KD                  0.0f */

// Setpoint weights
#define SW_B                1.0f
#define SW_C                1.0f

// Filter coefficient
/* #define N                   30.2022f */
#define N                   100.0f

// Sampling frequency
#define FS                  50.0f

// Saturation limits
#define OUTPUT_MIN          -12.0f
#define OUTPUT_MAX          12.0f

// Internal controller coefficients
#define INT_COEFF           KI * TS
#define DER_COEFF1          KD * N
#define DER_COEFF2          1.0f/(1.0f + N * TS)

// Sample time
#define TS                  1.0f / FS

#endif
