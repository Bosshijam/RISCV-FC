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
