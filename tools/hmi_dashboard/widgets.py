from __future__ import annotations

import math
import tkinter as tk
from tkinter import ttk

from dashboard_state import DashboardState, IMUState, MotorState, RCState, StatusState


BG = "#f3f4f6"
PANEL = "#ffffff"
PANEL_DARK = "#f8fafc"
TEXT = "#1f2937"
MUTED = "#64748b"
GRID = "#cbd5e1"
GREEN = "#22a06b"
AMBER = "#d9a321"
ORANGE = "#e66a2c"
RED = "#d64545"
GREY = "#94a3b8"
BLUE = "#2563eb"


def motor_color(value: int, stale: bool = False) -> str:
    if stale:
        return GREY
    if value <= 0:
        return GREY
    if value <= 300:
        return GREEN
    if value <= 650:
        return AMBER
    if value <= 850:
        return ORANGE
    return RED


def signed_color(value: float, limit: float) -> str:
    ratio = min(1.0, abs(value) / max(1.0, limit))
    if ratio < 0.35:
        return GREEN
    if ratio < 0.7:
        return AMBER
    return ORANGE


class Panel(ttk.Frame):
    def __init__(self, parent: tk.Widget, title: str) -> None:
        super().__init__(parent, style="Panel.TFrame", padding=10)
        self.columnconfigure(0, weight=1)
        ttk.Label(self, text=title, style="PanelTitle.TLabel").grid(row=0, column=0, sticky="ew")
        self.body = ttk.Frame(self, style="Content.TFrame")
        self.body.grid(row=1, column=0, sticky="nsew", pady=(8, 0))
        self.rowconfigure(1, weight=1)


class StatusLight(ttk.Frame):
    def __init__(self, parent: tk.Widget, label: str, good_when: bool = True, panel_style: bool = False) -> None:
        style_name = "Content.TFrame" if panel_style else "Status.TFrame"
        bg = PANEL if panel_style else BG
        super().__init__(parent, style=style_name)
        self._good_when = good_when
        self._canvas = tk.Canvas(self, width=16, height=16, bg=bg, highlightthickness=0)
        self._canvas.grid(row=0, column=0, padx=(0, 5))
        self._dot = self._canvas.create_oval(3, 3, 13, 13, fill=GREY, outline="")
        label_style = "Small.TLabel" if panel_style else "Status.TLabel"
        ttk.Label(self, text=label, style=label_style).grid(row=0, column=1, sticky="w")

    def set(self, active: bool) -> None:
        ok = active == self._good_when
        self._canvas.itemconfigure(self._dot, fill=GREEN if ok else RED)


class ValueBar(ttk.Frame):
    def __init__(
        self,
        parent: tk.Widget,
        label: str,
        minimum: int,
        maximum: int,
        orientation: str,
        width: int = 150,
        height: int = 32,
        centered: bool = False,
        value_format: str = "{:.0f}",
    ) -> None:
        super().__init__(parent, style="Content.TFrame")
        self.minimum = minimum
        self.maximum = maximum
        self.orientation = orientation
        self.centered = centered
        self.value_format = value_format
        self._label = ttk.Label(self, text=label, style="Field.TLabel")
        self._label.grid(row=0, column=0, sticky="ew")
        self._canvas = tk.Canvas(self, width=width, height=height, bg=PANEL_DARK, highlightthickness=1, highlightbackground=GRID)
        self._canvas.grid(row=1, column=0, sticky="nsew", pady=(3, 2))
        self._value = ttk.Label(self, text="0", style="Value.TLabel")
        self._value.grid(row=2, column=0, sticky="ew")
        self._current = 0.0
        self._color = GREEN
        self._stale = False
        self.bind("<Configure>", lambda _event: self._draw())

    def set(self, value: float, color: str | None = None, stale: bool = False) -> None:
        self._current = max(self.minimum, min(self.maximum, value))
        self._color = color or signed_color(self._current, max(abs(self.minimum), abs(self.maximum)))
        self._stale = stale
        self._value.configure(text=self.value_format.format(self._current))
        self._draw()

    def _draw(self) -> None:
        canvas = self._canvas
        canvas.delete("all")
        width = int(canvas.winfo_width() or canvas["width"])
        height = int(canvas.winfo_height() or canvas["height"])
        pad = 5
        canvas.create_rectangle(pad, pad, width - pad, height - pad, outline=GRID, fill=PANEL_DARK)
        fill = GREY if self._stale else self._color
        outline = RED if self._stale else GRID

        if self.orientation == "vertical":
            span = max(1, self.maximum - self.minimum)
            ratio = (self._current - self.minimum) / span
            top = height - pad - ratio * (height - 2 * pad)
            canvas.create_rectangle(pad + 1, top, width - pad - 1, height - pad - 1, outline="", fill=fill)
        elif self.orientation == "vertical_center":
            zero = height / 2
            canvas.create_line(pad, zero, width - pad, zero, fill=MUTED)
            limit = max(abs(self.minimum), abs(self.maximum), 1)
            delta = (self._current / limit) * ((height - 2 * pad) / 2)
            y0, y1 = sorted((zero, zero - delta))
            canvas.create_rectangle(pad + 1, y0, width - pad - 1, y1, outline="", fill=fill)
        elif self.orientation == "horizontal_center":
            zero = width / 2
            canvas.create_line(zero, pad, zero, height - pad, fill=MUTED)
            limit = max(abs(self.minimum), abs(self.maximum), 1)
            delta = (self._current / limit) * ((width - 2 * pad) / 2)
            x0, x1 = sorted((zero, zero + delta))
            canvas.create_rectangle(x0, pad + 1, x1, height - pad - 1, outline="", fill=fill)
        else:
            span = max(1, self.maximum - self.minimum)
            ratio = (self._current - self.minimum) / span
            canvas.create_rectangle(pad + 1, pad + 1, pad + ratio * (width - 2 * pad), height - pad - 1, outline="", fill=fill)

        if self._stale:
            canvas.create_rectangle(1, 1, width - 2, height - 2, outline=outline, width=2)


class MotorMap(Panel):
    def __init__(self, parent: tk.Widget) -> None:
        super().__init__(parent, "MOTOR OUTPUTS")
        body = self.body
        for col in range(3):
            body.columnconfigure(col, weight=1, uniform="motor")
        for row in range(5):
            body.rowconfigure(row, weight=1)

        ttk.Label(body, text="FRONT", style="Axis.TLabel").grid(row=0, column=0, columnspan=3, sticky="n")
        ttk.Label(body, text="BACK", style="Axis.TLabel").grid(row=4, column=0, columnspan=3, sticky="s")
        self.m4 = self._motor_bar(body, "M4", "Front Left", 1, 0)
        self.m1 = self._motor_bar(body, "M1", "Front Right", 1, 2)
        self.m3 = self._motor_bar(body, "M3", "Back Left", 3, 0)
        self.m2 = self._motor_bar(body, "M2", "Back Right", 3, 2)

    def _motor_bar(self, parent: tk.Widget, name: str, location: str, row: int, col: int) -> ValueBar:
        frame = ttk.Frame(parent, style="Content.TFrame")
        frame.grid(row=row, column=col, sticky="nsew", padx=8, pady=4)
        frame.columnconfigure(0, weight=1)
        ttk.Label(frame, text=name, style="MotorName.TLabel").grid(row=0, column=0, sticky="ew")
        ttk.Label(frame, text=location, style="Small.TLabel").grid(row=1, column=0, sticky="ew")
        bar = ValueBar(frame, "", 0, 1000, "vertical", width=58, height=130)
        bar.grid(row=2, column=0, sticky="n")
        return bar

    def update_state(self, motors: MotorState) -> None:
        self.m1.set(motors.m1_front_right, motor_color(motors.m1_front_right, motors.stale), motors.stale)
        self.m2.set(motors.m2_back_right, motor_color(motors.m2_back_right, motors.stale), motors.stale)
        self.m3.set(motors.m3_back_left, motor_color(motors.m3_back_left, motors.stale), motors.stale)
        self.m4.set(motors.m4_front_left, motor_color(motors.m4_front_left, motors.stale), motors.stale)


class RCPanel(Panel):
    def __init__(self, parent: tk.Widget) -> None:
        super().__init__(parent, "RC INPUTS")
        body = self.body
        body.columnconfigure(0, weight=0)
        body.columnconfigure(1, weight=0)
        body.columnconfigure(2, weight=1)
        self.throttle = ValueBar(body, "THROTTLE", 0, 1000, "vertical", width=72, height=145)
        self.pitch = ValueBar(body, "PITCH", -500, 500, "vertical_center", width=72, height=145)
        self.throttle.grid(row=0, column=0, rowspan=3, sticky="n", padx=(4, 18))
        self.pitch.grid(row=0, column=1, rowspan=3, sticky="n", padx=(0, 24))

        right = ttk.Frame(body, style="Content.TFrame")
        right.grid(row=0, column=2, sticky="nsew")
        right.columnconfigure(0, weight=1)
        self.yaw = ValueBar(right, "YAW", -500, 500, "horizontal_center", width=320, height=34)
        self.roll = ValueBar(right, "ROLL", -500, 500, "horizontal_center", width=320, height=34)
        self.yaw.grid(row=0, column=0, sticky="ew", pady=(0, 8))
        self.roll.grid(row=1, column=0, sticky="ew", pady=(0, 10))

        row = ttk.Frame(right, style="Content.TFrame")
        row.grid(row=2, column=0, sticky="ew", pady=(2, 0))
        row.columnconfigure(1, weight=1)
        row.columnconfigure(2, weight=0)
        self.estop = StatusLight(row, "E-STOP SAFE", good_when=True, panel_style=True)
        self.estop.grid(row=0, column=0, sticky="w", padx=(0, 16))
        self.ch6 = ttk.Label(row, text="CH6 0", style="Tile.TLabel", anchor="center")
        self.ch6.grid(row=0, column=1, sticky="ew")

        valid_row = ttk.Frame(row, style="Content.TFrame")
        valid_row.grid(row=0, column=2, sticky="e", padx=(14, 0))
        ttk.Label(valid_row, text="VALID", style="Small.TLabel").grid(row=0, column=0, padx=(0, 6))
        self.channel_lights: list[StatusLight] = []
        for index in range(6):
            light = StatusLight(valid_row, str(index + 1), panel_style=True)
            light.grid(row=0, column=index + 1, padx=2)
            self.channel_lights.append(light)

    def update_state(self, rc: RCState) -> None:
        self.throttle.set(rc.throttle, motor_color(rc.throttle, rc.stale), rc.stale)
        self.pitch.set(rc.pitch, stale=rc.stale)
        self.yaw.set(rc.yaw, stale=rc.stale)
        self.roll.set(rc.roll, stale=rc.stale)
        self.estop.set(rc.estop_safe and not rc.stale)
        self.ch6.configure(text="CH6 %d" % rc.channel_6)
        for light, valid in zip(self.channel_lights, rc.channel_valid):
            light.set(valid and not rc.stale)


class Compass(tk.Canvas):
    def __init__(self, parent: tk.Widget) -> None:
        super().__init__(parent, width=145, height=145, bg=PANEL_DARK, highlightthickness=1, highlightbackground=GRID)
        self._heading = 0.0

    def set_heading(self, heading: float) -> None:
        self._heading = heading % 360.0
        self._draw()

    def _draw(self) -> None:
        self.delete("all")
        width = int(self.winfo_width() or self["width"])
        height = int(self.winfo_height() or self["height"])
        cx, cy = width / 2, height / 2
        radius = min(width, height) / 2 - 14
        self.create_oval(cx - radius, cy - radius, cx + radius, cy + radius, outline=GRID, width=2)
        for label, angle in (("N", -90), ("E", 0), ("S", 90), ("W", 180)):
            rad = math.radians(angle)
            self.create_text(cx + math.cos(rad) * (radius - 16), cy + math.sin(rad) * (radius - 16), text=label, fill=MUTED, font=("Segoe UI", 10, "bold"))
        pointer_angle = math.radians(self._heading - 90)
        tip = (cx + math.cos(pointer_angle) * (radius - 8), cy + math.sin(pointer_angle) * (radius - 8))
        left = (cx + math.cos(pointer_angle + 2.5) * 13, cy + math.sin(pointer_angle + 2.5) * 13)
        right = (cx + math.cos(pointer_angle - 2.5) * 13, cy + math.sin(pointer_angle - 2.5) * 13)
        self.create_polygon(tip, left, right, fill=BLUE, outline="")
        self.create_oval(cx - 5, cy - 5, cx + 5, cy + 5, fill=TEXT, outline="")
        self.create_text(cx, height - 16, text="YAW %.0f deg" % self._heading, fill=TEXT, font=("Segoe UI", 10, "bold"))


class IMUPanel(Panel):
    def __init__(self, parent: tk.Widget) -> None:
        super().__init__(parent, "IMU / ATTITUDE")
        body = self.body
        body.columnconfigure(0, weight=1)
        body.columnconfigure(1, weight=0)
        self.roll = ValueBar(body, "ROLL", -45, 45, "horizontal_center", width=280, height=34, value_format="{:.1f} deg")
        self.roll.grid(row=0, column=0, columnspan=2, sticky="ew")
        self.compass = Compass(body)
        self.compass.grid(row=1, column=0, sticky="nsew", padx=(0, 10), pady=8)
        self.pitch = ValueBar(body, "PITCH", -45, 45, "vertical_center", width=72, height=135, value_format="{:.1f} deg")
        self.pitch.grid(row=1, column=1, sticky="n")
        self.rates = ttk.Label(body, text="", style="Small.TLabel")
        self.rates.grid(row=2, column=0, columnspan=2, sticky="ew", pady=(2, 8))
        health = ttk.Frame(body, style="Content.TFrame")
        health.grid(row=3, column=0, columnspan=2, sticky="ew")
        self.ready = StatusLight(health, "READY", panel_style=True)
        self.fusion = StatusLight(health, "FUSION", panel_style=True)
        self.error = StatusLight(health, "ERROR", good_when=False, panel_style=True)
        self.ready.grid(row=0, column=0, padx=(0, 8))
        self.fusion.grid(row=0, column=1, padx=8)
        self.error.grid(row=0, column=2, padx=8)
        self.cal = ttk.Label(health, text="CAL S/G/M/A 0/0/0/0", style="Small.TLabel")
        self.cal.grid(row=0, column=3, padx=(8, 0))

    def update_state(self, imu: IMUState) -> None:
        self.roll.set(imu.roll_deg, signed_color(imu.roll_deg, 45), imu.stale)
        self.pitch.set(imu.pitch_deg, signed_color(imu.pitch_deg, 45), imu.stale)
        self.compass.set_heading(imu.yaw_deg)
        self.rates.configure(text="Rates dps  Roll % 6.1f   Pitch % 6.1f   Yaw % 6.1f" % (imu.roll_rate_dps, imu.pitch_rate_dps, imu.yaw_rate_dps))
        self.ready.set(imu.ready and not imu.stale)
        self.fusion.set(imu.fusion and not imu.stale)
        self.error.set(imu.error)
        self.cal.configure(text="CAL S/G/M/A %d/%d/%d/%d" % (imu.cal_sys, imu.cal_gyro, imu.cal_mag, imu.cal_accel))


class StatusStrip(ttk.Frame):
    def __init__(self, parent: tk.Widget) -> None:
        super().__init__(parent, style="Status.TFrame", padding=(10, 6))
        self.connected = StatusLight(self, "CONNECTED")
        self.telemetry = StatusLight(self, "TELEMETRY")
        self.imu = StatusLight(self, "IMU READY")
        self.rc = StatusLight(self, "RC VALID")
        self.armed = StatusLight(self, "ARMED")
        self.error = StatusLight(self, "ERROR", good_when=False)
        for col, light in enumerate((self.connected, self.telemetry, self.imu, self.rc, self.armed, self.error)):
            light.grid(row=0, column=col, padx=(0, 18), sticky="w")

    def update_state(self, status: StatusState) -> None:
        self.connected.set(status.connected)
        self.telemetry.set(status.telemetry_active)
        self.imu.set(status.imu_ready)
        self.rc.set(status.rc_valid)
        self.armed.set(status.armed)
        self.error.set(status.error)


class FlightStatusPanel(Panel):
    def __init__(self, parent: tk.Widget) -> None:
        super().__init__(parent, "FLIGHT STATUS")
        self.labels: dict[str, ttk.Label] = {}
        for row, label in enumerate(("App Mode", "Flight Mode", "Run Mode", "Loop Age")):
            ttk.Label(self.body, text=label.upper(), style="Small.TLabel").grid(row=row, column=0, sticky="w", pady=3)
            value = ttk.Label(self.body, text="", style="Value.TLabel")
            value.grid(row=row, column=1, sticky="e", pady=3)
            self.labels[label] = value
        self.body.columnconfigure(1, weight=1)

    def update_state(self, status: StatusState) -> None:
        self.labels["App Mode"].configure(text=status.app_mode)
        self.labels["Flight Mode"].configure(text=status.flight_mode)
        self.labels["Run Mode"].configure(text=status.run_mode)
        self.labels["Loop Age"].configure(text="%d ms" % status.loop_age_ms)


class PIDPanel(Panel):
    def __init__(self, parent: tk.Widget) -> None:
        super().__init__(parent, "PID / CONTROL SUMMARY")
        self.text = ttk.Label(self.body, text="", style="Value.TLabel")
        self.text.grid(row=0, column=0, sticky="ew")

    def update_state(self, state: DashboardState) -> None:
        pid = state.pid
        self.text.configure(
            text="Setpoint Y/P/R: % 5.1f / % 5.1f / % 5.1f     PID Out Y/P/R: % 6.1f / % 6.1f / % 6.1f"
            % (pid.yaw_setpoint, pid.pitch_setpoint, pid.roll_setpoint, pid.yaw_output, pid.pitch_output, pid.roll_output)
        )


def configure_styles(root: tk.Tk) -> None:
    style = ttk.Style(root)
    style.theme_use("clam")
    root.configure(bg=BG)
    style.configure(".", background=BG, foreground=TEXT, font=("Segoe UI", 10))
    style.configure("Top.TFrame", background=BG)
    style.configure("Status.TFrame", background=BG)
    style.configure("Panel.TFrame", background=PANEL, borderwidth=1, relief="solid")
    style.configure("Content.TFrame", background=PANEL, borderwidth=0, relief="flat")
    style.configure("TLabel", background=BG, foreground=TEXT)
    style.configure("Title.TLabel", background=BG, foreground=TEXT, font=("Segoe UI", 15, "bold"))
    style.configure("Status.TLabel", background=BG, foreground=TEXT, font=("Segoe UI", 9, "bold"))
    style.configure("PanelTitle.TLabel", background=PANEL, foreground=TEXT, font=("Segoe UI", 11, "bold"))
    style.configure("Field.TLabel", background=PANEL, foreground=MUTED, font=("Segoe UI", 8, "bold"))
    style.configure("Small.TLabel", background=PANEL, foreground=MUTED, font=("Segoe UI", 9))
    style.configure("Value.TLabel", background=PANEL, foreground=TEXT, font=("Segoe UI", 10, "bold"))
    style.configure("Tile.TLabel", background=PANEL_DARK, foreground=TEXT, font=("Segoe UI", 10, "bold"), padding=(8, 5))
    style.configure("Axis.TLabel", background=PANEL, foreground=MUTED, font=("Segoe UI", 9, "bold"))
    style.configure("MotorName.TLabel", background=PANEL, foreground=TEXT, font=("Segoe UI", 12, "bold"), anchor="center")
    style.configure("TButton", background="#e5e7eb", foreground=TEXT)
    style.configure("TCombobox", fieldbackground=PANEL_DARK, background=PANEL_DARK, foreground=TEXT)
