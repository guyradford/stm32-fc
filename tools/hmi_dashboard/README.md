# STM32-FC HMI Dashboard

The Tkinter dashboard is a bench telemetry tool for the flight controller. It lives in the firmware repository because the layout is tied to the project motor order, RC channel meanings, and bench workflow.

The dashboard starts in live-ready mode. It does not open the serial port until Connect is pressed.

## Install

Install the Python serial dependency:

```powershell
python -m pip install -r tools\hmi_dashboard\requirements.txt
```

## Run

From the repository root:

```powershell
python tools\hmi_dashboard\hmi_dashboard.py
```

If `python` is not on PATH, use the Windows launcher:

```powershell
py -3 tools\hmi_dashboard\hmi_dashboard.py
```

Select the serial port, keep the baud rate at `115200` for the Nucleo USB VCOM, and press Connect. USART1 wireless telemetry normally uses `57600`.

On connect, the dashboard sends `h`, then `n`, so the firmware can leave any current HMI screen and enter Telemetry Mode. On disconnect it sends `$STOP*HH`.

Simulator mode is available from the Mode dropdown for UI work without hardware.

## Live Telemetry Scope

- Physical motor map with vertical bars:
  - M1 front-right
  - M2 back-right
  - M3 back-left
  - M4 front-left
- RC panel with vertical throttle and pitch, horizontal yaw and roll, e-stop state, channel 6, and validity lights.
- IMU panel with roll across the top, yaw compass in the middle, pitch on the side, rates, `$IMUC` calibration/health indicators, and `$PID` demand markers overlaid on roll/pitch/yaw.
- Top safety/status strip and bottom event log.
- First-class subjects: `$STAT`, `$RC`, `$IMU`, `$IMUC`, `$MOT`, `$PID`.
- Log-only subjects: `$ACK`, `$ERR`, `$RCR`, `$IMUR`, and other unsupported valid telemetry frames.

Until firmware emits `$STAT`, app mode, flight mode, run mode, armed state, and loop age remain `UNKNOWN` or stale in live mode.

## Deliberately Out Of Scope

- Firmware `$STAT` emission.
- Raw RC/IMU dashboard panels.
- Any command that can arm, tune, calibrate, or affect motor output.

## Tests

Run the pure Python parser and state mapping tests:

```powershell
python -m unittest discover -s tools\hmi_dashboard -p "test_*.py"
```
