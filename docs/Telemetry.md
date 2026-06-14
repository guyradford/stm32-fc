# Telemetry Mode

Telemetry Mode is a machine-readable serial mode for diagnostics and ground-station display. The existing HMI remains the default serial mode.

## Entering and Exiting

From the HMI main menu:

```text
n - Telemetry Mode
```

Exit Telemetry Mode with either:

```text
h
$STOP*HH
```

`h` is a human escape key. `$STOP*HH` is the protocol command used by ground-station software.

## Frame Format

Telemetry uses compact, NMEA-inspired ASCII frames, but does not target NMEA compatibility:

```text
$<SUBJECT>,<fixed fields>*HH\r\n
```

`HH` is the uppercase hexadecimal XOR checksum of all bytes between `$` and `*`.

Each subject has a fixed field count. Numeric values use firmware-native units so the ground station can choose its own display format.

## Outgoing Subjects

```text
$RC,<ms>,<thr>,<yaw>,<pitch>,<roll>,<estop>,<aux>,<valid>*HH
```

Corrected RC input used by flight logic. Values are `0..1000`. `thr` is zero-based. `yaw`, `pitch`, `roll`, `estop`, and `aux` are centered around `500`.

```text
$RCR,<ms>,<ch1>,<ch2>,<ch3>,<ch4>,<ch5>,<ch6>,<validMask>,<maxAge>*HH
```

Raw receiver pulse widths in microseconds. Normal values are about `1000..2000`; accepted valid range is `900..2100`; invalid/no signal is reported as `0`. `validMask` uses bit 0 for channel 1 through bit 5 for channel 6. `maxAge` is the oldest channel age in milliseconds.

```text
$IMU,<ms>,<roll>,<pitch>,<yaw>,<rollRate>,<pitchRate>,<yawRate>,<ready>*HH
```

Corrected/filtered IMU state. Angles are centidegrees. Rates are centidegrees per second.

```text
$IMUC,<ms>,<calSys>,<calGyro>,<calMag>,<calAccel>,<ready>*HH
```

BNO055 calibration status. Calibration values are `0..3`; `3` means fully calibrated for that component.

```text
$IMUR,<ms>,<ax>,<ay>,<az>,<gx>,<gy>,<gz>,<mx>,<my>,<mz>*HH
```

Raw accelerometer, gyroscope, and magnetometer values in sensor-native integer units.

```text
$MOT,<ms>,<m1>,<m2>,<m3>,<m4>*HH
```

Motor demand in firmware-native `0..1000` units. Percent is a ground-station display concern.

```text
$STAT,<ms>,<app>,<flight>,<run>,<armed>,<rc>,<imu>,<failsafe>,<error>,<loopAge>*HH
```

Flight-controller status for ground-station display. This subject is reserved for the next firmware telemetry phase; the Python HMI already treats it as a first-class subject when received. `app`, `flight`, and `run` are short text enums. Boolean fields are `0` or `1`. `loopAge` is milliseconds.

## Default Stream

Telemetry Mode starts with:

```text
$RC   10 Hz
$IMU  10 Hz
$IMUC 1 Hz
$MOT  5 Hz
```

Raw streams default off and are available by request or subscription:

```text
$RCR
$IMUR
```

## Commands

Commands affect telemetry only. They must not affect arming, calibration, PID tuning, setup mode, flight mode, or motor output.

```text
$REQ,<subject>*HH
```

Emit one sentence immediately.

```text
$SUB,<subject>,<rateHz>*HH
```

Subscribe to a periodic sentence. `0` disables that subject. Supported rates are `0..50 Hz`.

```text
$STOP*HH
```

Stop all telemetry and return to HMI mode.

Responses:

```text
$ACK,<cmd>,<subject>*HH
$ERR,<code>,<detail>*HH
```

Examples:

```text
$REQ,RCR*HH
$SUB,IMUR,2*HH
$SUB,MOT,0*HH
$STOP*HH
```

## Reserved Subjects

These names are reserved for later phases:

```text
$PID   PID internals, setpoints, errors, outputs
$RCX   extended corrected RC
$RCRX  extended raw RC
$IMUX  extended corrected IMU/status
$IMURX extended raw IMU/status
```
