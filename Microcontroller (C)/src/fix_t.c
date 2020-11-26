/* fix_t.c

 * Fixed point number representation and operations.
 *
 * Author: Aaron Lucas
 * Date Created: 2020/04/29
 *
 * Written for the Off-World Robotics Team
 */

#include "fix_t.h"

#include <stdbool.h>

// Obtain the sign of a fixed point number.
static Sign getSign(fix_t x);

// Check whether two fixed point numbers have the same sign.
// Return true if both numbers are positive or both are negative; false
// otherwise.
static bool haveSameSign(fix_t x, fix_t y);

fix_t fixAdd(fix_t x, fix_t y) {
    // Signed value overflow is undefined so make these values unsigned before
    // adding them.
    word_t _x = x, _y = y;
    word_t result = _x + _y;

    // Overflow occurs when the sign bit of both x and y are the same, but the
    // sign of the sum is different.
    if (haveSameSign(_x, _y) && !haveSameSign(_x, result)) {
        // Overflow has occurred
        return fixOverflow;
        /*
        if (getSign(_x) == SIGN_POSITIVE)
            // Saturate at positive limit
            return fixPointMax;
        else
            // Saturate at negative limit
            return fixPointMin;
        */
    }

    return result;
}

fix_t fixSubtract(fix_t x, fix_t y) {
    // Signed value overflow is undefined so make these values unsigned before
    // subtracting them.
    word_t _x = x, _y = y;
    word_t result = _x - _y;

    // Overflow occurs when the sign bits of x and y are different and the sign
    // of the subtraction is different to x.
    if (!haveSameSign(_x, _y) && !haveSameSign(_x, result)) {
        // Overflow has occurred
        return fixOverflow;
        /*
        if (getSign(_x) == SIGN_POSITIVE)
            // Saturate at positive limit
            return fixPointMax;
        else
            // Saturate at negative limit
            return fixPointMin;
        */
    }

    return result;
}

fix_t fixMultiply(fix_t x, fix_t y) {
    // Number of left-most bits to be truncated
    static const uint8_t hiTruncBits = WORD_SIZE - Q_POINT;
    
    // Shift amount to isolate the left bits to be truncated as well as the
    // sign bit of the result (normal length word)
    static const uint8_t signBitsShift = 2 * WORD_SIZE - (hiTruncBits + 1);

    // Bit mask to select just the bits obtained from shifting `signBitsShift`,
    // ignoring other bits introduced from the shift operation.
    static const word_t signBitsMask = (1 << (hiTruncBits + 1)) - 1;

    // Compute the product in a double-length word and then shift and truncate
    // result.
    dword_t result = (dword_t)((dint_t)x * (dint_t)y);
    Sign resultSign = (result >> (2 * WORD_SIZE - 1)) ? SIGN_NEGATIVE : SIGN_POSITIVE;

    // Select all left-most bits which need to be truncated as well as the sign
    // bit of the normal-length result. These bits must be all zeros (if
    // positive result) or all ones (if negative result), otherwise the result
    // is too big to fit in a normal length word and an overflow has occurred.
    word_t hiBits = (word_t)(result >> signBitsShift);
    
    if (resultSign == SIGN_POSITIVE && hiBits != 0)
        // Positive overflow
        return fixOverflow;
    else if (resultSign == SIGN_NEGATIVE && ((~hiBits) & signBitsMask) != 0)
        // Negative overflow
        return fixOverflow;
    else
        return (fix_t)(result >> Q_POINT);

}

/* fix_t fixMultiply_preshift(fix_t x, fix_t y, uint8_t shift, ShiftDirection direction) { */
/*     return x * y; // Placeholder */
/* } */

static Sign getSign(fix_t x) {
    if (x & signMask) {
        // Sign bit is 1
        return SIGN_NEGATIVE;
    } else {
        // Sign bit is 0
        return SIGN_POSITIVE;
    }
}

static bool haveSameSign(fix_t x, fix_t y) {
    // The value x ^ y will have a sign bit of 0 if the sign bits of both
    // values (x and y separately) are the same [1 ^ 1 = 0 and 0 ^ 0 = 0]. If
    // the signs are different then x ^ y will have a sign bit of 1 [1 ^ 0 = 1
    // and 0 ^ 1 = 1]
    if (getSign(x ^ y)) {
        return false;
    } else {
        return true;
    }
}

/* #ifdef DEBUG */
float fix2float(fix_t x) {
    return (float)x / CONVERSION_FACTOR;
}
/* #endif */
