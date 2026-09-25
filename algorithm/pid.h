#ifndef PID_H
#define PID_H

#include "fix16.h"

typedef struct {
    fix16_t kp;
    fix16_t ki;
    fix16_t kd;
    fix16_t integral;
    fix16_t integral_limit;
    fix16_t prev_measurement;
} pid_t;

void    pid_init(pid_t *pid, fix16_t kp, fix16_t ki, fix16_t kd, fix16_t i_limit);
fix16_t pid_update(pid_t *pid, fix16_t setpoint, fix16_t measurement, fix16_t dt);

#endif
