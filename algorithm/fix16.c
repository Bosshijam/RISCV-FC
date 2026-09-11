#include "fix16.h"

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
