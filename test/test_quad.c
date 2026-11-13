/* Full control chain on a simulated three-axis quad.
 *
 *   IMU (gyro + accel, noisy, gyro biased)
 *     -> gyro calibration (first 1 s, motors off)
 *     -> complementary filter  -> roll, pitch estimate
 *     -> angle loops (roll, pitch)  -> rate setpoints
 *     -> rate PIDs (roll, pitch, yaw) -> corrections
 *     -> mixer -> 4 motor commands -> simulated quad
 *
 * Everything in algorithm/ is the real flight code, in fixed point.
 * Only sim/quad.c (the "drone") and the fake sensors here use double. */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "../sim/quad.h"
#include "../algorithm/fix16.h"
#include "../algorithm/pid.h"
#include "../algorithm/angle.h"
#include "../algorithm/mixer.h"
#include "../algorithm/attitude.h"
#include "../algorithm/gyro_cal.h"

static uint32_t rng = 99;
static double noise(double amp) {                       /* uniform -amp..+amp */
    rng = rng * 1103515245u + 12345u;
    return amp * ((double)((rng >> 16) & 0x7FFF) / 16383.5 - 1.0);
}
static fix16_t F(double x)  { return (fix16_t)lround(x * 65536.0); }
static double  D(fix16_t x) { return x / 65536.0; }

/* what the pilot asks for at time t */
static void pilot(double t, double *roll_sp, double *pitch_sp, double *yaw_rate_sp) {
    *roll_sp = *pitch_sp = *yaw_rate_sp = 0;
    if (t >= 2.0 && t < 4.0)  *roll_sp  = 20;     /* roll right 20 deg */
    if (t >= 5.0 && t < 7.0)  *pitch_sp = 15;     /* nose up 15 deg    */
    if (t >= 8.0 && t < 9.0)  *yaw_rate_sp = 90;  /* turn right 90 deg/s */
    if (t >= 11.0 && t < 12.5) {                  /* all at once       */
        *roll_sp = -20; *pitch_sp = -15; *yaw_rate_sp = 60;
    }
}

int main(void) {
    const double  dt_s = 0.002;                   /* 500 Hz loop */
    const fix16_t dt   = F(dt_s);
    const int     N    = 7000;                    /* 14 s */
    const double  hover_throttle = 0.31;          /* ~1 kg quad, 4 x 8 N motors */
    const double  gbias[3] = { 0.8, -0.5, 0.3 };  /* deg/s, unknown to the controller */

    quad_t q;
    quad_init(&q);

    /* ---- flight code, all fixed point ---- */
    gyro_cal_t cal[3];
    for (int i = 0; i < 3; i++) gyro_cal_init(&cal[i], 500, F(5.0));   /* 1 s */

    attitude_t att;
    attitude_init(&att, F(0.998), F(5.0), dt);   /* gyro weight, accel filter 5 Hz */

    angle_ctrl_t ang_roll, ang_pitch;
    angle_init(&ang_roll,  F(6.0), F(250.0));
    angle_init(&ang_pitch, F(6.0), F(250.0));

    pid_params_t rp = {                          /* roll and pitch rate */
        .kp = F(0.0030), .ki = F(0.0100), .kd = F(0.00003),
        .i_limit = F(20.0), .out_limit = F(1.0), .dterm_cutoff_hz = F(40.0) };
    pid_params_t yp = {                          /* yaw rate */
        .kp = F(0.0150), .ki = F(0.0300), .kd = 0,
        .i_limit = F(10.0), .out_limit = F(0.4), .dterm_cutoff_hz = F(40.0) };
    pid_t pid_roll, pid_pitch, pid_yaw;
    pid_init(&pid_roll,  &rp, dt);
    pid_init(&pid_pitch, &rp, dt);
    pid_init(&pid_yaw,   &yp, dt);

    mixer_t mix;
    mixer_init_quadx(&mix, F(0.05));

    int squeezed = 0, armed = 0;
    double err_max = 0;

    printf("time,roll_sp,roll,roll_est,pitch_sp,pitch,pitch_est,yawrate_sp,yaw_rate,m1,m2,m3,m4\n");

    for (int i = 0; i < N; i++) {
        double t = i * dt_s;

        /* ---- fake IMU ---- */
        double gx = q.roll_rate  + gbias[0] + noise(1.0);
        double gy = q.pitch_rate + gbias[1] + noise(1.0);
        double gz = q.yaw_rate   + gbias[2] + noise(1.0);
        double ax, ay, az;
        quad_gravity(&q, &ax, &ay, &az);
        ax += noise(0.03); ay += noise(0.03); az += noise(0.03);

        /* ---- 1. calibrate while sitting still, motors off ---- */
        if (!armed) {
            gyro_cal_feed(&cal[0], F(gx));
            gyro_cal_feed(&cal[1], F(gy));
            gyro_cal_feed(&cal[2], F(gz));
            if (cal[0].done && cal[1].done && cal[2].done) {
                if (cal[0].failed || cal[1].failed || cal[2].failed) {
                    fprintf(stderr, "calibration failed - refusing to arm\n");
                    return 1;
                }
                armed = 1;
                fprintf(stderr, "armed at %.3f s, bias found: %.3f %.3f %.3f (true %.1f %.1f %.1f)\n",
                        t, D(cal[0].bias), D(cal[1].bias), D(cal[2].bias),
                        gbias[0], gbias[1], gbias[2]);
            }
            attitude_update(&att, 0, 0, F(ax), F(ay), F(az), dt);  /* settle the estimate */
            continue;
        }

        fix16_t rr = gyro_cal_apply(&cal[0], F(gx));
        fix16_t pr = gyro_cal_apply(&cal[1], F(gy));
        fix16_t yr = gyro_cal_apply(&cal[2], F(gz));

        /* ---- 2. estimate attitude ---- */
        attitude_update(&att, rr, pr, F(ax), F(ay), F(az), dt);

        /* ---- 3. pilot ---- */
        double rsp, psp, ysp;
        pilot(t, &rsp, &psp, &ysp);

        /* ---- 4. angle loops -> rate setpoints ---- */
        fix16_t roll_rate_sp  = angle_update(&ang_roll,  F(rsp), att.roll);
        fix16_t pitch_rate_sp = angle_update(&ang_pitch, F(psp), att.pitch);

        /* ---- 5. rate PIDs -> corrections ---- */
        fix16_t c_roll  = pid_update(&pid_roll,  roll_rate_sp,  rr, dt);
        fix16_t c_pitch = pid_update(&pid_pitch, pitch_rate_sp, pr, dt);
        fix16_t c_yaw   = pid_update(&pid_yaw,   F(ysp),        yr, dt);

        /* ---- 6. mixer -> motors ---- */
        fix16_t out[4];
        squeezed += mixer_update(&mix, F(hover_throttle), c_roll, c_pitch, c_yaw, out);
        double cmd[4] = { D(out[0]), D(out[1]), D(out[2]), D(out[3]) };

        /* a gust: sharp roll push at 10 s */
        double gust = (t >= 10.0 && t < 10.2) ? 0.3 : 0.0;
        quad_step(&q, cmd, gust, 0, 0, dt_s);

        if (t > 1.5) {
            double e = fabs(D(att.roll) - q.roll);
            if (fabs(D(att.pitch) - q.pitch) > e) e = fabs(D(att.pitch) - q.pitch);
            if (e > err_max) err_max = e;
        }
        if (i % 5 == 0)
            printf("%.3f,%.1f,%.3f,%.3f,%.1f,%.3f,%.3f,%.1f,%.2f,%.3f,%.3f,%.3f,%.3f\n",
                   t, rsp, q.roll, D(att.roll), psp, q.pitch, D(att.pitch),
                   ysp, q.yaw_rate, cmd[0], cmd[1], cmd[2], cmd[3]);
    }

    fprintf(stderr, "final: roll %.2f  pitch %.2f  yaw rate %.2f\n", q.roll, q.pitch, q.yaw_rate);
    fprintf(stderr, "worst estimate error after 1.5 s: %.2f deg\n", err_max);
    fprintf(stderr, "mixer had to squeeze on %d of %d armed samples\n", squeezed, N - 500);
    return 0;
}
