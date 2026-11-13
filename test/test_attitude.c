#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "../algorithm/attitude.h"

static uint32_t rng = 4242;
static double noise(double amp) {                 /* uniform, -amp .. +amp */
    rng = rng * 1103515245u + 12345u;
    return amp * ((double)((rng >> 16) & 0x7FFF) / 16383.5 - 1.0);
}
static fix16_t f16(double x) { return (fix16_t)(x * 65536.0 + (x >= 0 ? 0.5 : -0.5)); }
static double  dbl(fix16_t x) { return x / 65536.0; }

/* true motion: level, then 4 swings of +-20 deg, then level */
static void truth(double t, double *angle, double *rate) {
    if (t >= 10.0 && t < 50.0) {
        double w = 2.0 * M_PI * 0.1;
        *angle = 20.0 * sin(w * (t - 10.0));
        *rate  = 20.0 * w * cos(w * (t - 10.0));
    } else {
        *angle = 0.0;
        *rate  = 0.0;
    }
}

int main(void) {
    const double dt   = 0.002;
    const double bias = 0.5;          /* deg/s of gyro bias left after calibration */
    const int    N    = 60000;        /* 120 s */

    attitude_t att;
    attitude_init(&att, F16(0.998),F16(5.0),F16(0.002));  /* time constant about 1 s */

    double gyro_only = 0.0;
    double max_g = 0, max_a = 0, max_f = 0, sum_g = 0, sum_a = 0, sum_f = 0;
    int n = 0, burst_samples = 0, burst_accepted = 0, calm_samples = 0, calm_rejected = 0;

    printf("time,true,gyro_only,accel_only,filtered\n");

    for (int i = 0; i < N; i++) {
        double t = i * dt, ang, rate;
        truth(t, &ang, &rate);

        /* fake sensors */
        double gx = rate + bias + noise(0.5);
        double gy = bias + noise(0.5);
        double r  = ang * M_PI / 180.0;
        double ax = noise(0.05);
        double ay = sin(r) + noise(0.05);
        double az = cos(r) + noise(0.05);

        int burst = (t >= 70.0 && t < 72.0) || (t >= 90.0 && t < 92.0);
        if (burst) ay += 0.5;         /* fake sideways acceleration */

        /* the three estimates */
        gyro_only += gx * dt;
        double accel_only = atan2(ay, az) * 180.0 / M_PI;
        attitude_update(&att, f16(gx), f16(gy), f16(ax), f16(ay), f16(az), F16(0.002));
        double filt = dbl(att.roll);

        if (burst) { burst_samples++; if (att.acc_used) burst_accepted++; }
        else       { calm_samples++;  if (!att.acc_used) calm_rejected++; }

        if (t >= 1.0) {
            double eg = fabs(gyro_only - ang), ea = fabs(accel_only - ang), ef = fabs(filt - ang);
            if (eg > max_g) max_g = eg;
            if (ea > max_a) max_a = ea;
            if (ef > max_f) max_f = ef;
            sum_g += eg; sum_a += ea; sum_f += ef; n++;
        }

        if (i % 10 == 0)
            printf("%.3f,%.4f,%.4f,%.4f,%.4f\n", t, ang, gyro_only, accel_only, filt);
    }

    fprintf(stderr, "                 max error   mean error   (degrees)\n");
    fprintf(stderr, "gyro only      : %8.2f   %8.2f\n", max_g, sum_g / n);
    fprintf(stderr, "accel only     : %8.2f   %8.2f\n", max_a, sum_a / n);
    fprintf(stderr, "filtered       : %8.2f   %8.2f\n", max_f, sum_f / n);
    fprintf(stderr, "final pitch    : %8.2f   (true 0)\n", dbl(att.pitch));
    fprintf(stderr, "during bursts  : accelerometer accepted on %d of %d samples\n",
            burst_accepted, burst_samples);
    fprintf(stderr, "calm periods   : accelerometer rejected on %d of %d samples\n",
            calm_rejected, calm_samples);
    return 0;
}
