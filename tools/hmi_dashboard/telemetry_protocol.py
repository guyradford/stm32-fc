from __future__ import annotations

from dataclasses import dataclass


class TelemetryError(ValueError):
    pass


@dataclass(frozen=True)
class TelemetryFrame:
    subject: str
    fields: tuple[str, ...]
    payload: str


EXPECTED_FIELDS = {
    "RC": 8,
    "RCR": 9,
    "IMU": 8,
    "IMUC": 6,
    "IMUR": 10,
    "MOT": 5,
    "STAT": 10,
    "PID": 10,
    "CTL": 7,
}


def checksum(payload: str) -> int:
    value = 0
    for character in payload:
        value ^= ord(character)
    return value


def format_sentence(payload: str) -> str:
    return "$%s*%02X\r\n" % (payload, checksum(payload))


def format_stop() -> str:
    return format_sentence("STOP")


def format_req(subject: str) -> str:
    return format_sentence("REQ,%s" % subject)


def format_sub(subject: str, rate_hz: int) -> str:
    return format_sentence("SUB,%s,%d" % (subject, rate_hz))


def parse_sentence(line: str) -> TelemetryFrame:
    text = line.strip()
    if not text:
        raise TelemetryError("empty line")
    if not text.startswith("$"):
        raise TelemetryError("missing start marker")
    if "*" not in text:
        raise TelemetryError("missing checksum marker")

    payload, checksum_text = text[1:].split("*", 1)
    if len(checksum_text) != 2:
        raise TelemetryError("checksum must be two hex digits")
    try:
        received = int(checksum_text, 16)
    except ValueError as exc:
        raise TelemetryError("checksum is not hex") from exc

    calculated = checksum(payload)
    if received != calculated:
        raise TelemetryError("bad checksum: expected %02X got %02X" % (calculated, received))

    parts = payload.split(",")
    subject = parts[0]
    fields = tuple(parts[1:])
    if not subject:
        raise TelemetryError("missing subject")

    expected = EXPECTED_FIELDS.get(subject)
    if expected is not None and len(fields) != expected:
        raise TelemetryError("%s expected %d fields got %d" % (subject, expected, len(fields)))

    if subject == "ACK" and len(fields) not in (1, 2):
        raise TelemetryError("ACK expected one or two fields")
    if subject == "ERR" and len(fields) != 2:
        raise TelemetryError("ERR expected two fields")

    return TelemetryFrame(subject=subject, fields=fields, payload=payload)
