# STM32-FC HMI Dashboard

Phase 1 is a visual-only Tkinter dashboard prototype for the flight controller. It lives in the firmware repository because the layout is tied to the project motor order, RC channel meanings, and bench workflow.

## Run

From the repository root:

```powershell
python tools\hmi_dashboard\hmi_dashboard.py
```

If `python` is not on PATH, use the Windows launcher:

```powershell
py -3 tools\hmi_dashboard\hmi_dashboard.py
```

No serial device, firmware changes, telemetry sentences, or third-party Python packages are required for phase 1. The dashboard is driven by simulated data at about 10 Hz.

## Phase 1 Scope

- Physical motor map with vertical bars:
  - M1 front-right
  - M2 back-right
  - M3 back-left
  - M4 front-left
- RC panel with vertical throttle and pitch, horizontal yaw and roll, e-stop state, channel 6, and validity lights.
- IMU panel with roll across the top, yaw compass in the middle, pitch on the side, rates, and health indicators.
- Top safety/status strip and bottom event log.

## Deliberately Out Of Scope

- Serial connection.
- NMEA-style sentence parsing or formatting.
- Firmware telemetry output.
- Any command that can arm, tune, calibrate, or affect motor output.
