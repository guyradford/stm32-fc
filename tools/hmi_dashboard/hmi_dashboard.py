from __future__ import annotations

import tkinter as tk
from tkinter import scrolledtext, ttk

from dashboard_state import DashboardState
from simulator import DashboardSimulator
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
        configure_styles(root)

        root.title("STM32-FC HMI Dashboard - Phase 1 Visual Prototype")
        root.minsize(1120, 940)
        root.columnconfigure(0, weight=1)
        root.rowconfigure(2, weight=1)

        self._build_top_bar()
        self.status_strip = StatusStrip(root)
        self.status_strip.grid(row=1, column=0, sticky="ew")
        self._build_dashboard()
        self._build_log()
        self._schedule_update()

    def _build_top_bar(self) -> None:
        top = ttk.Frame(self.root, style="Top.TFrame", padding=(12, 10))
        top.grid(row=0, column=0, sticky="ew")
        top.columnconfigure(1, weight=1)

        ttk.Label(top, text="STM32-FC HMI Dashboard", style="Title.TLabel").grid(row=0, column=0, sticky="w")
        controls = ttk.Frame(top, style="Top.TFrame")
        controls.grid(row=0, column=2, sticky="e")

        ttk.Label(controls, text="Port").grid(row=0, column=0, padx=(0, 4))
        port = ttk.Combobox(controls, values=("COM3", "COM4", "USART1 Wireless"), width=16, state="disabled")
        port.set("Phase 2")
        port.grid(row=0, column=1, padx=(0, 8))
        ttk.Label(controls, text="Baud").grid(row=0, column=2, padx=(0, 4))
        baud = ttk.Combobox(controls, values=("115200", "57600"), width=10, state="disabled")
        baud.set("Simulated")
        baud.grid(row=0, column=3, padx=(0, 8))
        ttk.Button(controls, text="Connect", state="disabled").grid(row=0, column=4)

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
        self.simulator.update(self.state)
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


def main() -> None:
    root = tk.Tk()
    app = HMIDashboardApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
