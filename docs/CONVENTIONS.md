# Conventions

## Q-format table

| Quantity      | Unit  | Format          | Range        |
|---------------|-------|-----------------|--------------|
| Roll angle    | deg   | Q16.16          | -180..+180   |
| Pitch angle   | deg   | Q16.16          | -180..+180   |
| Yaw angle     | deg   | Q16.16          | 0..360       |
| Roll rate     | deg/s | Q16.16          | -2000..+2000 |
| Pitch rate    | deg/s | Q16.16          | -2000..+2000 |
| Yaw rate      | deg/s | Q16.16          | -2000..+2000 |
| PID gain kp   | -     | Q16.16          | 0..100       |
| Motor output  | -     | Q15 (int16_t)   | 0..1         |
| Raw gyro      | counts| int16_t (raw)   | -32768..32767|
| Altitude      | m     | Q16.16          | -100..+1000  |
| Battery volts | V     | Q16.16          | 0..30        |
