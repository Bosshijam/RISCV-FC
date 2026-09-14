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
    check("1.5 * 2.0", fix16_mul(F16(1.5), F16(2.0)), F16(3.0));
    check("0.5 * 0.5", fix16_mul(F16(0.5), F16(0.5)), F16(0.25));
    check("-2.0 * 3.0", fix16_mul(F16(-2.0), F16(3.0)), F16(-6.0));
    check("100 * 100", fix16_mul(F16(100.0), F16(100.0)), F16(10000.0));
    check("-0.5 * 0.5", fix16_mul(F16(-0.5), F16(0.5)), F16(-0.25));
    check("3.0 / 2.0", fix16_div(F16(3.0), F16(2.0)), F16(1.5));
    check("1.0 / 4.0", fix16_div(F16(1.0), F16(4.0)), F16(0.25));
    check("-6.0 / 3.0", fix16_div(F16(-6.0), F16(3.0)), F16(-2.0));
    check("div by zero +", fix16_div(F16(5.0), 0), FIX16_MAX);
    check("div by zero -", fix16_div(F16(-5.0), 0), FIX16_MIN);
    check("sin(0)",   fix16_sin(F16(0.0)),   F16(0.0));
    check("sin(90)",  fix16_sin(F16(90.0)),  F16(1.0));
    check("sin(180)", fix16_sin(F16(180.0)), F16(0.0));
    check("sin(270)", fix16_sin(F16(270.0)), F16(-1.0));
    check("cos(0)",   fix16_cos(F16(0.0)),   F16(1.0));
    check("cos(90)",  fix16_cos(F16(90.0)),  F16(0.0));
    printf("\n%s\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED");
    return failures;
}
