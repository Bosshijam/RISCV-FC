#include "attitude.h"

void attitude_init(attitude_t *a, fix16_t alpha) {
    a->roll  = 0;
    a->pitch = 0;
    a->alpha = alpha;
    a->one_minus_alpha = fix16_sub(F16(1.0), alpha);
    a->acc_min_sq = F16(0.81);     /* 0.9 g, squared */
    a->acc_max_sq = F16(1.21);     /* 1.1 g, squared */
    a->acc_used = 0;
    a->initialised = 0;
}

void attitude_update(attitude_t *a,
                     fix16_t gx, fix16_t gy,
                     fix16_t ax, fix16_t ay, fix16_t az,
                     fix16_t dt) {
    /* gyro prediction: where we were, plus how far we rotated */
    fix16_t roll_pred  = fix16_add(a->roll,  fix16_mul(gx, dt));
    fix16_t pitch_pred = fix16_add(a->pitch, fix16_mul(gy, dt));

    /* gravity alone is exactly 1 g. If the reading is far from 1 g,
     * the drone is accelerating and the accelerometer can't be trusted. */
    fix16_t mag_sq = fix16_add(fix16_add(fix16_mul(ax, ax), fix16_mul(ay, ay)),
                               fix16_mul(az, az));
    a->acc_used = (mag_sq >= a->acc_min_sq) && (mag_sq <= a->acc_max_sq);

    if (!a->acc_used) {
        a->roll  = roll_pred;
        a->pitch = pitch_pred;
        return;
    }

    /* tilt from the direction of gravity */
    fix16_t roll_acc  = fix16_atan2(ay, az);
    fix16_t yz        = fix16_sqrt(fix16_add(fix16_mul(ay, ay), fix16_mul(az, az)));
    fix16_t pitch_acc = fix16_atan2(fix16_sub(0, ax), yz);

    /* first good reading: start from the accelerometer instead of 0 */
    if (!a->initialised) {
        a->roll  = roll_acc;
        a->pitch = pitch_acc;
        a->initialised = 1;
        return;
    }

    /* angle = pred + (1 - alpha) * (acc - pred)
     * same as alpha*pred + (1-alpha)*acc, but one multiply and less rounding */
    a->roll  = fix16_add(roll_pred,
                 fix16_mul(a->one_minus_alpha, fix16_sub(roll_acc, roll_pred)));
    a->pitch = fix16_add(pitch_pred,
                 fix16_mul(a->one_minus_alpha, fix16_sub(pitch_acc, pitch_pred)));
}
