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
2. Leave the ESCs / quadcopter power rail powered off.
3. Power the transmitter.
4. Set transmitter throttle low.
5. Release the transmitter e-stop switch.
6. Hold the flight-controller setup button.
7. Power the flight controller.
8. Release the setup button after the flight controller has booted.
9. Open the serial HMI.
10. Press `c` to enter ESC throttle calibration.
11. Confirm the HMI shows ESC calibration state `0`.
12. Confirm throttle is still low.
13. Confirm e-stop is still released.
14. Press `x`.
15. Confirm the HMI shows ESC calibration state `1`.
16. Power the ESCs / quadcopter power rail.
17. Wait for the ESC high-throttle confirmation beeps.
18. Press `l`.
19. Confirm the HMI shows ESC calibration state `2`.
20. Wait for the ESC low-throttle confirmation beeps.
21. Press `d`.
22. Confirm the HMI shows ESC calibration state `3`.
23. Power off the ESCs / quadcopter power rail.
24. Press `h` to return home and command idle outputs.

To abort calibration, press `h` to return home and command idle outputs.

## Notes

This does not attempt to navigate an ESC beep-menu. It only provides the high
then low throttle pulses used by basic ESC endpoint calibration. Use the ESC
manufacturer's beep-menu instructions for settings such as brake, timing, or
battery cutoff.
