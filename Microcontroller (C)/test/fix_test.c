/* fix_test.c
 * Fixed point library tests
 *
 * Author: Aaron Lucas
 * Date Created: 2020/05/12
 *
 * Written for the Off-World Robotics Team.
 */

#include "fix_t.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// #define DEBUG_TOOLS

#ifdef DEBUG_TOOLS

#include <math.h>
static void fixPrint(fix_t x);

#endif

static void test_FIX_POINT(void);
static void test_fixAdd(void);
static void test_fixSubtract(void);
static void test_fixMultiply(void);

int main(void) {
    printf("Testing FIX_POINT() ... ");
    test_FIX_POINT();
    printf("Done!\n");

    printf("Testing fixAdd() ... ");
    test_fixAdd();
    printf("Done!\n");

    printf("Testing fixSubtract() ... ");
    test_fixSubtract();
    printf("Done!\n");

    printf("Testing fixMultiply() ... ");
    test_fixMultiply();
    printf("Done!\n");

    printf("\nAll tests completed successfully!\n");

    return EXIT_SUCCESS;
}

static void test_FIX_POINT(void) {
    fix_t smallPositive = FIX_POINT(1);
    /* assert((word_t)smallPositive == 0x00000100); */
    assert((word_t)smallPositive == 0x00010000);

    /* fix_t largePositive = FIX_POINT(1000000); */
    /* assert((word_t)largePositive == 0x0F424000); */

    fix_t smallNegative = FIX_POINT(-24);
    /* assert((word_t)smallNegative == 0xFFFFE800); */
    assert((word_t)smallNegative == 0xFFE80000);

    /* fix_t largeNegative = FIX_POINT(-5000000); */
    /* assert((word_t)largeNegative == 0xB3B4C000); */

    fix_t terminatingDecimal = FIX_POINT(2.5);
    /* assert((word_t)terminatingDecimal == 0x00000280); */
    assert((word_t)terminatingDecimal == 0x00028000);

    fix_t recurringDecimal = FIX_POINT(21.3);
    /* assert((word_t)recurringDecimal == 0x0000154D); */
    assert((word_t)recurringDecimal == 0x00154CCD);
}
static void test_fixAdd(void) {
    fix_t bothPositive = fixAdd(FIX_POINT(123), FIX_POINT(362));
    assert(bothPositive == FIX_POINT(485));

    fix_t bothNegative = fixAdd(FIX_POINT(-827), FIX_POINT(-21));
    assert(bothNegative == FIX_POINT(-848));

    fix_t negAndPos = fixAdd(FIX_POINT(-389), FIX_POINT(2379));
    assert(negAndPos == FIX_POINT(1990));

    fix_t posAndNeg = fixAdd(FIX_POINT(-2379), FIX_POINT(389));
    assert(posAndNeg == FIX_POINT(-1990));

    fix_t decimals = fixAdd(FIX_POINT(32.25), FIX_POINT(9766.125));
    assert(decimals == FIX_POINT(9798.375));

    fix_t negDecimals = fixAdd(FIX_POINT(-92.0625), FIX_POINT(-18.0078125));
    assert(negDecimals == FIX_POINT(-110.0703125));

    fix_t roundedDecimal = fixAdd(FIX_POINT(1.3), FIX_POINT(1.7));
    assert(roundedDecimal == FIX_POINT(3));

    /* fix_t overflowPos = fixAdd(FIX_POINT(8000000), FIX_POINT(8000000)); */
    fix_t overflowPos = fixAdd(FIX_POINT(30000), FIX_POINT(30000));
    assert(overflowPos == fixOverflow);

    /* fix_t overflowNeg = fixAdd(FIX_POINT(-8000000), FIX_POINT(-8000000)); */
    fix_t overflowNeg = fixAdd(FIX_POINT(-30000), FIX_POINT(-30000));
    assert(overflowNeg == fixOverflow);
}

static void test_fixSubtract(void) {
    fix_t bothPositive = fixSubtract(FIX_POINT(123), FIX_POINT(362));
    assert(bothPositive == FIX_POINT(-239));

    fix_t bothNegative = fixSubtract(FIX_POINT(-827), FIX_POINT(-21));
    assert(bothNegative == FIX_POINT(-806));

    fix_t negAndPos = fixSubtract(FIX_POINT(-389), FIX_POINT(2379));
    assert(negAndPos == FIX_POINT(-2768));

    fix_t posAndNeg = fixSubtract(FIX_POINT(389), FIX_POINT(-2379));
    assert(posAndNeg == FIX_POINT(2768));

    fix_t decimals = fixSubtract(FIX_POINT(32.25), FIX_POINT(0.03125));
    assert(decimals == FIX_POINT(32.21875));

    fix_t negDecimals = fixSubtract(FIX_POINT(-92.0625), FIX_POINT(-18.0078125));
    assert(negDecimals == FIX_POINT(-74.0546875));

    fix_t roundedDecimal = fixSubtract(FIX_POINT(1.3), FIX_POINT(1.7));
    assert(roundedDecimal == FIX_POINT(-0.4));

    /* fix_t overflowPos = fixSubtract(FIX_POINT(8000000), FIX_POINT(-8000000)); */
    fix_t overflowPos = fixSubtract(FIX_POINT(30000), FIX_POINT(-30000));
    assert(overflowPos == fixOverflow);

    /* fix_t overflowNeg = fixSubtract(FIX_POINT(-8000000), FIX_POINT(8000000)); */
    fix_t overflowNeg = fixSubtract(FIX_POINT(-30000), FIX_POINT(30000));
    assert(overflowNeg == fixOverflow);
}

static void test_fixMultiply(void) {
    fix_t bothPositive = fixMultiply(FIX_POINT(36), FIX_POINT(29));
    assert(bothPositive == FIX_POINT(1044));

    fix_t bothNegative = fixMultiply(FIX_POINT(-100), FIX_POINT(-100));
    assert(bothNegative == FIX_POINT(10000));

    fix_t posAndNeg = fixMultiply(FIX_POINT(324), FIX_POINT(-89));
    assert(posAndNeg == FIX_POINT(-28836));

    /* fix_t bigAndSmall = fixMultiply(FIX_POINT(2000000), FIX_POINT(2)); */
    /* assert(bigAndSmall == FIX_POINT(4000000)); */
    fix_t bigAndSmall = fixMultiply(FIX_POINT(15000), FIX_POINT(2));
    assert(bigAndSmall == FIX_POINT(30000));
    
    fix_t decimals = fixMultiply(FIX_POINT(0.25), FIX_POINT(23.5));
    assert(decimals == FIX_POINT(5.875));

    fix_t roundedDecimals = fixMultiply(FIX_POINT(100), FIX_POINT(0.1));
    assert(roundedDecimals == FIX_POINT(10.0006103515625));

    fix_t overflowPos = fixMultiply(FIX_POINT(2000), FIX_POINT(5000));
    assert(overflowPos == fixOverflow);

    fix_t overflowNeg = fixMultiply(FIX_POINT(-2000), FIX_POINT(5000));
    assert(overflowNeg == fixOverflow);
}

#ifdef DEBUG_TOOLS
static void fixPrint(fix_t x) {
    double decimal = (double)x / pow(2, Q_POINT);
    printf("%lf\t\t[", decimal);

    word_t mask = 1 << (WORD_SIZE - 1);

    for (int i = WORD_SIZE; i > 0; i--) {
        printf("%d", (x & mask) > 0);
        if (i == WORD_SIZE || i == Q_POINT + 1)
            printf(" ");
        mask >>= 1;
    }

    printf("]\n");
}
#endif

