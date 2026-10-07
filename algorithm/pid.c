#include "pid.h"

void pid_init(pid_t *pid, const pid_params_t *params, fix16_t dt) {
    pid->params           = *params;
    pid->integral         = 0;
    pid->prev_measurement = 0;
    pt1_init(&pid->dterm_filter, params->dterm_cutoff_hz, dt);
}

/* change gains or cutoff while running: keeps integral and filter state */
void pid_set_params(pid_t *pid, const pid_params_t *params, fix16_t dt) {
    pid->params = *params;
    pt1_set_cutoff(&pid->dterm_filter, params->dterm_cutoff_hz, dt);
}

fix16_t pid_update(pid_t *pid, fix16_t setpoint, fix16_t measurement, fix16_t dt) {
    const pid_params_t *p = &pid->params;
    fix16_t error = fix16_sub(setpoint, measurement);

    /* P */
    fix16_t p_term = fix16_mul(p->kp, error);

    /* I: accumulate error, clamped against windup */
    pid->integral = fix16_add(pid->integral, fix16_mul(error, dt));
    if (pid->integral >  p->i_limit) pid->integral =  p->i_limit;
    if (pid->integral < fix16_sub(0, p->i_limit))
        pid->integral = fix16_sub(0, p->i_limit);
    fix16_t i_term = fix16_mul(p->ki, pid->integral);

    /* D on measurement, negated, then low-passed */
    fix16_t change     = fix16_sub(measurement, pid->prev_measurement);
    fix16_t derivative = fix16_div(change, dt);
    derivative = pt1_apply(&pid->dterm_filter, derivative);
    fix16_t d_term = fix16_mul(p->kd, fix16_sub(0, derivative));

    pid->prev_measurement = measurement;

    return fix16_add(fix16_add(p_term, i_term), d_term);
}
