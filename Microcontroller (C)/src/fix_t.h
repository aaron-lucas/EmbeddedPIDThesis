/* fix_t.h
 *
 * Definitions for fixed point number representation and operations.
 *
 * Author: Aaron Lucas
 * Date Created: 2020/04/24
 *
 * Written for the Off-World Robotics Team
 */

#ifndef FIX_POINT_T_H
#define FIX_POINT_T_H

#include <stdint.h>

// ============================================================================
// Note:
//
// If changing the format of the fixed point representation such as the number
// of bits or location of the decimal point then the following definitions must
// be changed.

// Number of fraction bits in fixed point numbers
#define Q_POINT 14

// Number of bits in fixed point number
#define WORD_SIZE 32

// Conversion factor to turn a constant number into its fixed point
// representation. Equal to 2^Q_POINT which when multiplying, has the same
// effect as bit shifting Q_POINT bits to the left.  Used in the FIX_POINT(x) macro
// and is a floating point type to allow conversion of decimal values.
#define CONVERSION_FACTOR 16384.0

// Fixed point number type.
// The fix_t type is only for fixed point number representation. Any other data
// should use the word_t type or int_t type.
typedef uint32_t word_t;
typedef int32_t fix_t;

// Double-length datatype for multiplication
typedef uint64_t dword_t;
typedef int64_t dint_t;

// ============================================================================
// The following definitions do NOT need to be modified
// ============================================================================
typedef enum sign {
    SIGN_POSITIVE,
    SIGN_NEGATIVE
} Sign;


// Macro to convert constant values (integer and floating point) to fixed point
// representation. Multiplying a constant by the CONVERSION_FACTOR performs an
// effective bit shift of Q_POINT bits and adding/subtracting 0.5 will cause
// the correct rounding to occur when cast to a fix_t type, which is dependent
// on the sign of the number (positive of negative).
//
// Decimals with non-terminating binary representation will be rounded NOT
// truncated.
#define FIX_POINT(x) (fix_t)(                                   \
                              ((x) >= 0)                        \
                              ? ((x) * CONVERSION_FACTOR + 0.5) \
                              : ((x) * CONVERSION_FACTOR - 0.5) \
                            )

// Bitmask selecting the sign bit (left-most bit) of a number.
// This is a bitmask so it is unsigned.
static const word_t signMask = (word_t)1 << (WORD_SIZE - 1);

// Define largest possible representable value
// All bits except the sign bit are 1
// static const fix_t fixPointMax = (fix_t)(~signMask);

// Define smallest possible representable value
// Only the sign bit is 1
// static const fix_t fixPointMin = (fix_t)signMask;

// Constant representing an overflow has occurred in a fixed-point operation.
// Defined as the most negative possible value which is least likely to be
// legitimately calculated.
static const fix_t fixOverflow = (fix_t)0 | signMask;

// ============================================================================
// Function Definitions
// ============================================================================

fix_t fixAdd(fix_t x, fix_t y);

fix_t fixSubtract(fix_t x, fix_t y);

fix_t fixMultiply(fix_t x, fix_t y);

/* #ifdef DEBUG */
float fix2float(fix_t x);
/* #endif */

#endif
