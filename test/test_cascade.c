/* Angle loop + rate PID (cascade) against the one-axis plant.
 *
 * Scenario (6 s at 500 Hz):
 *   0.5 s  pilot asks for 20 deg of roll
 *   2.5 s  pilot returns the stick to centre (0 deg)
 *   3.5 s  a constant disturbance torque starts (like an off-centre battery)
 *
 * Plant is made quad-like: inertia 1, motors can give up to 2000 deg/s^2.
 * Sensors: gyro rate and angle estimate, both with noise.               */
#include <stdio.h>
#include <stdint.h>
#include "../sim/plant.h"
#include "../algorithm/pid.h"
#include "../algorithm/angle.h"

static uint32_t rng = 2024;
static fix16_t noise(fix16_t amp) {                    /* -amp .. +amp */
    rng = rng * 1103515245u + 12345u;
    int32_t r = (int32_t)((rng >> 16) & 0x7FFF) - 16384;
    return fix16_mul(amp, (fix16_t)(r << 2));
}

int main(void) {
    const fix16_t dt = F16(0.002);
    const int     N  = 3000;                           /* 6 s */

    plant_t p;
    plant_init(&p, F16(1.0));
    p.max_torque = F16(2000.0);                         /* quad-like authority */

    /* inner loop: rate PID (deg/s in, torque out) */
    pid_params_t rate_params = {
        .kp = F16(20.0),
        .ki = F16(40.0),
        .kd = F16(0.0),
        .i_limit = F16(20.0),
        .out_limit = F16(2000.0),
        .dterm_cutoff_hz = F16(50.0),
    };
    pid_t rate_pid;
    pid_init(&rate_pid, &rate_params, dt);

    /* outer loop: angle P (deg in, deg/s out) */
    angle_ctrl_t angle;
    angle_init(&angle, F16(5.0), F16(200.0));

    printf("time,angle_sp,angle,rate_sp,rate,torque\n");

    for (int i = 0; i < N; i++) {
        double t = i * 0.002;

        fix16_t angle_sp = (t >= 0.5 && t < 2.5) ? F16(20.0) : 0;
        fix16_t disturb  = (t >= 3.5) ? F16(300.0) : 0;

        /* what the sensors would report */
        fix16_t gyro      = fix16_add(p.rate,  noise(F16(0.5)));   /* deg/s */
        fix16_t angle_est = fix16_add(p.angle, noise(F16(0.1)));   /* deg   */

        /* the cascade */
        fix16_t rate_sp = angle_update(&angle, angle_sp, angle_est);
        fix16_t torque  = pid_update(&rate_pid, rate_sp, gyro, dt);

        plant_step(&p, fix16_add(torque, disturb), dt);

        if (i % 2 == 0)
            printf("%.3f,%.2f,%.4f,%.3f,%.3f,%.2f\n", t,
                   angle_sp / 65536.0, p.angle / 65536.0,
                   rate_sp / 65536.0, p.rate / 65536.0, torque / 65536.0);
    }
    return 0;
}
