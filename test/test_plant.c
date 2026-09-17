#include <stdio.h>
#include "../sim/plant.h"

int main(void) {
    plant_t p;
    plant_init(&p, F16(1.0));        /* inertia of 1 */

    fix16_t dt = F16(0.002);          /* 2 ms, a 500 Hz loop */
    fix16_t torque = F16(10.0);       /* constant push */

    printf("time,angle,rate\n");

    for (int i = 0; i < 500; i++) {   /* one second */
        plant_step(&p, torque, dt);

        double t     = (i + 1) * 0.002;
        double angle = (double)p.angle / 65536.0;
        double rate  = (double)p.rate  / 65536.0;

        printf("%.3f,%.4f,%.4f\n", t, angle, rate);
    }

    return 0;
}
