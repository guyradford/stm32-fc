from __future__ import annotations

import queue
import threading
import time
from dataclasses import dataclass
from typing import Literal

try:
    import serial
    from serial.tools import list_ports
except ModuleNotFoundError:
    class _SerialShim:
        EIGHTBITS = 8
        PARITY_NONE = "N"
        STOPBITS_ONE = 1

        class SerialException(Exception):
            pass

        class SerialTimeoutException(SerialException):
            pass

        def Serial(self, *_args: object, **_kwargs: object) -> object:
            raise self.SerialException("pyserial is not installed")

    serial = _SerialShim()
    list_ports = None

from telemetry_protocol import TelemetryError, TelemetryFrame, format_stop, format_sub, parse_sentence


SerialEventKind = Literal["frame", "log", "error", "closed"]


@dataclass(frozen=True)
class SerialEvent:
    kind: SerialEventKind
    message: str
    frame: TelemetryFrame | None = None
    raw_sentence: str | None = None


def list_serial_ports() -> list[str]:
    if list_ports is None:
        return []
    return [port.device for port in list_ports.comports()]


class TelemetrySerialClient:
    def __init__(self) -> None:
        self.events: queue.Queue[SerialEvent] = queue.Queue()
        self._serial: serial.Serial | None = None
        self._thread: threading.Thread | None = None
        self._handshake_thread: threading.Thread | None = None
        self._stop_requested = threading.Event()
        self._telemetry_seen = threading.Event()
        self._write_lock = threading.Lock()

    @property
    def connected(self) -> bool:
        return self._serial is not None and self._serial.is_open

    def connect(self, port: str, baud: int) -> None:
        if self.connected:
            return

        self._stop_requested.clear()
        self._telemetry_seen.clear()
        self._serial = serial.Serial(
            port=port,
            baudrate=baud,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.1,
            write_timeout=0.2,
            rtscts=False,
            dsrdtr=False,
            xonxoff=False,
        )
        self._serial.dtr = True
        self._serial.rts = True
        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._thread = threading.Thread(target=self._read_loop, name="hmi-telemetry-serial", daemon=True)
        try:
            self._thread.start()
            self._handshake_thread = threading.Thread(target=self._handshake_loop, name="hmi-telemetry-handshake", daemon=True)
            self._handshake_thread.start()
            self.events.put(SerialEvent("log", "Opened %s at %d baud" % (port, baud)))
            self.events.put(SerialEvent("log", "Serial lines DTR:%s RTS:%s CTS:%s DSR:%s" % (
                self._serial.dtr,
                self._serial.rts,
                self._serial.cts,
                self._serial.dsr,
            )))
        except serial.SerialException:
            self.disconnect()
            raise

    def disconnect(self) -> None:
        self._stop_requested.set()
        if self._serial is not None and self._serial.is_open:
            try:
                self.send_text(format_stop())
            except serial.SerialException:
                pass

        if self._handshake_thread is not None:
            self._handshake_thread.join(timeout=0.5)
        if self._thread is not None:
            self._thread.join(timeout=0.5)
        if self._serial is not None:
            try:
                self._serial.close()
            except serial.SerialException:
                pass
        self._serial = None
        self._thread = None
        self._handshake_thread = None
        self.events.put(SerialEvent("closed", "Serial connection closed"))

    def send_text(self, text: str) -> None:
        if self._serial is None or not self._serial.is_open:
            raise serial.SerialException("serial port is not open")
        data = text.encode("ascii")
        with self._write_lock:
            written = self._serial.write(data)
            self._serial.flush()
        if written != len(data):
            raise serial.SerialTimeoutException("only wrote %d of %d bytes" % (written, len(data)))

    def request_home(self) -> None:
        self.send_text("h")
        self.events.put(SerialEvent("log", "TX h"))

    def request_telemetry(self) -> None:
        self.send_text("n")
        self.events.put(SerialEvent("log", "TX n"))

    def subscribe(self, subject: str, rate_hz: int) -> None:
        self.send_text(format_sub(subject, rate_hz))
        self.events.put(SerialEvent("log", "TX SUB %s %d" % (subject, rate_hz)))

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
            if "Telemetry Mode" in line and not self._telemetry_seen.is_set():
                try:
                    self.request_telemetry()
                    self.events.put(SerialEvent("log", "Telemetry menu detected"))
                except serial.SerialException as exc:
                    self.events.put(SerialEvent("error", "Telemetry menu response failed: %s" % exc))
            return

        try:
            frame = parse_sentence(line)
        except TelemetryError as exc:
            self.events.put(SerialEvent("error", "Telemetry parse error: %s" % exc))
            return

        self.events.put(SerialEvent("frame", frame.payload, frame, line + "\r\n"))
        self._telemetry_seen.set()

    def _handshake_loop(self) -> None:
        self._telemetry_seen.wait(timeout=1.0)
        if self._stop_requested.is_set() or self._telemetry_seen.is_set():
            return

        try:
            self.request_home()
            time.sleep(0.5)
        except serial.SerialException as exc:
            self.events.put(SerialEvent("error", "Telemetry handshake failed: %s" % exc))
            return

        attempt = 1
        while not self._stop_requested.is_set() and not self._telemetry_seen.is_set():
            if attempt > 30:
                self.events.put(SerialEvent("error", "Telemetry handshake still waiting for data"))
                attempt = 1
            if self._stop_requested.is_set() or self._telemetry_seen.is_set():
                return
            try:
                self.request_telemetry()
            except serial.SerialException as exc:
                self.events.put(SerialEvent("error", "Telemetry handshake failed: %s" % exc))
                return
            self.events.put(SerialEvent("log", "Telemetry handshake attempt %d" % attempt))
            attempt += 1
            self._telemetry_seen.wait(timeout=1.0)
