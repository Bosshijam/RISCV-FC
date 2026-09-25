Control Systems — Progress Report

Owner: control loops, state estimation Period: 14 Aug – 25 Sep 2026 Status: PID controller complete and tuned in simulation. All control code cross-compiles for the VEGA.

1. Summary
Item	State	Evidence
Fixed-point math library	Done	27 unit tests + 400,000 random trials, all passing
Trig (sin / cos / atan2)	Done	Lookup table + interpolation, accurate to ±0.5°
One-axis plant simulator	Done	Verified against hand-calculated physics
PID controller (P, I, D)	Done	14.5% overshoot, zero steady-state error
Anti-windup clamp	Done	Integral bounded, tested
Log plotting tool	Done	Python, reads CSV, reports overshoot automatically
Filters (PT1, notch)	Next	Required before real sensor data
Complementary filter	Next	Attitude estimation from gyro + accel

Everything in algorithm/ compiles for the board, not just the laptop:

bash
riscv64-vega-elf-gcc -c -O2 -march=rv32im -mabi=ilp32 algorithm/fix16.c -o /tmp/fix16.o
riscv64-vega-elf-objdump -f /tmp/fix16.o     # -> elf32-littleriscv, riscv:rv32

Current footprint: fix16.o is 3,244 bytes text, 0 data, 0 bss — about 1.3% of the 250 KB budget.

2. Architecture
flight-controller/
├── algorithm/          portable C — no hardware headers
│   ├── fix16.c/.h      Q16.16 fixed-point maths
│   ├── sin_table.h     generated, 257 entries
│   └── pid.c/.h        PID controller
├── sim/
│   └── plant.c/.h      one-axis physics model
├── test/               unit tests and simulation runners
├── tools/
│   ├── gen_sin_table.c generates sin_table.h
│   └── plot.py         plots CSV, reports overshoot
└── docs/
    ├── CONVENTIONS.md  Q-format table
    └── TUNING.md       tuning log, one row per experiment

The rule that makes this work: nothing in algorithm/ includes a hardware header. The same pid.c compiles for the laptop simulator and for the VEGA. When real sensors arrive, only the input source changes — the control code does not.

That is why the controller could be built, tested and tuned before the board was available.

3. Fixed-point arithmetic

The THEJAS32 has no floating-point unit (RV32IM). Every float operation becomes a software emulation call — tens to hundreds of cycles. At 500 Hz that does not fit in the time budget.

All maths therefore uses Q16.16: a decimal stored as a 32-bit integer scaled by 65536. 1.5 is stored as 98304.

c
typedef int32_t fix16_t;

#define FIX16_ONE  65536
#define F16(x) ((fix16_t)((x) * 65536.0))    /* compile-time constants only */

Addition and subtraction work unchanged — the scale factor survives. Multiplication doubles the scale factor, so it needs a right-shift by 16 and a 64-bit intermediate:

c
fix16_t fix16_mul(fix16_t a, fix16_t b) {
    int64_t product = (int64_t)a * (int64_t)b;
    product += 32768;          /* round to nearest, not toward zero */
    product >>= 16;            /* remove the second scale factor */
    if (product > FIX16_MAX) return FIX16_MAX;
    if (product < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)product;
}

Division cancels the scale factor entirely, so the numerator is shifted left first:

c
fix16_t fix16_div(fix16_t a, fix16_t b) {
    if (b == 0) return (a >= 0) ? FIX16_MAX : FIX16_MIN;   /* never trap in flight */
    int64_t result = ((int64_t)a << 16) / b;
    if (result > FIX16_MAX) return FIX16_MAX;
    if (result < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)result;
}

All operations saturate rather than wrap. In plain C, adding two large integers past the limit silently produces a large negative number — which in a flight controller means a correction that suddenly reverses direction. Saturation is still wrong, but wrong in a safe direction.

Good news on performance

RV32IM has hardware MUL and MULH, so a 32×32→64 multiply is two instructions rather than a slow library call. Fixed point is cheap on this chip.

Three bugs found during development
Bug	Symptom	Found by
fix16_mul rounded negatives the wrong way	Off by one on every negative multiply	Unit test with a negative operand
fix16_sin missing a << 16	Every angle returned near zero	Printing intermediate values
pid.h had duplicated members after a bad edit	Build silently failed; test ran a stale binary	Cross-compile step

Two of the three were scale-factor errors — the characteristic mistake of fixed-point work. None would have been found by reading the code. Each was caught by a test or by printing an intermediate value.

The third is worth noting for process reasons: the build was failing while the test still ran an old binary, so the results looked stable but were meaningless. Cross-compiling before every test run now catches this.

4. Trigonometry by lookup table

atan2 is needed to derive roll and pitch from the accelerometer. Computing it properly is far too slow for a 500 Hz loop, so sin and cos come from a 257-entry table covering 0°–90°, generated once on the laptop:

c
for (int i = 0; i <= ENTRIES; i++) {
    double degrees = 90.0 * i / ENTRIES;
    double radians = degrees * M_PI / 180.0;
    int32_t fixed  = (int32_t)(sin(radians) * 65536.0 + 0.5);
    printf("    %10d,   /* %7.3f deg */\n", fixed, degrees);
}

The full circle is covered by symmetry — sin(180−x) = sin(x), sin(180+x) = −sin(x) — and values between entries are linearly interpolated. Entries sit 0.352° apart, so interpolation error is far below sensor noise.

atan2 uses a polynomial approximation rather than a second table, with no squaring and no division by a possibly-zero denominator.

Memory cost: 257 × 4 bytes ≈ 1 KB, marked static const so it lands in flash rather than RAM.

5. Plant simulator

A one-axis rigid body. Six lines of physics:

c
void plant_step(plant_t *p, fix16_t torque, fix16_t dt) {
    fix16_t accel = fix16_div(torque, p->inertia);        /* Newton */
    p->rate  = fix16_add(p->rate,  fix16_mul(accel, dt)); /* integrate once  */
    p->angle = fix16_add(p->angle, fix16_mul(p->rate, dt));/* integrate twice */
}

It does not model our actual airframe, and does not need to. What it needs is to be unstable in the same way — nothing stops it, so a controller that stabilises this will stabilise a real one.

Verified against hand calculation: torque 10, inertia 1, after one second the rate reads 9.99 °/s (expect 10) and the angle 5.00° (expect 5, because it accelerated from zero). The small discrepancy is discretisation error from 2 ms steps.

Why this matters to the schedule: it allows the controller to be tested in seconds rather than flights, with zero crash risk, and provides ground-truth angle — something impossible to obtain from a real aircraft.

6. PID controller
c
fix16_t pid_update(pid_t *pid, fix16_t setpoint, fix16_t measurement, fix16_t dt) {
    fix16_t error = fix16_sub(setpoint, measurement);

    /* P */
    fix16_t p_term = fix16_mul(pid->kp, error);

    /* I — accumulate error over time, clamped against windup */
    pid->integral = fix16_add(pid->integral, fix16_mul(error, dt));
    if (pid->integral >  pid->integral_limit) pid->integral =  pid->integral_limit;
    if (pid->integral < fix16_sub(0, pid->integral_limit))
        pid->integral = fix16_sub(0, pid->integral_limit);
    fix16_t i_term = fix16_mul(pid->ki, pid->integral);

    /* D — on measurement, not error, then negated */
    fix16_t change     = fix16_sub(measurement, pid->prev_measurement);
    fix16_t derivative = fix16_div(change, dt);
    fix16_t d_term     = fix16_mul(pid->kd, fix16_sub(0, derivative));

    pid->prev_measurement = measurement;
    return fix16_add(fix16_add(p_term, i_term), d_term);
}
Three design decisions worth stating

D is computed on the measurement, not the error, and negated. If the error were differentiated, then the moment a pilot moves a stick the setpoint jumps, the error jumps with it, and D produces an enormous spike — "derivative kick". Differentiating the measurement gives identical damping against disturbances and no kick on commands.

dt is a parameter, not a constant. Real loop timing varies — interrupts fire, bus transactions take longer than expected. Hard-coding dt would make the gains loop-rate-dependent, so the aircraft would silently retune itself whenever timing shifted. On hardware this will be measured with hal_micros().

The integral clamp is not optional. Without it, if the motors saturate and the error persists, the integrator grows without bound. When the aircraft finally responds, that stored-up integral drives a violent overshoot.

7. Tuning results

Plant: inertia 1.0, no damping. Setpoint 10°, dt = 2 ms, 100 s run.

#	kp	ki	kd	Overshoot	Final	Note
1	2.0	0	0	100%	oscillates	P only — never settles
2	2.0	0	1.0	30.5%	12.13	D added, but permanent error
3	2.0	0.5	1.0	52.7%	10.00	I fixes error, costs overshoot
4	2.0	0.5	2.0	26.7%	10.01	doubled D
5	2.0	0.5	4.0	24.0%	9.92	diminishing returns on D
6	1.0	0.5	2.0	46.5%	10.05	lower P made it worse
7	1.0	0.1	2.0	14.5%	10.33	cut ki — windup confirmed
8	1.0	0.1	2.0	14.5%	10.00	converges fully — current best
What each term demonstrably did

P alone oscillates forever. At the moment the angle reaches the target the error is zero, so P commands nothing — but the body is still rotating at 14 °/s and nothing slows it. It sails past, the error reverses, and it swings back. A pure sine wave that never settles.

D stopped the oscillation. Overshoot fell from 100% to 30.5%. D is the only term that responds to how fast the measurement is changing, so it brakes on approach.

I removed the steady-state error. P-D settled at 12.13° and stayed there permanently — P was producing just enough torque to balance whatever held it, and neither side could win. On an aircraft that is a permanent 2° lean and continuous sideways drift. Adding I brought the final angle to exactly 10.00.

The non-obvious finding

Run 6 contradicts the standard advice. Reducing P should reduce overshoot. Instead it nearly doubled it, from 26.7% to 46.5%.

Cause: weak P means a slow approach, which means the error stays large for longer, which means the integrator has more time to wind up. By the time the system arrives, I has built a large stored correction that keeps pushing after P has gone quiet.

Confirmed in run 7 by cutting ki from 0.5 to 0.1 with everything else unchanged: overshoot dropped from 46.5% to 14.5%.

Practical consequence for the aircraft: P and I cannot be tuned independently. Lowering P without also lowering I will make things worse — the opposite of what most tuning guides suggest. This will matter during the real tuning campaign in November.

A process point

Two working theories were wrong during this session, and both were killed by measurement rather than argument:

"Adding I made it worse" — it had not; the test window was too short and the system was still converging
"The integral saturated at its limit" — it had not; printing the raw value showed 1.27 against a limit of 50

The habit of printing intermediate values before changing anything is what made run 6 a finding rather than a dead end. The same method will be needed in the field, where each experiment costs a battery and a flight.

8. Tooling

Plot script reads the simulator's CSV output and reports the numbers that matter, so tuning is comparison rather than estimation:

saved plot.png
  final angle : 10.00
  peak angle  : 11.45
  overshoot   : 14.5%

Tuning log (docs/TUNING.md) records one row per experiment — gains in, overshoot out. Tuning is comparison, and without a written record the work goes in circles. In November each row will cost a battery and a walk out to the field.

Conventions file (docs/CONVENTIONS.md) fixes the Q-format, unit and range of every quantity in the firmware. Most bugs that are not timing bugs are sign errors or unit errors; writing the conventions down is the cheapest debugging available.

9. Next steps

All of the following run on the laptop and need no hardware.

Add noise to the simulated gyro and observe the D term amplify it. This is the problem filters exist to solve, and it is better seen than described.
PT1 low-pass filter on the gyro and on the D term specifically.
Gyro bias in the simulator, plus a boot-time calibration routine.
Output saturation and a one-loop delay — proves the anti-windup clamp earns its place.
Complementary filter — fuse simulated gyro and accelerometer into a trusted angle, with accelerometer rejection when the measured magnitude is not close to 1 g.
Angle loop on top of the rate loop, then the motor mixer, then three axes.
Two caveats carried into the next phase

These gains are for this simulator only. A real quad has different inertia, air resistance, motor lag and battery sag. The procedure transfers; the numbers do not.

The simulated gyro is perfect. kd = 2.0 works because there is no noise. On real hardware that D term will amplify sensor noise into motor buzz and heat, and the gains will need backing off once filters are in place. Step 1 above exists to find that limit before the aircraft does.

Report generated 25 September 2026. Tuning log and conventions live in docs/ in the repo.
