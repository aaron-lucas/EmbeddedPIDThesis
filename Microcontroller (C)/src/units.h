// units.h
//
// Written By: Aaron Lucas
// Date Created: 2020/08/06
//
// Definition of commonly used units with conversion functions.
//
// Written for the Off-World Robotics Team.

#ifndef UNITS_H
#define UNITS_H

#define _USE_MATH_DEFINES
#include <math.h>

// M_PI is not guaranteed to be defined in the math library
#ifndef M_PI
#define M_PI 3.1415926535897932f
#endif

// Time units
typedef float seconds;
typedef float milliseconds;

// Frequency units
typedef float hertz;
typedef float kilohertz;

// Angular units
typedef float degrees;
typedef float radians;

typedef float rpm;
typedef float radPerSec;

// Rotation directions
// Set numerical associations to be consistent with that returned by the
// Tivaware QEI peripheral's QEIDirectionGet function
enum RotateDir {
    ANTICLOCKWISE = -1,
    NO_ROTATION   =  0,
    CLOCKWISE     =  1,
};

struct AngularVel {
    enum RotateDir  direction;
    rpm             speed;
};

// Other units
typedef float percent;


// Unit conversion
static inline milliseconds secToMillisec(seconds time) { return time * 1000.0f; }
static inline seconds millisecToSec(milliseconds time) { return time / 1000.0f; }

static inline hertz khzToHz(kilohertz freq) { return freq * 1000.0f; }
static inline kilohertz hzToKhz(hertz freq) { return freq / 1000.0f; }

static inline degrees radToDeg(radians angle) { return angle * 180.0f / M_PI; }
static inline radians degToRad(degrees angle) { return angle * M_PI / 180.0f; }

static inline rpm radPerSecToRpm(radPerSec vel) { return vel * 30.0f / M_PI; }
static inline radPerSec rpmToRadPerSec(rpm vel) { return vel * M_PI / 30.0f; }

#endif
