# Testing

This project has a host-side test harness for application logic that can run on a development machine or in CI. These tests do not replace bench verification on the real flight controller.

## Host Tests

Configure, build, and run the host tests from the repository root:

```powershell
cmake -S Tests -B cmake-build-host-tests
cmake --build cmake-build-host-tests
ctest --test-dir cmake-build-host-tests --output-on-failure
```

The host tests use Unity and CTest. They compile selected application modules with fake hardware-layer implementations, so they do not require `arm-none-eabi-gcc` or connected STM32 hardware.

On Windows, install a host C compiler and choose a matching CMake generator if CMake cannot find one automatically. For example, use Visual Studio Build Tools with a Visual Studio generator, or MinGW with the MinGW generator.

## Firmware Build

The firmware build remains separate and uses the generated root CMake project:

```powershell
cmake -S . -B cmake-build-debug-stm32-arm -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug-stm32-arm
```

## Safety

Changes that affect motors, arming, e-stop, setup/calibration mode, RC input, or IMU readings still need hardware verification with props removed. Run the relevant bench checks after host tests pass.

## Prop-Off Yaw Heading Check

Use this check after yaw-control changes, with propellers removed and the airframe secured. Select automatic flight mode, keep pitch and roll centered, arm at low throttle with yaw-left then release yaw to center, raise throttle just above idle, and observe the motor deltas from the commanded throttle value.

Centered yaw should hold the arming heading and produce no heading-relative motor bias:

| Heading at arming | Expected yaw PID | Motor 1 delta | Motor 2 delta | Motor 3 delta | Motor 4 delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| 0 deg | 0 | 0 | 0 | 0 | 0 |
| 90 deg | 0 | 0 | 0 | 0 | 0 |
| 180 deg | 0 | 0 | 0 | 0 | 0 |
| 270 deg | 0 | 0 | 0 | 0 | 0 |
| 359 deg | 0 | 0 | 0 | 0 | 0 |

For wraparound, arm at 359 deg, keep yaw centered, then rotate the airframe to 1 deg. The yaw PID should report about +2 deg of error, producing approximately -2, +2, -2, +2 motor deltas before clamping. Rotate from 1 deg to 359 deg and the signs should reverse.
