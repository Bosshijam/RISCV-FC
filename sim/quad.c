#include "quad.h"
#include <math.h>

#define DEG (180.0 / M_PI)

void quad_init(quad_t *q) {
    q->arm        = 0.125;     /* 250-class frame        */
    q->ixx        = 0.010;
    q->iyy        = 0.010;
    q->izz        = 0.020;
    q->max_thrust = 8.0;       /* ~3.2 kg total thrust   */
    q->yaw_coeff  = 0.016;
    q->motor_tau  = 0.030;     /* 30 ms spin-up          */
    for (int i = 0; i < 4; i++) q->thrust[i] = 0;
    q->roll = q->pitch = q->yaw = 0;
    q->roll_rate = q->pitch_rate = q->yaw_rate = 0;
}

void quad_step(quad_t *q, const double cmd[4],
               double dist_roll, double dist_pitch, double dist_yaw, double dt) {
    /* motors lag behind their commands */
    double k = dt / (q->motor_tau + dt);
    for (int i = 0; i < 4; i++)
        q->thrust[i] += k * (cmd[i] * q->max_thrust - q->thrust[i]);

    double t1 = q->thrust[0], t2 = q->thrust[1], t3 = q->thrust[2], t4 = q->thrust[3];
    double lever = q->arm / sqrt(2.0);          /* X frame: arms at 45 deg */

    /* roll right  <- left (M3, M4) pushing harder than right (M1, M2)   */
    double tq_roll  = ((t3 + t4) - (t1 + t2)) * lever + dist_roll;
    /* nose up     <- front (M2, M4) pushing harder than rear (M1, M3)   */
    double tq_pitch = ((t2 + t4) - (t1 + t3)) * lever + dist_pitch;
    /* nose right  <- CCW props (M2, M3) dragging harder than CW (M1, M4) */
    double tq_yaw   = ((t2 + t3) - (t1 + t4)) * q->yaw_coeff + dist_yaw;

    q->roll_rate  += (tq_roll  / q->ixx) * DEG * dt;
    q->pitch_rate += (tq_pitch / q->iyy) * DEG * dt;
    q->yaw_rate   += (tq_yaw   / q->izz) * DEG * dt;

    q->roll  += q->roll_rate  * dt;
    q->pitch += q->pitch_rate * dt;
    q->yaw   += q->yaw_rate   * dt;
}

void quad_gravity(const quad_t *q, double *ax, double *ay, double *az) {
    double r = q->roll / DEG, p = q->pitch / DEG;
    *ax = -sin(p);
    *ay =  sin(r) * cos(p);
    *az =  cos(r) * cos(p);
}
