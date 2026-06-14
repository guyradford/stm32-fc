from __future__ import annotations

import time

from dashboard_state import DashboardState
from telemetry_protocol import TelemetryFrame


STALE_SECONDS = 1.5


def apply_frame(state: DashboardState, frame: TelemetryFrame, received_at: float | None = None) -> None:
    now = time.monotonic() if received_at is None else received_at
    state.status.connected = True
    state.status.telemetry_active = True
    state.last_rx_s = now

    if frame.subject == "RC":
        _apply_rc(state, frame, now)
    elif frame.subject == "IMU":
        _apply_imu(state, frame, now)
    elif frame.subject == "IMUC":
        _apply_imuc(state, frame, now)
    elif frame.subject == "MOT":
        _apply_mot(state, frame, now)
    elif frame.subject == "STAT":
        _apply_stat(state, frame, now)


def mark_stale(state: DashboardState, now: float | None = None) -> None:
    current = time.monotonic() if now is None else now
    state.status.telemetry_active = state.last_rx_s is not None and current - state.last_rx_s <= STALE_SECONDS
    state.rc.stale = state.last_rc_s is None or current - state.last_rc_s > STALE_SECONDS
    state.imu.stale = state.last_imu_s is None or current - state.last_imu_s > STALE_SECONDS
    state.motors.stale = state.last_mot_s is None or current - state.last_mot_s > STALE_SECONDS

    if state.last_stat_s is None or current - state.last_stat_s > STALE_SECONDS:
        state.status.app_mode = "UNKNOWN"
        state.status.flight_mode = "UNKNOWN"
        state.status.run_mode = "UNKNOWN"
        state.status.armed = False
        state.status.error = False


def reset_live_state(state: DashboardState) -> None:
    state.status.connected = False
    state.status.telemetry_active = False
    state.status.imu_ready = False
    state.status.rc_valid = False
    state.status.armed = False
    state.status.error = False
    state.status.app_mode = "UNKNOWN"
    state.status.flight_mode = "UNKNOWN"
    state.status.run_mode = "UNKNOWN"
    state.status.loop_age_ms = 0
    state.last_rx_s = None
    state.last_rc_s = None
    state.last_imu_s = None
    state.last_mot_s = None
    state.last_stat_s = None
    state.rc.stale = True
    state.rc.channel_valid = [False] * 6
    state.imu.stale = True
    state.imu.ready = False
    state.imu.fusion = False
    state.imu.error = False
    state.motors.stale = True


def _apply_rc(state: DashboardState, frame: TelemetryFrame, now: float) -> None:
    _ms, throttle, yaw, pitch, roll, estop, aux, valid = frame.fields
    rc_valid = _to_bool(valid)

    state.rc.throttle = _to_int(throttle)
    state.rc.yaw = _to_int(yaw) - 500
    state.rc.pitch = 500 - _to_int(pitch)
    state.rc.roll = 500 - _to_int(roll)
    state.rc.estop_safe = _to_int(estop) >= 500
    state.rc.channel_6 = _to_int(aux)
    state.rc.channel_valid = [rc_valid] * 6
    state.rc.stale = False
    state.status.rc_valid = rc_valid
    state.last_rc_s = now


def _apply_imu(state: DashboardState, frame: TelemetryFrame, now: float) -> None:
    _ms, roll, pitch, yaw, roll_rate, pitch_rate, yaw_rate, ready = frame.fields
    imu_ready = _to_bool(ready)

    state.imu.roll_deg = _to_int(roll) / 100.0
    state.imu.pitch_deg = _to_int(pitch) / 100.0
    state.imu.yaw_deg = _to_int(yaw) / 100.0
    state.imu.roll_rate_dps = _to_int(roll_rate) / 100.0
    state.imu.pitch_rate_dps = _to_int(pitch_rate) / 100.0
    state.imu.yaw_rate_dps = _to_int(yaw_rate) / 100.0
    state.imu.ready = imu_ready
    state.imu.fusion = imu_ready
    state.imu.stale = False
    state.status.imu_ready = imu_ready
    state.last_imu_s = now


def _apply_imuc(state: DashboardState, frame: TelemetryFrame, now: float) -> None:
    _ms, cal_sys, cal_gyro, cal_mag, cal_accel, ready = frame.fields
    imu_ready = _to_bool(ready)

    state.imu.cal_sys = _to_int(cal_sys)
    state.imu.cal_gyro = _to_int(cal_gyro)
    state.imu.cal_mag = _to_int(cal_mag)
    state.imu.cal_accel = _to_int(cal_accel)
    state.imu.ready = imu_ready
    state.status.imu_ready = imu_ready


def _apply_mot(state: DashboardState, frame: TelemetryFrame, now: float) -> None:
    _ms, m1, m2, m3, m4 = frame.fields
    state.motors.m1_front_right = _to_int(m1)
    state.motors.m2_back_right = _to_int(m2)
    state.motors.m3_back_left = _to_int(m3)
    state.motors.m4_front_left = _to_int(m4)
    state.motors.stale = False
    state.last_mot_s = now


def _apply_stat(state: DashboardState, frame: TelemetryFrame, now: float) -> None:
    _ms, app, flight, run, armed, rc_valid, imu_ready, failsafe, error, loop_age = frame.fields
    state.status.app_mode = app
    state.status.flight_mode = flight
    state.status.run_mode = "FAILSAFE" if _to_bool(failsafe) else run
    state.status.armed = _to_bool(armed)
    state.status.rc_valid = _to_bool(rc_valid)
    state.status.imu_ready = _to_bool(imu_ready)
    state.status.error = _to_bool(error)
    state.status.loop_age_ms = _to_int(loop_age)
    state.last_stat_s = now


def _to_int(text: str) -> int:
    return int(text, 10)


def _to_bool(text: str) -> bool:
    return _to_int(text) != 0
