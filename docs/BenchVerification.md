# Bench Verification

Always remove propellers before running motor, arming, mixer, or IMU sign checks.

## IMU and Mixer Sign Check

Use the HMI main menu option `b - Prop-off Bench Verification` to show the expected automatic correction trends for the documented motor layout:

| Motor | Position |
| --- | --- |
| M1 | Front right |
| M2 | Back right |
| M3 | Back left |
| M4 | Front left |

With props removed, arm only in a controlled bench setup and gently tilt the frame. The corrected IMU angle and motor output trend should match this table:

| Frame tilt | Corrected IMU sign | Expected motor trend |
| --- | --- | --- |
| Nose down | Pitch negative | M1 up, M2 down, M3 down, M4 up |
| Nose up | Pitch positive | M1 down, M2 up, M3 up, M4 down |
| Right side down | Roll negative | M1 up, M2 up, M3 down, M4 down |
| Left side down | Roll positive | M1 down, M2 down, M3 up, M4 up |

The HMI also prints current motor outputs so the live trend can be compared with the expected trend while the frame is tilted.

## Correction Rule

Change the IMU sign first when the automatic correction is backwards for the physical tilt. The sign constants are in `Application/Inc/config.h`:

```c
#define IMU_INPUT_PITCH_SIGN -1.0f
#define IMU_INPUT_ROLL_SIGN   1.0f
#define IMU_INPUT_YAW_SIGN    1.0f
```

Change mixer signs only when the physical motor wiring, numbering, or frame layout differs from the documented layout above. The mixer constants are also in `Application/Inc/config.h`.
