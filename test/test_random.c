#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../algorithm/fix16.h"

#define TRIALS 100000
#define TOLERANCE 3      /* units in the last place */

int failures = 0;

/* convert fixed point back to double so we can compare */
double to_double(fix16_t x) {
    return (double)x / 65536.0;
}

/* a random value in a sensible flight-controller range */
fix16_t random_fix(double limit) {
    double r = ((double)rand() / RAND_MAX) * 2.0 - 1.0;   /* -1..+1 */
    return (fix16_t)(r * limit * 65536.0);
}

void report(const char *op, fix16_t a, fix16_t b, fix16_t got, double want) {
    if (failures < 10) {
        printf("FAIL %s: a=%f b=%f got=%f want=%f\n",
               op, to_double(a), to_double(b), to_double(got), want);
    }
    failures++;
}

int main(void) {
    srand(12345);          /* fixed seed so failures are reproducible */

    printf("Running %d random trials per operation...\n\n", TRIALS);

    /* --- addition --- */
    for (int i = 0; i < TRIALS; i++) {
        fix16_t a = random_fix(1000.0);
        fix16_t b = random_fix(1000.0);
        fix16_t got = fix16_add(a, b);
        double want = to_double(a) + to_double(b);
        if (fabs(to_double(got) - want) > TOLERANCE / 65536.0)
            report("add", a, b, got, want);
    }

    /* --- multiplication --- */
    for (int i = 0; i < TRIALS; i++) {
        fix16_t a = random_fix(100.0);
        fix16_t b = random_fix(100.0);
        fix16_t got = fix16_mul(a, b);
        double want = to_double(a) * to_double(b);
        if (fabs(to_double(got) - want) > TOLERANCE / 65536.0)
            report("mul", a, b, got, want);
    }

    /* --- division --- */
    for (int i = 0; i < TRIALS; i++) {
        fix16_t a = random_fix(100.0);
        fix16_t b = random_fix(100.0);
        if (b == 0) continue;
        if (fabs(to_double(b)) < 0.01) continue;   /* tiny divisors blow up */
        fix16_t got = fix16_div(a, b);
        double want = to_double(a) / to_double(b);
        if (fabs(want) > 30000.0) continue;        /* would saturate */
        if (fabs(to_double(got) - want) > TOLERANCE / 65536.0)
            report("div", a, b, got, want);
    }

    /* --- sine, wider tolerance: interpolation error --- */
    for (int i = 0; i < TRIALS; i++) {
        fix16_t a = random_fix(360.0);
        fix16_t got = fix16_sin(a);
        double want = sin(to_double(a) * M_PI / 180.0);
        if (fabs(to_double(got) - want) > 0.001)
            report("sin", a, 0, got, want);
    }

    printf("\n%d failures out of %d trials\n", failures, TRIALS * 4);
    printf("%s\n", failures ? "SOME TESTS FAILED" : "ALL RANDOM TESTS PASSED");
    return failures ? 1 : 0;
}
