#include "pid.h"

void pid_init(pid_t *pid, fix16_t kp, fix16_t ki, fix16_t kd,fix16_t i_limit, fix16_t dterm_cutoff_hz, fix16_t dt) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0;
    pid->integral_limit = i_limit;
    pid->prev_measurement = 0;
    pt1_init(&pid->dterm_filter, dterm_cutoff_hz, dt);
}

fix16_t pid_update(pid_t *pid, fix16_t setpoint, fix16_t measurement, fix16_t dt) {
    fix16_t error = fix16_sub(setpoint, measurement);

    /* P */
    fix16_t p_term = fix16_mul(pid->kp, error);

    /* I: accumulate error over time, clamped */
    pid->integral = fix16_add(pid->integral, fix16_mul(error, dt));
    if (pid->integral >  pid->integral_limit) pid->integral =  pid->integral_limit;
    if (pid->integral < fix16_sub(0, pid->integral_limit))
        pid->integral = fix16_sub(0, pid->integral_limit);
    fix16_t i_term = fix16_mul(pid->ki, pid->integral);

    /* D on measurement, negated */
    fix16_t change = fix16_sub(measurement, pid->prev_measurement);
    fix16_t derivative = fix16_div(change, dt);
    derivative = pt1_apply(&pid->dterm_filter, derivative);      /* <-- new */
    fix16_t d_term = fix16_mul(pid->kd, fix16_sub(0, derivative));

    pid->prev_measurement = measurement;

    return fix16_add(fix16_add(p_term, i_term), d_term);
}
