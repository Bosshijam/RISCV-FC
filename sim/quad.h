#ifndef QUAD_H
#define QUAD_H

/* Simulated quad-X, three axes. LAPTOP ONLY - uses double, never runs on the board.
 *
 * Conventions (must match the mixer and attitude filter):
 *   roll  + = right side down     pitch + = nose up     yaw + = nose right
 *
 * Motor layout is the same as algorithm/mixer.h:
 *        front
 *     M4(CW)   M2(CCW)
 *     M3(CCW)  M1(CW)
 *        rear
 *
 * Physics kept deliberately simple:
 *   - each motor: thrust = max_thrust * command, reached through a first-order lag
 *   - roll/pitch torque from thrust differences across the arms
 *   - yaw torque from propeller drag (a CCW prop twists the frame clockwise)
 *   - small angles: each axis treated on its own (fine below about 30 deg)  */

typedef struct {
    /* airframe */
    double arm;            /* m, centre to motor                       */
    double ixx, iyy, izz;  /* kg m^2                                   */
    double max_thrust;     /* N per motor at command 1.0               */
    double yaw_coeff;      /* N m of yaw torque per N of thrust        */
    double motor_tau;      /* s, motor spin-up time constant           */

    /* state */
    double thrust[4];      /* N, what each motor is actually producing */
    double roll, pitch, yaw;            /* deg   */
    double roll_rate, pitch_rate, yaw_rate; /* deg/s */
} quad_t;

void quad_init(quad_t *q);

/* cmd[i] in 0..1 (mixer output), dist_* = external torques in N m */
void quad_step(quad_t *q, const double cmd[4],
               double dist_roll, double dist_pitch, double dist_yaw, double dt);

/* what an ideal accelerometer would read at rest, in g */
void quad_gravity(const quad_t *q, double *ax, double *ay, double *az);

#endif
