# Tuning log — one-axis simulator

Plant: inertia 1.0, no damping. Setpoint 10 deg. dt = 2 ms, 20 s run.

| # | kp  | ki  | kd  | i_lim | overshoot | final | note                    |
|---|-----|-----|-----|-------|-----------|-------|-------------------------|
| 1 | 2.0 | 0   | 0   | -     | 100.0%    | osc   | P only, never settles   |
| 2 | 2.0 | 0   | 1.0 | -     | 30.5%     | 12.13 | D added, but SS error   |
| 3 | 2.0 | 0.5 | 1.0 | 50    | 52.7%     | 10.00 | I fixes SS, costs overshoot |
| 4 | 2.0 | 0.5 | 2.0 | 50    | 26.7%     | 10.01 | doubled D               |
| 5 | 2.0 | 0.5 | 4.0 | 50    | 24.0%     | 9.92  | diminishing returns on D |
| 6 | 1.0 | 0.5 | 2.0 | 50    | 46.5%     | 10.05 | LOWER P made it WORSE - I winds up during slow approach |
| 7 | 1.0 | 0.1 | 2.0 | 50    | 14.5%     | 10.33 | cut ki - windup theory confirmed |
| 8 | 1.0 | 0.1 | 2.0 | 50    | 14.5%     | 10.00 | converges fully - GOOD |
| 9 | 1.0 | 0.1 | 2.0 | 50  | 14.4%       | 10.00 |       0.1 deg noise, 20 Hz D filter |
| 9  | 1.0 | 0.1 | 2.0 | 50 | 14.4%       | 10.00 |      0.1 deg noise, 20 Hz D filter |
| 10 | 1.0 | 0.1 | 2.0 | 50 | 14.1%       | 10.00 |      5 Hz D filter, lag cost not visible on this slow plant |
| 12 | 1.0 | 0.1 | 2.0 | 50 | 14.4%       | 10.00 |      params struct refactor, matches run 9 |
