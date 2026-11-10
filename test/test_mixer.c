#include <stdio.h>
#include "../algorithm/mixer.h"

static int fails = 0;
static const char *name[4] = { "M1 rear-right ", "M2 front-right", "M3 rear-left  ", "M4 front-left " };

static void show(const char *title, const fix16_t o[4], int sq) {
    printf("\n%s%s\n", title, sq ? "   (squeezed)" : "");
    for (int i = 0; i < 4; i++)
        printf("  %s  %.3f  -> %d us\n", name[i], o[i] / 65536.0, (int)mixer_to_us(o[i]));
}

static void check(int ok, const char *what) {
    printf("  %s  %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) fails++;
}

#define NEAR(a, b) ((a) - (b) < 2 && (b) - (a) < 2)

int main(void) {
    mixer_t m;
    fix16_t o[4];
    int sq;
    mixer_init_quadx(&m, F16(0.05));

    sq = mixer_update(&m, F16(0.5), 0, 0, 0, o);
    show("Hover: throttle 0.5, no correction", o, sq);
    check(o[0] == o[1] && o[1] == o[2] && o[2] == o[3], "all motors equal");
    check(NEAR(o[0], F16(0.5)), "all at 0.5");

    sq = mixer_update(&m, F16(0.5), F16(0.1), 0, 0, o);
    show("Roll right: left motors faster", o, sq);
    check(o[2] > o[0] && o[3] > o[1], "left side (M3, M4) > right side (M1, M2)");

    sq = mixer_update(&m, F16(0.5), 0, F16(0.1), 0, o);
    show("Pitch up (nose up): front motors faster", o, sq);
    check(o[1] > o[0] && o[3] > o[2], "front (M2, M4) > rear (M1, M3)");

    sq = mixer_update(&m, F16(0.5), 0, 0, F16(0.1), o);
    show("Yaw right: CCW motors faster", o, sq);
    check(o[1] > o[0] && o[2] > o[3], "CCW (M2, M3) > CW (M1, M4)");

    sq = mixer_update(&m, F16(0.5), F16(0.1), F16(0.1), F16(0.1), o);
    {
        fix16_t sum = o[0] + o[1] + o[2] + o[3];
        check(NEAR(sum, F16(2.0)), "corrections don't change total thrust (sum = 4 x 0.5)");
    }

    sq = mixer_update(&m, F16(0.95), F16(0.2), 0, 0, o);
    show("Full throttle + roll: would exceed 1, throttle shifted down", o, sq);
    check(sq, "squeeze reported");
    check(o[2] <= FIX16_ONE && o[3] <= FIX16_ONE, "no motor above 1");
    check(NEAR(o[2] - o[0], F16(0.4)), "roll difference kept at 0.4");

    sq = mixer_update(&m, F16(0.0), F16(0.2), 0, 0, o);
    show("Zero throttle + roll: lifted to idle so correction still works", o, sq);
    check(o[0] >= F16(0.05) - 1, "no motor below idle");
    check(NEAR(o[2] - o[0], F16(0.4)), "roll difference kept at 0.4");

    sq = mixer_update(&m, F16(0.5), F16(0.8), F16(0.8), 0, o);
    show("Huge correction: wider than the usable range, scaled down", o, sq);
    {
        int ok = 1;
        for (int i = 0; i < 4; i++) if (o[i] < F16(0.05) - 1 || o[i] > FIX16_ONE) ok = 0;
        check(ok, "every motor between idle and 1");
        check(o[3] > o[1] && o[3] > o[2] && o[0] < o[1], "still corrects in the right direction");
    }

    check(mixer_to_us(0) == 1000 && mixer_to_us(FIX16_ONE) == 2000 && mixer_to_us(F16(0.5)) == 1500,
          "0 -> 1000 us, 0.5 -> 1500 us, 1 -> 2000 us");

    printf("\n%s\n", fails ? "SOME TESTS FAILED" : "ALL PASSED");
    return fails ? 1 : 0;
}
