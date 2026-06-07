# ESC Calibration

Always remove propellers before calibrating ESCs.

The setup-mode HMI can drive all four ESC outputs through a simple throttle
calibration sequence. This is intended for basic PWM ESCs that learn throttle
endpoints from high/low pulse widths.

## Safety Gates

ESC calibration output is available only in setup mode. The firmware also
requires:

* Valid throttle and e-stop RC channels.
* E-stop released.
* Throttle low before any setup output is accepted.

If RC signal is lost, e-stop is active, or calibration is not in the explicit
high-output state, all ESC outputs are forced to idle.

## Procedure

1. Remove all propellers.
2. Power the transmitter and set throttle low.
3. Hold the setup button while powering the flight controller.
4. Open the serial HMI.
5. Press `c` for ESC throttle calibration.
6. Confirm e-stop is released and throttle is low.
7. Press `x` to command maximum ESC output.
8. Power the ESCs / quadcopter power rail and wait for the ESC high-throttle
   confirmation beeps.
9. Press `l` to command minimum ESC output and wait for the low-throttle
   confirmation beeps.
10. Press `d` to mark the calibration sequence done.
11. Power off the ESCs / quadcopter power rail.

Press `h` at any time to return home and command idle outputs.

## Notes

This does not attempt to navigate an ESC beep-menu. It only provides the high
then low throttle pulses used by basic ESC endpoint calibration. Use the ESC
manufacturer's beep-menu instructions for settings such as brake, timing, or
battery cutoff.
