#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../algorithm/fix16.h"

#define TOL 0.01      /* degrees */

static double diff_deg(double got, double want) {
    double d = got - want;
    if (d >  180.0) d -= 360.0;      /* +180 and -180 are the same angle */
    if (d < -180.0) d += 360.0;
    return fabs(d);
}

int main(void) {
    int fails = 0;
    double worst = 0, worst_at = 0;

    /* every whole degree around the circle */
    for (int deg = -180; deg <= 180; deg++) {
        double r = deg * M_PI / 180.0;
        fix16_t y = (fix16_t)lround(sin(r) * 65536.0);
        fix16_t x = (fix16_t)lround(cos(r) * 65536.0);
        double got  = fix16_atan2(y, x) / 65536.0;
        double want = atan2(y / 65536.0, x / 65536.0) * 180.0 / M_PI;
        double e = diff_deg(got, want);
        if (e > worst) { worst = e; worst_at = deg; }
        if (e > TOL) fails++;
    }
    printf("full circle: worst error %.4f deg at %.0f deg, failures %d\n",
           worst, worst_at, fails);

    /* random vectors of all lengths and directions */
    srand(1);
    double rworst = 0;
    int rfails = 0;
    for (int i = 0; i < 200000; i++) {
        fix16_t y = (fix16_t)(rand() % 13107201) - 6553600;   /* -100 .. +100 */
        fix16_t x = (fix16_t)(rand() % 13107201) - 6553600;
        if (abs(x) < 655 && abs(y) < 655) continue;           /* skip near-zero vectors */
        double got  = fix16_atan2(y, x) / 65536.0;
        double want = atan2(y / 65536.0, x / 65536.0) * 180.0 / M_PI;
        double e = diff_deg(got, want);
        if (e > rworst) rworst = e;
        if (e > TOL) rfails++;
    }
    printf("random: 200000 vectors, worst error %.4f deg, failures %d\n", rworst, rfails);

    fails += rfails;
    printf("%s\n", fails ? "SOME TESTS FAILED" : "ALL PASSED");
    return fails ? 1 : 0;
}
