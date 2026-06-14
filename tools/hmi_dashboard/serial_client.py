from __future__ import annotations

import queue
import threading
import time
from dataclasses import dataclass
from typing import Literal

import serial
from serial.tools import list_ports

from telemetry_protocol import TelemetryError, TelemetryFrame, format_stop, parse_sentence


SerialEventKind = Literal["frame", "log", "error", "closed"]


@dataclass(frozen=True)
class SerialEvent:
    kind: SerialEventKind
    message: str
    frame: TelemetryFrame | None = None


def list_serial_ports() -> list[str]:
    return [port.device for port in list_ports.comports()]


class TelemetrySerialClient:
    def __init__(self) -> None:
        self.events: queue.Queue[SerialEvent] = queue.Queue()
        self._serial: serial.Serial | None = None
        self._thread: threading.Thread | None = None
        self._stop_requested = threading.Event()

    @property
    def connected(self) -> bool:
        return self._serial is not None and self._serial.is_open

    def connect(self, port: str, baud: int) -> None:
        if self.connected:
            return

        self._stop_requested.clear()
        self._serial = serial.Serial(port=port, baudrate=baud, timeout=0.1, write_timeout=0.2)
        self._thread = threading.Thread(target=self._read_loop, name="hmi-telemetry-serial", daemon=True)
        try:
            self._thread.start()
            self.send_text("h")
            time.sleep(0.05)
            self.send_text("n")
            self.events.put(SerialEvent("log", "Opened %s at %d baud" % (port, baud)))
        except serial.SerialException:
            self.disconnect()
            raise

    def disconnect(self) -> None:
        if self._serial is not None and self._serial.is_open:
            try:
                self.send_text(format_stop())
            except serial.SerialException:
                pass

        self._stop_requested.set()
        if self._thread is not None:
            self._thread.join(timeout=0.5)
        if self._serial is not None:
            try:
                self._serial.close()
            except serial.SerialException:
                pass
        self._serial = None
        self._thread = None
        self.events.put(SerialEvent("closed", "Serial connection closed"))

    def send_text(self, text: str) -> None:
        if self._serial is None or not self._serial.is_open:
            raise serial.SerialException("serial port is not open")
        self._serial.write(text.encode("ascii"))

    def _read_loop(self) -> None:
        buffer = bytearray()
        while not self._stop_requested.is_set():
            try:
                assert self._serial is not None
                chunk = self._serial.read(64)
            except serial.SerialException as exc:
                self.events.put(SerialEvent("error", "Serial read failed: %s" % exc))
                break

            if not chunk:
                continue

            for byte in chunk:
                if byte in (10, 13):
                    if buffer:
                        self._process_line(bytes(buffer))
                        buffer.clear()
                else:
                    buffer.append(byte)
                    if len(buffer) > 160:
                        self.events.put(SerialEvent("error", "Dropped overlong telemetry line"))
                        buffer.clear()

        self.events.put(SerialEvent("closed", "Serial reader stopped"))

    def _process_line(self, raw_line: bytes) -> None:
        try:
            line = raw_line.decode("ascii")
        except UnicodeDecodeError:
            self.events.put(SerialEvent("error", "Dropped non-ASCII telemetry line"))
            return

        if not line.startswith("$"):
            self.events.put(SerialEvent("log", line))
            return

        try:
            frame = parse_sentence(line)
        except TelemetryError as exc:
            self.events.put(SerialEvent("error", "Telemetry parse error: %s" % exc))
            return

        self.events.put(SerialEvent("frame", frame.payload, frame))
