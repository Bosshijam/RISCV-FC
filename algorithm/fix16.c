#include "fix16.h"
#include "sin_table.h"
fix16_t fix16_add(fix16_t a, fix16_t b) {
    int64_t sum = (int64_t)a + (int64_t)b;
    if (sum > FIX16_MAX) return FIX16_MAX;
    if (sum < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)sum;
}

fix16_t fix16_sub(fix16_t a, fix16_t b) {
    int64_t diff = (int64_t)a - (int64_t)b;
    if (diff > FIX16_MAX) return FIX16_MAX;
    if (diff < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)diff;
}
fix16_t fix16_mul(fix16_t a, fix16_t b) {
    int64_t product = (int64_t)a * (int64_t)b;

    /* round to nearest instead of truncating toward zero */
    product += 32768;

    /* remove the second scale factor */
    product >>= 16;

    if (product > FIX16_MAX) return FIX16_MAX;
    if (product < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)product;
}
fix16_t fix16_div(fix16_t a, fix16_t b) {
    /* never divide by zero on a flying aircraft */
    if (b == 0) {
        return (a >= 0) ? FIX16_MAX : FIX16_MIN;
    }

    /* restore the scale factor that division cancels out */
    int64_t result = ((int64_t)a << 16) / b;

    if (result > FIX16_MAX) return FIX16_MAX;
    if (result < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)result;
}
/* sine of an angle in degrees, Q16.16 in and out */
fix16_t fix16_sin(fix16_t degrees) {
    int negate = 0;

    /* wrap into 0..360 */
    while (degrees < 0)            degrees = fix16_add(degrees, F16(360.0));
    while (degrees >= F16(360.0))  degrees = fix16_sub(degrees, F16(360.0));

    /* fold the bottom half onto the top, remembering the sign */
    if (degrees >= F16(180.0)) {
        degrees = fix16_sub(degrees, F16(180.0));
        negate = 1;
    }

    /* mirror the second quadrant onto the first */
    if (degrees > F16(90.0)) {
        degrees = fix16_sub(F16(180.0), degrees);
    }

    /* which table entry? 256 steps span 90 degrees */
    int64_t position = (((int64_t)degrees * SIN_TABLE_ENTRIES) << 16) / F16(90.0);
    int index = (int)(position >> 16);
        if (index < 0) index = 0;
    if (index >= SIN_TABLE_ENTRIES) {
        /* exactly 90 degrees - return the last entry directly */
        return negate ? fix16_sub(0, sin_table[SIN_TABLE_ENTRIES])
                      : sin_table[SIN_TABLE_ENTRIES];    }

    /* how far between this entry and the next, as a Q16.16 fraction */
    fix16_t fraction = (fix16_t)(position & 0xFFFF);

    fix16_t low  = sin_table[index];
    fix16_t high = sin_table[index + 1];
    fix16_t gap  = fix16_sub(high, low);

    fix16_t result = fix16_add(low, fix16_mul(gap, fraction));
    return negate ? fix16_sub(0, result) : result;
}

/* cos(x) = sin(x + 90) */
fix16_t fix16_cos(fix16_t degrees) {
    return fix16_sin(fix16_add(degrees, F16(90.0)));
}
/* arctangent of y/x, returns degrees in -180..+180 */
fix16_t fix16_atan2(fix16_t y, fix16_t x) {
    fix16_t abs_y = (y < 0) ? fix16_sub(0, y) : y;
    fix16_t angle;

    /* both zero: no direction, return 0 rather than garbage */
    if (x == 0 && y == 0) return 0;

    if (x >= 0) {
        /* ratio stays within -1..+1 */
        fix16_t denom = fix16_add(abs_y, x);
        if (denom == 0) return 0;
        fix16_t r = fix16_div(fix16_sub(x, abs_y), denom);
        angle = fix16_sub(F16(45.0), fix16_mul(F16(45.0), r));
    } else {
        fix16_t denom = fix16_add(abs_y, fix16_sub(0, x));
        if (denom == 0) return 0;
        fix16_t r = fix16_div(fix16_add(x, abs_y), denom);
        angle = fix16_sub(F16(135.0), fix16_mul(F16(45.0), r));
    }

    /* mirror into the lower half-plane if y was negative */
    return (y < 0) ? fix16_sub(0, angle) : angle;
}
