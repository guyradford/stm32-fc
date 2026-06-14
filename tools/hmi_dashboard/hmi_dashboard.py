from __future__ import annotations

import queue
import tkinter as tk
from tkinter import messagebox, scrolledtext, ttk

from dashboard_state import DashboardState
from serial_client import TelemetrySerialClient, list_serial_ports
from simulator import DashboardSimulator
from telemetry_mapper import apply_frame, mark_stale, reset_live_state
from widgets import (
    BG,
    FlightStatusPanel,
    IMUPanel,
    MotorMap,
    PANEL_DARK,
    PIDPanel,
    RCPanel,
    StatusStrip,
    TEXT,
    configure_styles,
)


class HMIDashboardApp:
    UPDATE_MS = 100

    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.state = DashboardState()
        self.simulator = DashboardSimulator()
        self.serial_client = TelemetrySerialClient()
        self.mode = tk.StringVar(value="Live")
        self.port = tk.StringVar()
        self.baud = tk.StringVar(value="115200")
        self.connect_text = tk.StringVar(value="Connect")
        configure_styles(root)

        reset_live_state(self.state)
        root.title("STM32-FC HMI Dashboard")
        root.minsize(1120, 940)
        root.columnconfigure(0, weight=1)
        root.rowconfigure(2, weight=1)

        self._build_top_bar()
        self.status_strip = StatusStrip(root)
        self.status_strip.grid(row=1, column=0, sticky="ew")
        self._build_dashboard()
        self._build_log()
        self._refresh_ports()
        self._schedule_update()

    def _build_top_bar(self) -> None:
        top = ttk.Frame(self.root, style="Top.TFrame", padding=(12, 10))
        top.grid(row=0, column=0, sticky="ew")
        top.columnconfigure(1, weight=1)

        ttk.Label(top, text="STM32-FC HMI Dashboard", style="Title.TLabel").grid(row=0, column=0, sticky="w")
        controls = ttk.Frame(top, style="Top.TFrame")
        controls.grid(row=0, column=2, sticky="e")

        ttk.Label(controls, text="Port").grid(row=0, column=0, padx=(0, 4))
        self.port_combo = ttk.Combobox(controls, textvariable=self.port, width=16, state="readonly")
        self.port_combo.grid(row=0, column=1, padx=(0, 8))
        ttk.Label(controls, text="Baud").grid(row=0, column=2, padx=(0, 4))
        self.baud_combo = ttk.Combobox(controls, textvariable=self.baud, values=("115200", "57600"), width=10, state="readonly")
        self.baud_combo.grid(row=0, column=3, padx=(0, 8))
        ttk.Label(controls, text="Mode").grid(row=0, column=4, padx=(0, 4))
        self.mode_combo = ttk.Combobox(controls, textvariable=self.mode, values=("Live", "Simulator"), width=10, state="readonly")
        self.mode_combo.grid(row=0, column=5, padx=(0, 8))
        self.mode_combo.bind("<<ComboboxSelected>>", lambda _event: self._on_mode_changed())
        ttk.Button(controls, text="Refresh", command=self._refresh_ports).grid(row=0, column=6, padx=(0, 8))
        self.connect_button = ttk.Button(controls, textvariable=self.connect_text, command=self._toggle_connection)
        self.connect_button.grid(row=0, column=7)

    def _build_dashboard(self) -> None:
        main = ttk.Frame(self.root, style="Top.TFrame", padding=10)
        main.grid(row=2, column=0, sticky="nsew")
        main.columnconfigure(0, weight=1)
        main.columnconfigure(1, weight=2)
        main.columnconfigure(2, weight=2)
        main.rowconfigure(0, weight=3, minsize=310)
        main.rowconfigure(1, weight=2, minsize=270)

        self.flight = FlightStatusPanel(main)
        self.flight.grid(row=0, column=0, sticky="nsew", padx=(0, 8), pady=(0, 8))
        self.imu = IMUPanel(main)
        self.imu.grid(row=0, column=1, sticky="nsew", padx=8, pady=(0, 8))
        self.motors = MotorMap(main)
        self.motors.grid(row=0, column=2, rowspan=2, sticky="nsew", padx=(8, 0), pady=(0, 8))
        self.rc = RCPanel(main)
        self.rc.grid(row=1, column=0, columnspan=2, sticky="nsew", padx=(0, 8), pady=(8, 8))
        self.pid = PIDPanel(main)
        self.pid.grid(row=2, column=0, columnspan=3, sticky="ew", pady=(0, 0))

    def _build_log(self) -> None:
        frame = ttk.Frame(self.root, style="Panel.TFrame", padding=10)
        frame.grid(row=3, column=0, sticky="ew", padx=10, pady=(0, 10))
        frame.columnconfigure(0, weight=1)
        ttk.Label(frame, text="EVENT LOG", style="PanelTitle.TLabel").grid(row=0, column=0, sticky="w")
        self.log = scrolledtext.ScrolledText(
            frame,
            height=3,
            bg=PANEL_DARK,
            fg=TEXT,
            insertbackground=TEXT,
            relief="solid",
            borderwidth=1,
            font=("Consolas", 9),
            wrap="none",
        )
        self.log.grid(row=1, column=0, sticky="ew", pady=(8, 0))
        self.log.configure(state="disabled")

    def _schedule_update(self) -> None:
        self._drain_serial_events()
        if self.mode.get() == "Simulator":
            self.simulator.update(self.state)
        else:
            self.state.status.connected = self.serial_client.connected
            mark_stale(self.state)
        self._apply_state()
        self.root.after(self.UPDATE_MS, self._schedule_update)

    def _apply_state(self) -> None:
        self.status_strip.update_state(self.state.status)
        self.flight.update_state(self.state.status)
        self.imu.update_state(self.state.imu)
        self.motors.update_state(self.state.motors)
        self.rc.update_state(self.state.rc)
        self.pid.update_state(self.state)
        self._sync_log()

    def _sync_log(self) -> None:
        content = "\n".join(self.state.log_lines[-80:])
        self.log.configure(state="normal")
        self.log.delete("1.0", tk.END)
        self.log.insert(tk.END, content)
        self.log.see(tk.END)
        self.log.configure(state="disabled")

    def _refresh_ports(self) -> None:
        ports = list_serial_ports()
        self.port_combo.configure(values=ports)
        if ports and self.port.get() not in ports:
            self.port.set(ports[0])

    def _on_mode_changed(self) -> None:
        if self.mode.get() == "Simulator":
            if self.serial_client.connected:
                self._disconnect()
            self.state.add_log("Simulator mode selected")
        else:
            reset_live_state(self.state)
            self.state.add_log("Live mode selected")

    def _toggle_connection(self) -> None:
        if self.serial_client.connected:
            self._disconnect()
            return
        self._connect()

    def _connect(self) -> None:
        if self.mode.get() != "Live":
            self.mode.set("Live")
            reset_live_state(self.state)
        port = self.port.get().strip()
        if not port:
            messagebox.showerror("Serial Port", "Select a serial port before connecting.")
            return
        try:
            baud = int(self.baud.get())
            self.serial_client.connect(port, baud)
        except Exception as exc:
            messagebox.showerror("Serial Connect", str(exc))
            self.state.add_log("CONNECT ERROR %s" % exc)
            return

        self.state.status.connected = True
        self.connect_text.set("Disconnect")

    def _disconnect(self) -> None:
        try:
            self.serial_client.disconnect()
        except Exception as exc:
            self.state.add_log("DISCONNECT ERROR %s" % exc)
        reset_live_state(self.state)
        self.connect_text.set("Connect")

    def _drain_serial_events(self) -> None:
        while True:
            try:
                event = self.serial_client.events.get_nowait()
            except queue.Empty:
                break

            if event.kind == "frame" and event.frame is not None:
                if event.frame.subject in ("RC", "IMU", "MOT", "STAT"):
                    try:
                        apply_frame(self.state, event.frame)
                    except ValueError as exc:
                        self.state.add_log("MAP ERROR %s: %s" % (event.frame.subject, exc))
                elif event.frame.subject in ("ACK", "ERR"):
                    self.state.add_log(event.frame.payload)
                else:
                    self.state.add_log("UNSUPPORTED %s" % event.frame.payload)
            elif event.kind == "closed":
                self.state.status.connected = self.serial_client.connected
                if not self.serial_client.connected:
                    self.connect_text.set("Connect")
                self.state.add_log(event.message)
            else:
                self.state.add_log(event.message)


def main() -> None:
    root = tk.Tk()
    app = HMIDashboardApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
