#include "plant.h"


/* simple pseudo-random generator - deterministic, so runs are repeatable */
static uint32_t rng_state = 12345;

static int32_t rand_signed(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return (int32_t)((rng_state >> 16) & 0x7FFF) - 16384;   /* -16384..+16383 */
}
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

fix16_t plant_measure(plant_t *p, fix16_t noise_amplitude) {
    if (noise_amplitude == 0) return p->angle;

    /* rand_signed() gives -16384..+16383, i.e. roughly +-2^14.
     * Shift right by 14 to scale it into roughly -1..+1 in Q16.16,
     * then multiply by the amplitude. No division, no overflow. */
    fix16_t unit = (fix16_t)(rand_signed() << 2);   /* -65536..+65532 = -1.0..+1.0 */
    fix16_t noise = fix16_mul(noise_amplitude, unit);

    return fix16_add(p->angle, noise);
}
