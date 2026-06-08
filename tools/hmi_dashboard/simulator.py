from __future__ import annotations

import math
import time

from dashboard_state import DashboardState


class DashboardSimulator:
    """Generates phase-1 sample data without touching serial or firmware."""

    def __init__(self) -> None:
        self._started = time.monotonic()
        self._last_log_second = -1

    def update(self, state: DashboardState) -> None:
        elapsed = time.monotonic() - self._started
        phase = elapsed * 1.35

        state.status.connected = True
        state.status.telemetry_active = True
        state.status.imu_ready = True
        state.status.rc_valid = int(elapsed) % 18 != 12
        state.status.armed = int(elapsed) % 20 >= 10
        state.status.error = int(elapsed) % 37 == 18
        state.status.app_mode = "RUNNING"
        state.status.flight_mode = "AUTO_LEVEL"
        state.status.run_mode = "ARMED" if state.status.armed else "DISARMED"
        state.status.loop_age_ms = 35 + int(12 * abs(math.sin(phase * 0.7)))

        state.rc.throttle = self._clamp_int(520 + 350 * math.sin(phase * 0.35), 0, 1000)
        state.rc.yaw = self._clamp_int(280 * math.sin(phase * 0.7), -500, 500)
        state.rc.pitch = self._clamp_int(360 * math.sin(phase * 0.45 + 1.4), -500, 500)
        state.rc.roll = self._clamp_int(320 * math.sin(phase * 0.55 - 0.8), -500, 500)
        state.rc.estop_safe = int(elapsed) % 24 != 16
        state.rc.channel_6 = self._clamp_int(500 + 460 * math.sin(phase * 0.22), 0, 1000)
        state.rc.channel_valid = [state.status.rc_valid] * 6

        base = max(0, state.rc.throttle - 180)
        roll_mix = state.rc.roll * 0.22
        pitch_mix = state.rc.pitch * 0.22
        state.motors.m1_front_right = self._clamp_int(base - roll_mix - pitch_mix, 0, 1000)
        state.motors.m2_back_right = self._clamp_int(base - roll_mix + pitch_mix, 0, 1000)
        state.motors.m3_back_left = self._clamp_int(base + roll_mix + pitch_mix, 0, 1000)
        state.motors.m4_front_left = self._clamp_int(base + roll_mix - pitch_mix, 0, 1000)
        state.motors.stale = not state.status.telemetry_active

        state.imu.roll_deg = 28.0 * math.sin(phase * 0.45)
        state.imu.pitch_deg = 22.0 * math.sin(phase * 0.35 + 0.9)
        state.imu.yaw_deg = (elapsed * 32.0) % 360.0
        state.imu.roll_rate_dps = 12.0 * math.cos(phase * 0.45)
        state.imu.pitch_rate_dps = 8.0 * math.cos(phase * 0.35 + 0.9)
        state.imu.yaw_rate_dps = 32.0
        state.imu.ready = True
        state.imu.fusion = True
        state.imu.error = state.status.error
        state.imu.cal_sys = 3
        state.imu.cal_gyro = 3
        state.imu.cal_mag = 2 + (int(elapsed) % 6 == 0)
        state.imu.cal_accel = 3

        state.pid.yaw_setpoint = state.rc.yaw / 100.0
        state.pid.pitch_setpoint = state.rc.pitch / 100.0
        state.pid.roll_setpoint = state.rc.roll / 100.0
        state.pid.yaw_output = 0.12 * state.rc.yaw
        state.pid.pitch_output = 0.14 * state.rc.pitch
        state.pid.roll_output = 0.14 * state.rc.roll

        log_second = int(elapsed)
        if log_second != self._last_log_second:
            self._last_log_second = log_second
            state.add_log(
                "SIM %05.1fs motors M1:%04d M2:%04d M3:%04d M4:%04d RC valid:%s"
                % (
                    elapsed,
                    state.motors.m1_front_right,
                    state.motors.m2_back_right,
                    state.motors.m3_back_left,
                    state.motors.m4_front_left,
                    "Y" if state.status.rc_valid else "N",
                )
            )

    @staticmethod
    def _clamp_int(value: float, low: int, high: int) -> int:
        return max(low, min(high, int(round(value))))
