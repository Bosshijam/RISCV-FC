Flight controller firmware for the ARIES v3 board, built around the indigenous VEGA ET1031 RISC-V processor (THEJAS32 SoC).

Developed for NIDAR 2.0 — an Indian drone innovation challenge requiring an autopilot built on Indian processor platforms with non-Chinese components.

Why this exists

No flight controller firmware runs on this chip. ArduPilot and PX4 cannot be ported to it:

Constraint	ARIES v3	ArduPilot needs
Program size	250 KB, runs from RAM	1–2 MB, executes from flash
Floating point	None (RV32IM)	Float maths throughout
RTOS	None available	ChibiOS / NuttX

So the firmware is written from scratch — but not from nothing. Proven algorithms are adapted from Betaflight and iNav, which were designed for small chips without FPUs, and MAVLink is used unchanged for ground station communication.

Current state
Component	Status
Fixed-point math library (Q16.16)	Complete — 27 unit tests, 400,000 random trials
Trigonometry (sin / cos / atan2)	Complete — lookup table, ±0.5°
One-axis plant simulator	Complete
PID controller (P, I, D + anti-windup)	Complete — 14.5% overshoot, zero steady-state error
Plotting and analysis tools	Complete
Digital filters (PT1, notch)	In progress
Attitude estimation	Planned
Sensor drivers	Planned

Everything in algorithm/ cross-compiles for the VEGA and is verified as 32-bit RISC-V object code.

Repository layout
algorithm/       portable C — no hardware headers
  fix16.c/.h       Q16.16 fixed-point arithmetic
  sin_table.h      generated lookup table, 257 entries
  pid.c/.h         PID controller
sim/
  plant.c/.h       one-axis rigid body model
test/            unit tests and simulation runners
tools/
  gen_sin_table.c  generates sin_table.h
  plot.py          plots CSV output, reports overshoot
docs/
  CONVENTIONS.md   Q-format, units and ranges for every quantity
  TUNING.md        tuning log — one row per experiment
  TOOLCHAIN.md     working setup notes
  PROGRESS.md      detailed progress report

Design rule: nothing in algorithm/ includes a hardware header. The same pid.c compiles for the laptop simulator and for the VEGA. When real sensors arrive, only the input source changes.

That is what allowed the controller to be built, tested and tuned before the board was available.

Fixed point, not floating point

The THEJAS32 has no FPU, so every float operation becomes a software emulation call — far too slow for a 500 Hz control loop.

All maths uses Q16.16: a decimal stored as a 32-bit integer scaled by 65536. 1.5 is stored as 98304.

c
typedef int32_t fix16_t;
#define F16(x) ((fix16_t)((x) * 65536.0))    /* compile-time constants only */

Addition and subtraction work unchanged. Multiplication needs a right-shift by 16 with a 64-bit intermediate; division needs a left-shift by 16 first. All operations saturate rather than wrap — a wrapped integer in a flight controller means a correction that suddenly reverses direction.

Building and testing

Everything here runs on a normal Linux laptop — no board required.

bash
# unit tests
gcc -Wall -Wextra -o test/run_fix16 test/test_fix16.c algorithm/fix16.c
./test/run_fix16

# randomised tests (400k trials)
gcc -Wall -Wextra -o test/run_random test/test_random.c algorithm/fix16.c -lm
./test/run_random

# PID against the simulator, then plot
gcc -Wall -Wextra -o test/run_pid test/test_pid.c sim/plant.c algorithm/pid.c algorithm/fix16.c
./test/run_pid > /tmp/pid.csv
python3 tools/plot.py /tmp/pid.csv 10

Cross-compile check for the target:

bash
riscv64-vega-elf-gcc -c -O2 -march=rv32im -mabi=ilp32 algorithm/pid.c -o /tmp/pid.o
riscv64-vega-elf-objdump -f /tmp/pid.o     # -> elf32-littleriscv, riscv:rv32
Tuning results

Plant: inertia 1.0, no damping. Setpoint 10°, dt = 2 ms.

kp	ki	kd	Overshoot	Final angle
2.0	0	0	100%	oscillates forever
2.0	0	1.0	30.5%	12.13 (permanent error)
2.0	0.5	1.0	52.7%	10.00
1.0	0.1	2.0	14.5%	10.00

One finding worth recording: lowering P made overshoot worse, not better. A slower approach leaves the error large for longer, which gives the integrator more time to wind up. P and I cannot be tuned independently on this plant.

Full log in docs/TUNING.md.

Hardware
	
Board	ARIES v3.0 (C-DAC)
SoC	THEJAS32
Core	VEGA ET1031, RV32IM, 100 MHz, no FPU
RAM	256 KB total, 250 KB usable (bootloader reserves the top 6 KB)
Peripherals	3 UART, 4 SPI, 3 I2C, 8 PWM, 3 timers, 32 GPIO

Toolchain: riscv64-vega-elf GCC, Taurus SDK, vega-xmodem for flashing. Setup notes in docs/TOOLCHAIN.md.

References
VEGA Processors — C-DAC
Taurus SDK — community rewrite of the official VEGA SDK
Betaflight — reference for PID structure, filters, mixer
iNav — reference for navigation and position hold
MAVLink — ground station protocol
