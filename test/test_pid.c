#include <stdio.h>
#include "../sim/plant.h"
#include "../algorithm/pid.h"
#include <stdint.h>

int main(void) {
    plant_t p;
    pid_t   pid;

    fix16_t dt       = F16(0.002);
    fix16_t setpoint = F16(10.0);
    
    plant_init(&p, F16(1.0));
        pid_params_t params = {
        .kp = F16(1.0),
        .ki = F16(0.1),
        .kd = F16(2.0),
        .i_limit = F16(50.0),
        .dterm_cutoff_hz = F16(20.0),
    };
    pid_init(&pid, &params, dt);

    printf("time,angle,rate\n");

    for (int i = 0; i < 200000; i++) {
        fix16_t measured = plant_measure(&p, F16(0.1));   /* 0.1 deg of noise */
        if (i < 5) {
           fprintf(stderr, "i=%d true=%.4f measured=%.4f\n", i,
           (double)p.angle / 65536.0,
           (double)measured / 65536.0);
        }
        fix16_t torque = pid_update(&pid, setpoint, measured, dt);
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
 
