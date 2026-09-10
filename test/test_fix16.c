#include <stdio.h>
#include "../algorithm/fix16.h"

int failures = 0;

void check(const char *name, fix16_t got, fix16_t want) {
    if (got == want) {
        printf("PASS  %s\n", name);
    } else {
        printf("FAIL  %s: got %d, want %d\n", name, got, want);
        failures++;
    }
}

int main(void) {
    /* 1.5 + 2.0 = 3.5 */
    check("1.5 + 2.0", fix16_add(F16(1.5), F16(2.0)), F16(3.5));

    /* 5.0 - 2.25 = 2.75 */
    check("5.0 - 2.25", fix16_sub(F16(5.0), F16(2.25)), F16(2.75));

    /* negative results work */
    check("1.0 - 3.0", fix16_sub(F16(1.0), F16(3.0)), F16(-2.0));

    /* overflow must SATURATE, not wrap */
    check("max + max", fix16_add(FIX16_MAX, FIX16_MAX), FIX16_MAX);
    check("min - max", fix16_sub(FIX16_MIN, FIX16_MAX), FIX16_MIN);

    printf("\n%s\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED");
    return failures;
}
