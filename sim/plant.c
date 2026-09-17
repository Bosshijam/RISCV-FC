#include "plant.h"

void plant_init(plant_t *p, fix16_t inertia) {
    p->angle   = 0;
    p->rate    = 0;
    p->inertia = inertia;
}

void plant_step(plant_t *p, fix16_t torque, fix16_t dt) {
    /* Newton: acceleration = torque / inertia */
    fix16_t accel = fix16_div(torque, p->inertia);

    /* integrate acceleration to get rate */
    p->rate = fix16_add(p->rate, fix16_mul(accel, dt));

    /* integrate rate to get angle */
    p->angle = fix16_add(p->angle, fix16_mul(p->rate, dt));
}
