#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../algorithm/fix16.h"

#define TOL 2.0e-5     /* a little over 1 LSB (1/65536 = 1.5e-5) */

static int fails = 0;

static void check(double x) {
    fix16_t fx  = (fix16_t)(x * 65536.0 + (x >= 0 ? 0.5 : -0.5));
    double got  = fix16_sqrt(fx) / 65536.0;
    double want = (fx > 0) ? sqrt(fx / 65536.0) : 0.0;
    double err  = fabs(got - want);
    int ok = err < TOL;
    if (!ok) fails++;
    printf("%s  sqrt(%11.5f) = %10.6f   want %10.6f   err %.1e\n",
           ok ? "PASS" : "FAIL", x, got, want, err);
}

int main(void) {
    /* known values */
    check(0.0);
    check(1.0);
    check(4.0);
    check(2.0);           /* 1.414214 */
    check(0.25);          /* 0.5 */
    check(100.0);
    check(0.0001);        /* very small */
    check(32767.0);       /* near the top of the range: 181.0 */
    check(-1.0);          /* negative: expect 0 */

    /* random trials across the whole positive range */
    srand(1);
    double worst = 0;
    int rfails = 0;
    for (int i = 0; i < 200000; i++) {
        fix16_t fx  = (fix16_t)(((uint32_t)rand() << 1 ^ (uint32_t)rand()) & 0x7FFFFFFF);
        double got  = fix16_sqrt(fx) / 65536.0;
        double want = sqrt(fx / 65536.0);
        double err  = fabs(got - want);
        if (err > worst) worst = err;
        if (err >= TOL) rfails++;
    }
    printf("\nrandom: 200000 trials, worst error %.2e, failures %d\n", worst, rfails);

    fails += rfails;
    printf("%s\n", fails ? "SOME TESTS FAILED" : "ALL PASSED");
    return fails ? 1 : 0;
}
