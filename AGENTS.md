# AGENTS.md

Guidance for Codex and other coding agents working in this repository.

## Project Overview

This is an STM32-based quadcopter/drone flight controller firmware project for an STM32L152RE target. The project is generated/configured with STM32CubeIDE/CubeMX and also has a CMake build path for `arm-none-eabi-gcc`.

The firmware is still work in progress. Treat behavior that can affect motors, arming, calibration, radio input, or IMU readings as safety-critical.

## Build Commands

Use an ARM embedded GCC toolchain on `PATH`:

```powershell
arm-none-eabi-gcc --version
cmake --version
```

Configure and build with CMake:

```powershell
cmake -S . -B cmake-build-debug-stm32-arm -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug-stm32-arm
```

Expected build outputs are:

- `cmake-build-debug-stm32-arm/stm32-fc.elf`
- `cmake-build-debug-stm32-arm/stm32-fc.hex`
- `cmake-build-debug-stm32-arm/stm32-fc.bin`

The STM32CubeIDE-generated `Debug/` directory also contains managed makefiles. If using CubeIDE, prefer building from the IDE unless intentionally validating the generated makefiles.

## Test and Verification

There is a host-side Unity/CMake test suite under `Tests/`. CI runs it on Ubuntu with:

```powershell
cmake -S Tests -B cmake-build-host-tests
cmake --build cmake-build-host-tests
ctest --test-dir cmake-build-host-tests --output-on-failure
```

On this Windows workstation, run host tests locally before pushing. A portable w64devkit toolchain is available at:

```powershell
$toolBin = "$env:USERPROFILE\.cache\codex-tools\w64devkit\2.8.0\w64devkit\bin"
$env:PATH = "$toolBin;$env:PATH"
cmake -S Tests -B cmake-build-host-tests-local -G Ninja -DCMAKE_C_COMPILER=gcc
cmake --build cmake-build-host-tests-local
ctest --test-dir cmake-build-host-tests-local --output-on-failure
```

Avoid naming Windows host-test executables with `setup` in the filename. Windows UAC installer detection can require elevation for names such as `test_setup_mode.exe`, which makes CTest report `BAD_COMMAND`/permission denied even when the test binary is otherwise valid.

Before handing changes back, at minimum:

```powershell
cmake --build cmake-build-debug-stm32-arm
```

For firmware behavior changes, also verify on hardware where possible:

- RC input capture still reports sane pulse widths for all six channels.
- ESC outputs remain bounded and idle safely before arming.
- Python dashboard tests pass when dashboard protocol/mapping changes are made:

```powershell
python -m unittest discover -s tools\hmi_dashboard -p "test_*.py"
```

- Setup/calibration mode is still entered only from the setup button at boot.
- UART/HMI output still works without blocking the main loop.
- IMU initialization and calibration complete before entering running mode.

## Repository Layout

- `Core/`: STM32CubeMX-generated startup, peripheral initialization, interrupt handlers, and `main.c`.
- `Drivers/`: STM32 HAL and CMSIS vendor code.
- `External/`: third-party sensor/device drivers, including BNO055/Waveshare IMU support.
- `HardwareLayer/`: board-attached hardware wrappers such as RC receiver capture, ESC PWM output, LEDs, UART/HMI output buffering, and IMU access.
- `Application/`: flight-controller application logic, setup mode, HMI screens, RC normalization, LED mode selection, IMU calibration/filtering, output mixing, and flight modes.
- `docs/`: hardware/software/interface notes and wiring tables.
- `stm32-fc.ioc`: CubeMX configuration. Keep this in sync with generated peripheral code.
- `STM32L152RETX_FLASH.ld` and `STM32L152RETX_RAM.ld`: linker scripts.
- `CMakeLists.txt`: generated from `CMakeLists_template.txt`; avoid manual edits unless regenerating the template too.

## Architecture Notes

The project intentionally follows three layers:

1. `Core` / HAL layer: MCU clocks, GPIO, timers, UART, I2C, startup, and interrupt dispatch.
2. `HardwareLayer`: device-level wrappers around actual connected hardware.
3. `Application`: business logic and flight behavior, independent of raw HAL details where practical.

Important runtime path:

- `Core/Src/main.c` initializes HAL, clocks, GPIO, I2C, UARTs, TIM2/TIM3/TIM4, BNO055, input capture, PWM, then calls `Application_Init(setupMode)`.
- The main loop repeatedly calls `Application_OnTick(HAL_GetTick())`.
- TIM3/TIM4 input capture callbacks route to `RC_TimerCallback()` for RC channels 1-6.
- UART callbacks feed `UARTInterface_OnReceive()` and `HMIOutput_OnSendComplete()`.
- EXTI setup-button callbacks call `Application_OnButtonRelease()`.
- `Application_OnTick()` drives setup, calibration, running, error, HMI, and LED behavior.
- `FlightMode_OnTick()` handles arming/disarming, mode switching, RC demand conversion, PID calculations, and motor mixing.

Current flight-tuning/IMU context:

- The active recovery/tuning branch is `codex/recover-flight-tuning`.
- `Application/Inc/config.h` is the main place for PID, mixer, RC, and IMU sign/axis constants.
- IMU angle signs and gyro-rate signs are intentionally separate. Do not collapse them back to one sign constant.
- Current IMU rate axis mapping was introduced because fast pitch movement was driving the yaw motor pair (`M1` + `M3`) instead of the pitch pair. The current mapping is:
  - Pitch rate: raw BNO055 gyro X
  - Roll rate: raw BNO055 gyro Y
  - Yaw rate: raw BNO055 gyro Z
- Current angle signs are:
  - Pitch angle sign `-1.0f`
  - Roll angle sign `1.0f`
  - Yaw angle sign `1.0f`
- Current rate signs are:
  - Pitch rate sign `1.0f`
  - Roll rate sign `-1.0f`
  - Yaw rate sign `1.0f`
- Current first-hover tuning is deliberately conservative: pitch/roll P `0.8`, I `0.0`, D `0.005`, PID output limit `180`, pitch/roll angle-to-rate gain `4.0`, yaw angle-to-rate gain `2.0`, max roll/pitch rate `180 dps`, max yaw rate `120 dps`. Pitch/roll angle-to-rate gain and max roll/pitch rate were raised after the first stable flight because stick authority was too weak; yaw keeps its separate lower angle gain because yaw hold was behaving well.
- Yaw rate PID is separate from pitch/roll: yaw P `1.0`, yaw I `0.15`, yaw D `0.0`. Yaw I is intentionally active only while the yaw stick is centered and throttle is at least `FM_YAW_INTEGRAL_MIN_THROTTLE` (`250`) so it can trim slow hands-off spin without winding up during commanded yaw or no-lift ground running.
- First stable prop-on hop happened after yaw low-throttle rebasing; yaw hold is behaving. The remaining pilot-command issue was reversed yaw stick input, so `FM_YAW_INPUT_SIGN` is currently `-1.0f`. This intentionally affects yaw stick demand only, not IMU yaw sign or yaw-left/yaw-right arming gates.
- Bench expectation with props off:
  - Nose down: corrected pitch negative; M1/M4 increase; fast nose-down should not drive M1/M3.
  - Right side down: corrected roll negative; M1/M2 increase.

Hardware mappings from `docs/Interfaces.md`:

- TIM2 CH1-CH4: ESC outputs on PA0, PA1, PB10, PB11.
- TIM3 CH1-CH4: RC inputs 1-4 on PA6, PA7, PC8, PC9.
- TIM4 CH1-CH2: RC inputs 5-6 on PB6, PB7.
- I2C1: BNO055/IMU on PB8/PB9.
- USART2: fixed UART on PA2/PA3.
- LEDs: green PA5, red PA12.

## Coding Conventions

- Preserve CubeMX `/* USER CODE BEGIN ... */` / `/* USER CODE END ... */` regions in generated files. Put custom edits inside user blocks whenever editing `Core/` generated files.
- Prefer adding application behavior under `Application/` and hardware-facing behavior under `HardwareLayer/` instead of putting more logic into `main.c`.
- Keep HAL-specific calls out of `Application/` unless there is already a local precedent.
- Maintain the existing C style: small C modules with matching headers, `uint*_t` fixed-width types, module-prefixed functions such as `Application_OnTick`, `RCInput_Calibrate`, `Output_SetMotorSpeeds`.
- Use the existing calibration constants in `Application/Inc/config.h` for PID, RC, and IMU tuning values.
- When changing IMU orientation, check both slow/static angle correction and fast gyro-rate correction. A slow tilt can look right while the rate loop is mapped to the wrong axis.
- Avoid blocking delays in the main loop or interrupt callbacks. The control path is tick/callback driven.
- Keep interrupt callbacks short. Dispatch to existing module callbacks rather than doing heavy work in ISR context.
- Be careful with shared state updated from callbacks and read from the main loop. If adding new shared values, consider `volatile` or a small accessor pattern.
- Treat motor output ranges defensively. Existing ESC demand is mixed as `0..1000` and written to TIM2 CCR values with a `+1000` offset.
- Do not edit files under `Drivers/` unless updating vendor code intentionally.
- Do not hand-edit generated build artifacts under `Debug/` or `cmake-build-debug-stm32-arm/`.

## Safety Notes

- Never assume motors are disconnected. Changes that affect arming, throttle, e-stop, setup mode, or ESC output should fail safe.
- Preserve e-stop behavior in `FlightMode_OnTick()` via RC channel 5 unless intentionally redesigning it.
- Keep arming gated by low throttle and yaw-left behavior; keep disarming gated by low throttle and yaw-right behavior unless explicitly changing the control scheme.
- After any motor-output change, verify initial outputs are zero/idle before the running state.
- After any IMU sign, gyro-axis, PID, or mixer change, repeat props-off bench checks before any prop-on test. A flip usually indicates sign/axis/motor-order error, not just gain.

## Git Hygiene

The working tree may contain user edits. Before modifying source files, check:

```powershell
git status --short
```

Do not revert user changes unless explicitly asked. Keep generated IDE metadata churn out of commits unless the task is specifically about project configuration.
