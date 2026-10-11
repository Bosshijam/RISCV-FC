#ifndef ATTITUDE_H
#define ATTITUDE_H

#include "fix16.h"

typedef struct {
    fix16_t roll;              /* degrees */
    fix16_t pitch;             /* degrees */
    fix16_t alpha;             /* gyro weight, e.g. 0.998 */
    fix16_t one_minus_alpha;   /* accelerometer weight */
    fix16_t acc_min_sq;        /* accept the accelerometer only if |a|^2 */
    fix16_t acc_max_sq;        /* is inside this window (in g^2) */
    int     acc_used;          /* 1 if the last update used the accelerometer */
    int     initialised;
} attitude_t;

void attitude_init(attitude_t *a, fix16_t alpha);

/* gyro in deg/s, accelerometer in g, dt in seconds */
void attitude_update(attitude_t *a,
                     fix16_t gx, fix16_t gy,
                     fix16_t ax, fix16_t ay, fix16_t az,
                     fix16_t dt);

#endif
