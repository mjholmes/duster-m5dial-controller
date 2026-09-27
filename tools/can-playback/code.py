# CAN bus playback simulator - Adafruit RP2040 CAN Bus Feather (CircuitPython)
#
# Drop a log file onto the CIRCUITPY drive as LOG_FILENAME below. Supports
# TWO formats, auto-detected from the file's contents:
#   - candump -l style:      (1736600000.123456) can0 186#0000320320E5
#   - SavvyCAN/GVRET CSV:    Time Stamp,ID,Extended,Dir,Bus,LEN,D1..D8
#                            7907393,00000186,false,Rx,0,7,00,00,32,03,20,04,E5,00,
#
# On boot/reset this replays the file with the original inter-frame timing,
# looping forever. The file is re-read from disk each lap rather than held
# in memory, so log length isn't limited by the board's RAM.
#
# NeoPixel status:
#   yellow (brief) = detecting format / starting up
#   blue           = playing a candump-format log
#   magenta        = playing a SavvyCAN/GVRET-format log
#   orange (brief) = a bad line was skipped or a frame failed to send
#   red            = fatal error, playback halted - blink count tells you why:
#     1 blink  = log file not found / unreadable
#     2 blinks = CAN controller failed to initialise
#     3 blinks = file exists but no valid frames could be parsed from it
#     4 blinks = file format not recognised (neither candump nor GVRET)
#
# Requires /lib/adafruit_mcp2515/ and /lib/adafruit_bus_device/ from the
# Adafruit CircuitPython Library Bundle matching your CircuitPython version.

import time
import board
from digitalio import DigitalInOut

LOG_FILENAME = "/playback.log"
BUS_BAUDRATE = 500000     # Duster OBD-CAN runs at 500 kbit/s
SKIP_TX_FRAMES = True     # GVRET logs may include the capture device's own
                           # Tx frames (e.g. diagnostic requests) - skipped
                           # by default so only real car traffic is replayed
MAX_FRAME_WAIT = 5.0      # seconds - clamp any single inter-frame gap so a
                           # corrupt/huge timestamp can't stall playback

# --- NeoPixel status helper (optional - degrades gracefully if absent) ---
try:
    import neopixel

    pixel = neopixel.NeoPixel(board.NEOPIXEL, 1, brightness=0.2)
except (AttributeError, ImportError, ValueError):
    pixel = None

COLOR_STARTUP = (40, 40, 0)
COLOR_CANDUMP = (0, 0, 40)
COLOR_GVRET = (40, 0, 40)
COLOR_WARN = (40, 20, 0)
COLOR_ERROR = (40, 0, 0)
COLOR_OFF = (0, 0, 0)


def set_status(color):
    if pixel is not None:
        try:
            pixel[0] = color
        except Exception:
            pass


def flash(color, duration=0.15):
    set_status(color)
    time.sleep(duration)


def halt(blink_count):
    """Blink red N times, pause, repeat forever - a serial-console-free
    diagnostic code for when something is wrong at startup."""
    while True:
        for _ in range(blink_count):
            set_status(COLOR_ERROR)
            time.sleep(0.25)
            set_status(COLOR_OFF)
            time.sleep(0.25)
        time.sleep(1.0)


# --- CAN bus init (defensive: don't brick the board on a wiring fault) ---
set_status(COLOR_STARTUP)
try:
    from adafruit_mcp2515 import MCP2515 as CAN
    from adafruit_mcp2515.canio import Message

    cs = DigitalInOut(board.CAN_CS)
    cs.switch_to_output()
    spi = board.SPI()
    can_bus = CAN(spi, cs, baudrate=BUS_BAUDRATE, loopback=False, silent=False)
except Exception as e:
    print("CAN init failed:", e)
    halt(2)


# --- File existence check, separate from format detection ---
try:
    with open(LOG_FILENAME, "r"):
        pass
except OSError as e:
    print("Could not open", LOG_FILENAME, ":", e)
    halt(1)


# --- Format-specific line parsers ---
# Each returns (timestamp_seconds, can_id, data_bytes, extended, direction)
# or None for a line that should be skipped (blank, header, malformed).

def parse_candump_line(line):
    line = line.strip()
    if not line or not line.startswith("("):
        return None
    try:
        ts_end = line.index(")")
        timestamp = float(line[1:ts_end])
        rest = line[ts_end + 1:].strip()
        _iface, frame = rest.split(None, 1)
        id_str, data_str = frame.split("#", 1)
        id_str = id_str.strip()
        can_id = int(id_str, 16)
        data = bytes.fromhex(data_str.strip())
        extended = len(id_str) > 3
        return timestamp, can_id, data, extended, "Rx"
    except (ValueError, IndexError):
        return None


def parse_gvret_line(line):
    line = line.strip()
    if not line:
        return None
    if line.lower().startswith("time stamp"):
        return None  # header row
    fields = line.split(",")
    if len(fields) < 14:
        return None
    try:
        ts_raw = int(fields[0].strip())
        can_id = int(fields[1].strip(), 16)
        extended = fields[2].strip().lower() == "true"
        direction = fields[3].strip() or "Rx"
        length = int(fields[5].strip())
        if length < 0 or length > 8:
            return None
        data_fields = fields[6:6 + length]
        if len(data_fields) < length:
            return None
        data = bytes(int(b.strip(), 16) for b in data_fields)
        timestamp = ts_raw / 1_000_000.0
        return timestamp, can_id, data, extended, direction
    except (ValueError, IndexError):
        return None


def detect_format(path):
    """Peek at the file to decide which parser to use. Returns
    (parser_function, status_color) or (None, None) if unrecognised."""
    try:
        with open(path, "r") as f:
            for raw_line in f:
                line = raw_line.strip()
                if not line:
                    continue
                if line.startswith("("):
                    return parse_candump_line, COLOR_CANDUMP
                if line.lower().startswith("time stamp"):
                    return parse_gvret_line, COLOR_GVRET
                # No header present - check if it parses as a GVRET data
                # row anyway (some exports omit the header row).
                if parse_gvret_line(line) is not None:
                    return parse_gvret_line, COLOR_GVRET
                return None, None
    except OSError:
        return None, None
    return None, None


print("Detecting format of", LOG_FILENAME, "...")
parser, play_color = detect_format(LOG_FILENAME)

if parser is None:
    print("Could not recognise file format")
    halt(4)


def frame_source(path, parser):
    """Generator: re-reads the file from disk and yields valid frames one
    at a time, so a long log never has to fit in memory all at once."""
    with open(path, "r") as f:
        for raw_line in f:
            frame = parser(raw_line)
            if frame is None:
                continue
            timestamp, can_id, data, extended, direction = frame
            if SKIP_TX_FRAMES and direction == "Tx":
                continue
            yield timestamp, can_id, data, extended


# --- One-pass check: make sure the file actually yields something to play ---
valid_frame_found = False
for _ in frame_source(LOG_FILENAME, parser):
    valid_frame_found = True
    break

if not valid_frame_found:
    print("No valid frames found in", LOG_FILENAME)
    halt(3)

print("Format OK - starting playback loop")
set_status(play_color)

skipped_count = 0
send_failures = 0

while True:
    playback_start = time.monotonic()
    log_start_ts = None

    for timestamp, can_id, data, extended in frame_source(LOG_FILENAME, parser):
        if log_start_ts is None:
            log_start_ts = timestamp

        target_elapsed = timestamp - log_start_ts
        now_elapsed = time.monotonic() - playback_start
        wait = target_elapsed - now_elapsed

        if wait > MAX_FRAME_WAIT:
            wait = MAX_FRAME_WAIT
        if wait > 0:
            time.sleep(wait)

        try:
            message = Message(id=can_id, data=data, extended=extended)
            ok = can_bus.send(message)
            if not ok:
                send_failures += 1
                flash(COLOR_WARN, 0.02)
                set_status(play_color)
        except Exception as e:
            skipped_count += 1
            print("Send error for ID", hex(can_id), ":", e)
            flash(COLOR_WARN, 0.02)
            set_status(play_color)

    print("Lap complete - skipped:", skipped_count, "send failures:", send_failures)
    # loop back to the start of the file and repeat indefinitely
