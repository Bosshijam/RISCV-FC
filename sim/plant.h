#ifndef PLANT_H
#define PLANT_H

#include "../algorithm/fix16.h"

typedef struct {
    fix16_t angle;      /* degrees */
    fix16_t rate;       /* degrees per second */
    fix16_t inertia;    /* resistance to being spun up */
    fix16_t max_torque;  /* saturtion  limit */
} plant_t;

void plant_init(plant_t *p, fix16_t inertia);
void plant_step(plant_t *p, fix16_t torque, fix16_t dt);
/* returns the plant's angle with simulated sensor noise added */
fix16_t plant_measure(plant_t *p, fix16_t noise_amplitude);

#endif
