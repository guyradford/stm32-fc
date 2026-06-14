from __future__ import annotations

import unittest

from dashboard_state import DashboardState
from telemetry_mapper import apply_frame, mark_stale, reset_live_state
from telemetry_protocol import TelemetryError, format_sentence, format_stop, parse_sentence


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


class TelemetryMapperTests(unittest.TestCase):
    def test_rc_mapping_centers_axes(self) -> None:
        state = DashboardState()
        reset_live_state(state)

        apply_frame(state, parse_sentence(format_sentence("RC,1000,250,510,490,750,1000,333,1")), 10.0)

        self.assertEqual(250, state.rc.throttle)
        self.assertEqual(10, state.rc.yaw)
        self.assertEqual(-10, state.rc.pitch)
        self.assertEqual(250, state.rc.roll)
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


if __name__ == "__main__":
    unittest.main()
