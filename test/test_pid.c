#include <stdio.h>
#include "../sim/plant.h"
#include "../algorithm/pid.h"

int main(void) {
    plant_t p;
    pid_t   pid;

    plant_init(&p, F16(1.0));
    pid_init(&pid, F16(2.0), F16(0.5), F16(1.0), F16(50.0));

    fix16_t dt       = F16(0.002);
    fix16_t setpoint = F16(10.0);

    printf("time,angle,rate\n");

    for (int i = 0; i < 10000; i++) {
        fix16_t torque = pid_update(&pid, setpoint, p.angle, dt);
        plant_step(&p, torque, dt);

        printf("%.3f,%.4f,%.4f\n",
               (i + 1) * 0.002,
               (double)p.angle / 65536.0,
               (double)p.rate  / 65536.0);
    }
    fprintf(stderr, "final integral raw = %d  (%.4f)\n",
    pid.integral, (double)pid.integral / 65536.0);

    return 0;
}
