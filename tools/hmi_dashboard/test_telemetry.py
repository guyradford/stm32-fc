from __future__ import annotations

import unittest

from dashboard_state import DashboardState, IMUState, MotorState, RCState, StatusState
from serial_client import TelemetrySerialClient
from telemetry_mapper import apply_frame, mark_stale, reset_live_state
from telemetry_protocol import TelemetryError, format_sentence, format_stop, parse_sentence
from widgets import (
    HEALTH_BAD,
    HEALTH_NORMAL,
    HEALTH_WARN,
    flight_panel_health,
    imu_panel_health,
    motor_panel_health,
    rc_panel_health,
)


LIVE_RC_LINE = "$RC,239514,0,500,500,500,0,500,0*29"
LIVE_IMU_LINE = "$IMU,239514,0,0,0,0,0,0,0*69"
LIVE_IMUC_LINE = format_sentence("IMUC,239514,3,3,2,1,0")
LIVE_MOT_LINE = "$MOT,239514,1001,1002,1003,1004*76"
DEMAND_MOT_LINE = format_sentence("MOT,239514,1,2,3,4")


class TelemetryProtocolTests(unittest.TestCase):
    def test_format_sentence_adds_checksum(self) -> None:
        self.assertEqual("$RC,1*0C\r\n", format_sentence("RC,1"))

    def test_stop_command_formats_with_checksum(self) -> None:
        self.assertEqual("$STOP*18\r\n", format_stop())

    def test_parse_valid_sentence(self) -> None:
        frame = parse_sentence(format_sentence("MOT,1000,10,20,30,40"))

        self.assertEqual("MOT", frame.subject)
        self.assertEqual(("1000", "10", "20", "30", "40"), frame.fields)

    def test_bad_checksum_rejected(self) -> None:
        with self.assertRaises(TelemetryError):
            parse_sentence("$MOT,1000,10,20,30,40*00")

    def test_expected_field_count_rejected(self) -> None:
        with self.assertRaises(TelemetryError):
            parse_sentence(format_sentence("RC,1000,1"))

    def test_stat_sentence_is_first_class(self) -> None:
        frame = parse_sentence(format_sentence("STAT,1000,RUN,AUTO,ARMED,1,1,1,0,0,12"))

        self.assertEqual("STAT", frame.subject)

    def test_parses_live_fc_sample_lines(self) -> None:
        self.assertEqual("RC", parse_sentence(LIVE_RC_LINE).subject)
        self.assertEqual("IMU", parse_sentence(LIVE_IMU_LINE).subject)
        self.assertEqual("IMUC", parse_sentence(LIVE_IMUC_LINE).subject)
        self.assertEqual("MOT", parse_sentence(LIVE_MOT_LINE).subject)


class TelemetryMapperTests(unittest.TestCase):
    def test_rc_mapping_centers_axes(self) -> None:
        state = DashboardState()
        reset_live_state(state)

        apply_frame(state, parse_sentence(format_sentence("RC,1000,250,510,490,750,1000,333,1")), 10.0)

        self.assertEqual(250, state.rc.throttle)
        self.assertEqual(10, state.rc.yaw)
        self.assertEqual(10, state.rc.pitch)
        self.assertEqual(-250, state.rc.roll)
        self.assertTrue(state.rc.estop_safe)
        self.assertEqual(333, state.rc.channel_6)
        self.assertEqual([True] * 6, state.rc.channel_valid)
        self.assertTrue(state.status.rc_valid)

    def test_imu_mapping_converts_centidegrees(self) -> None:
        state = DashboardState()
        reset_live_state(state)

        apply_frame(state, parse_sentence(format_sentence("IMU,1000,1234,-567,9012,345,-678,90,1")), 10.0)

        self.assertEqual(12.34, state.imu.roll_deg)
        self.assertEqual(-5.67, state.imu.pitch_deg)
        self.assertEqual(90.12, state.imu.yaw_deg)
        self.assertEqual(3.45, state.imu.roll_rate_dps)
        self.assertEqual(-6.78, state.imu.pitch_rate_dps)
        self.assertEqual(0.90, state.imu.yaw_rate_dps)
        self.assertTrue(state.status.imu_ready)

    def test_imuc_mapping_updates_calibration_status(self) -> None:
        state = DashboardState()
        reset_live_state(state)

        apply_frame(state, parse_sentence(format_sentence("IMUC,1000,3,2,1,0,1")), 10.0)

        self.assertEqual(3, state.imu.cal_sys)
        self.assertEqual(2, state.imu.cal_gyro)
        self.assertEqual(1, state.imu.cal_mag)
        self.assertEqual(0, state.imu.cal_accel)
        self.assertTrue(state.status.imu_ready)

    def test_mot_mapping_preserves_native_units(self) -> None:
        state = DashboardState()
        reset_live_state(state)

        apply_frame(state, parse_sentence(format_sentence("MOT,1000,1,2,3,4")), 10.0)

        self.assertEqual(1, state.motors.m1_front_right)
        self.assertEqual(2, state.motors.m2_back_right)
        self.assertEqual(3, state.motors.m3_back_left)
        self.assertEqual(4, state.motors.m4_front_left)

    def test_stat_mapping_owns_status_panel_fields(self) -> None:
        state = DashboardState()
        reset_live_state(state)

        apply_frame(state, parse_sentence(format_sentence("STAT,1000,RUN,AUTO,ARMED,1,1,1,0,0,12")), 10.0)

        self.assertEqual("RUN", state.status.app_mode)
        self.assertEqual("AUTO", state.status.flight_mode)
        self.assertEqual("ARMED", state.status.run_mode)
        self.assertTrue(state.status.armed)
        self.assertTrue(state.status.rc_valid)
        self.assertTrue(state.status.imu_ready)
        self.assertFalse(state.status.error)
        self.assertEqual(12, state.status.loop_age_ms)

    def test_stale_keeps_values_but_marks_panels(self) -> None:
        state = DashboardState()
        reset_live_state(state)
        apply_frame(state, parse_sentence(format_sentence("MOT,1000,1,2,3,4")), 10.0)

        mark_stale(state, 12.0)

        self.assertEqual(1, state.motors.m1_front_right)
        self.assertTrue(state.motors.stale)
        self.assertFalse(state.status.telemetry_active)


class PanelHealthTests(unittest.TestCase):
    def test_rc_panel_health_marks_stale_invalid_or_estop_unsafe_bad(self) -> None:
        self.assertEqual(HEALTH_BAD, rc_panel_health(RCState(stale=True)))
        self.assertEqual(HEALTH_BAD, rc_panel_health(RCState(channel_valid=[True, True, False, True, True, True])))
        self.assertEqual(HEALTH_BAD, rc_panel_health(RCState(estop_safe=False)))
        self.assertEqual(HEALTH_NORMAL, rc_panel_health(RCState()))

    def test_imu_panel_health_marks_stale_error_calibrating_and_ready(self) -> None:
        ready = IMUState(ready=True, fusion=True, cal_sys=3, cal_gyro=3, cal_mag=3, cal_accel=3)

        self.assertEqual(HEALTH_BAD, imu_panel_health(IMUState(stale=True)))
        self.assertEqual(HEALTH_BAD, imu_panel_health(IMUState(error=True)))
        self.assertEqual(HEALTH_WARN, imu_panel_health(IMUState(ready=True, fusion=True, cal_sys=3, cal_gyro=3, cal_mag=2, cal_accel=3)))
        self.assertEqual(HEALTH_NORMAL, imu_panel_health(ready))

    def test_motor_panel_health_marks_stale_bad(self) -> None:
        self.assertEqual(HEALTH_BAD, motor_panel_health(MotorState(stale=True)))
        self.assertEqual(HEALTH_NORMAL, motor_panel_health(MotorState()))

    def test_flight_panel_health_marks_bad_disarmed_and_healthy(self) -> None:
        healthy = StatusState(connected=True, telemetry_active=True, armed=True, run_mode="ARMED")

        self.assertEqual(HEALTH_BAD, flight_panel_health(StatusState(connected=False, telemetry_active=False)))
        self.assertEqual(HEALTH_BAD, flight_panel_health(StatusState(connected=True, telemetry_active=True, error=True)))
        self.assertEqual(HEALTH_BAD, flight_panel_health(StatusState(connected=True, telemetry_active=True, run_mode="FAILSAFE")))
        self.assertEqual(HEALTH_WARN, flight_panel_health(StatusState(connected=True, telemetry_active=True, armed=False, run_mode="DISARMED")))
        self.assertEqual(HEALTH_NORMAL, flight_panel_health(healthy))


class FakeSerial:
    def __init__(self) -> None:
        self.is_open = True
        self.writes: list[bytes] = []

    def write(self, data: bytes) -> int:
        self.writes.append(data)
        return len(data)

    def flush(self) -> None:
        pass


class TelemetrySerialClientTests(unittest.TestCase):
    def test_handshake_does_not_send_home_when_telemetry_already_seen(self) -> None:
        client = TelemetrySerialClient()
        fake_serial = FakeSerial()
        client._serial = fake_serial  # noqa: SLF001 - intentional white-box regression test

        client._process_line(LIVE_RC_LINE.encode("ascii"))  # noqa: SLF001
        client._stop_requested.set()  # noqa: SLF001
        client._handshake_loop()  # noqa: SLF001

        self.assertEqual([], fake_serial.writes)

    def test_menu_detection_sends_telemetry_request(self) -> None:
        client = TelemetrySerialClient()
        fake_serial = FakeSerial()
        client._serial = fake_serial  # noqa: SLF001

        client._process_line(b"n - Telemetry Mode.")  # noqa: SLF001

        self.assertEqual([b"n"], fake_serial.writes)

    def test_live_samples_map_to_visible_dashboard_values(self) -> None:
        state = DashboardState()
        reset_live_state(state)

        apply_frame(state, parse_sentence(LIVE_RC_LINE), 10.0)
        apply_frame(state, parse_sentence(LIVE_IMU_LINE), 10.0)
        apply_frame(state, parse_sentence(LIVE_IMUC_LINE), 10.0)
        apply_frame(state, parse_sentence(DEMAND_MOT_LINE), 10.0)

        self.assertEqual(0, state.rc.throttle)
        self.assertEqual(0, state.rc.pitch)
        self.assertFalse(state.status.rc_valid)
        self.assertFalse(state.status.imu_ready)
        self.assertEqual(3, state.imu.cal_sys)
        self.assertEqual(1, state.imu.cal_accel)
        self.assertEqual(1, state.motors.m1_front_right)
        self.assertEqual(4, state.motors.m4_front_left)
        self.assertTrue(state.status.telemetry_active)


if __name__ == "__main__":
    unittest.main()
