from __future__ import annotations

from dataclasses import dataclass, field
from typing import List


@dataclass
class StatusState:
    connected: bool = False
    telemetry_active: bool = False
    imu_ready: bool = False
    rc_valid: bool = False
    armed: bool = False
    error: bool = False
    app_mode: str = "SIM"
    flight_mode: str = "AUTO_LEVEL"
    run_mode: str = "DISARMED"
    loop_age_ms: int = 0


@dataclass
class RCState:
    throttle: int = 0
    yaw: int = 0
    pitch: int = 0
    roll: int = 0
    estop_safe: bool = True
    channel_6: int = 0
    channel_valid: List[bool] = field(default_factory=lambda: [True] * 6)
    stale: bool = False


@dataclass
class MotorState:
    m1_front_right: int = 0
    m2_back_right: int = 0
    m3_back_left: int = 0
    m4_front_left: int = 0
    stale: bool = False


@dataclass
class IMUState:
    roll_deg: float = 0.0
    pitch_deg: float = 0.0
    yaw_deg: float = 0.0
    roll_rate_dps: float = 0.0
    pitch_rate_dps: float = 0.0
    yaw_rate_dps: float = 0.0
    ready: bool = False
    fusion: bool = False
    error: bool = False
    cal_sys: int = 0
    cal_gyro: int = 0
    cal_mag: int = 0
    cal_accel: int = 0
    stale: bool = False


@dataclass
class PIDState:
    yaw_setpoint: float = 0.0
    pitch_setpoint: float = 0.0
    roll_setpoint: float = 0.0
    yaw_output: float = 0.0
    pitch_output: float = 0.0
    roll_output: float = 0.0


@dataclass
class DashboardState:
    status: StatusState = field(default_factory=StatusState)
    rc: RCState = field(default_factory=RCState)
    motors: MotorState = field(default_factory=MotorState)
    imu: IMUState = field(default_factory=IMUState)
    pid: PIDState = field(default_factory=PIDState)
    log_lines: List[str] = field(default_factory=list)
    last_rx_s: float | None = None
    last_rc_s: float | None = None
    last_imu_s: float | None = None
    last_mot_s: float | None = None
    last_stat_s: float | None = None

    def add_log(self, line: str, limit: int = 200) -> None:
        self.log_lines.append(line)
        if len(self.log_lines) > limit:
            del self.log_lines[: len(self.log_lines) - limit]
