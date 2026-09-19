#include "pid.h"

void pid_init(pid_t *pid, fix16_t kp) {
    pid->kp = kp;
    pid->prev_measurement = 0;
}

fix16_t pid_update(pid_t *pid, fix16_t setpoint, fix16_t measurement, fix16_t dt) {
    (void)dt;   /* unused until D and I are added */

    fix16_t error = fix16_sub(setpoint, measurement);
    fix16_t output = fix16_mul(pid->kp, error);

    pid->prev_measurement = measurement;
    return output;
}
