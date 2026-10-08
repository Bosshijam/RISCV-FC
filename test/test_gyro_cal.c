#include <stdio.h>
#include <stdint.h>
#include "../algorithm/gyro_cal.h"

static uint32_t rng = 777;
static fix16_t noise(fix16_t amp) {            /* -amp .. +amp */
    rng = rng * 1103515245u + 12345u;
    int32_t r = (int32_t)((rng >> 16) & 0x7FFF) - 16384;
    return fix16_mul(amp, (fix16_t)(r << 2));
}

int main(void) {
    fix16_t true_bias = F16(1.5);              /* deg/s the gyro reads while still */
    fix16_t dt = F16(0.002);

    gyro_cal_t cal;
    gyro_cal_init(&cal, 1000, F16(2.0));       /* 1000 samples = 2 s */

    for (int i = 0; i < 1000; i++) {
        fix16_t reading = fix16_add(true_bias, noise(F16(0.2)));
        gyro_cal_feed(&cal, reading);
    }

    printf("calibration: done=%d failed=%d bias=%.4f (true %.4f)\n",
           cal.done, cal.failed,
           (double)cal.bias / 65536.0, (double)true_bias / 65536.0);

    /* integrate 60 seconds of a stationary gyro, with and without correction */
    fix16_t raw_angle = 0, corrected_angle = 0;
    for (int i = 0; i < 30000; i++) {
        fix16_t reading = fix16_add(true_bias, noise(F16(0.2)));
        raw_angle       = fix16_add(raw_angle, fix16_mul(reading, dt));
        corrected_angle = fix16_add(corrected_angle,
                              fix16_mul(gyro_cal_apply(&cal, reading), dt));
    }

    printf("after 60 s stationary:\n");
    printf("  uncorrected angle: %.3f deg\n", (double)raw_angle / 65536.0);
    printf("  corrected angle:   %.3f deg\n", (double)corrected_angle / 65536.0);

    /* now simulate someone moving the aircraft during calibration */
    gyro_cal_t bad;
    gyro_cal_init(&bad, 1000, F16(2.0));
    for (int i = 0; i < 1000; i++) {
        fix16_t reading = fix16_add(true_bias, noise(F16(0.2)));
        if (i > 400 && i < 600) reading = fix16_add(reading, F16(30.0));   /* picked up */
        gyro_cal_feed(&bad, reading);
    }
    printf("moved during calibration: done=%d failed=%d (want failed=1)\n",
           bad.done, bad.failed);

    return 0;
}
