#ifndef PLANT_H
#define PLANT_H

#include "../algorithm/fix16.h"

typedef struct {
    fix16_t angle;      /* degrees */
    fix16_t rate;       /* degrees per second */
    fix16_t inertia;    /* resistance to being spun up */
} plant_t;

void plant_init(plant_t *p, fix16_t inertia);
void plant_step(plant_t *p, fix16_t torque, fix16_t dt);

#endif
