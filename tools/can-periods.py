#!/usr/bin/env python3
"""Measure per-ID frame periods from a CAN log.

Supports the same two formats as tools/can-playback/code.py:
  candump -l:        (1736600000.123456) can0 186#0000320320E5
  SavvyCAN/GVRET:    Time Stamp,ID,Extended,Dir,Bus,LEN,D1..D8  (timestamp in us)

Usage:
  python3 can-periods.py LOGFILE [--ids 354,3B7,3BF,5DE,55D,350]

Output per ID: frame count, median / p95 / max inter-frame gap in ms.
Use the max gap to confirm each signal the firmware times out on (SPEC 6)
broadcasts comfortably inside its timeout.
"""
import argparse
import statistics
import sys
from collections import defaultdict


def parse_candump(line):
    line = line.strip()
    if not line.startswith("("):
        return None
    try:
        end = line.index(")")
        ts = float(line[1:end])
        _iface, frame = line[end + 1:].strip().split(None, 1)
        id_str, _data = frame.split("#", 1)
        return ts, int(id_str.strip(), 16), "Rx"
    except (ValueError, IndexError):
        return None


def parse_gvret(line):
    line = line.strip()
    if not line or line.lower().startswith("time stamp"):
        return None
    f = line.split(",")
    if len(f) < 6:
        return None
    try:
        return int(f[0]) / 1_000_000.0, int(f[1], 16), (f[3].strip() or "Rx")
    except ValueError:
        return None


def detect(path):
    with open(path) as fh:
        for raw in fh:
            s = raw.strip()
            if not s:
                continue
            if s.startswith("("):
                return parse_candump
            if s.lower().startswith("time stamp") or parse_gvret(s):
                return parse_gvret
            return None
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--ids", default="", help="comma-separated hex IDs to report (default: all)")
    args = ap.parse_args()

    parser = detect(args.log)
    if parser is None:
        sys.exit("unrecognised log format")

    wanted = {int(x, 16) for x in args.ids.split(",") if x.strip()}
    stamps = defaultdict(list)
    with open(args.log) as fh:
        for raw in fh:
            r = parser(raw)
            if r is None:
                continue
            ts, cid, direction = r
            if direction == "Tx":
                continue
            if wanted and cid not in wanted:
                continue
            stamps[cid].append(ts)

    print(f"{'ID':>6} {'count':>7} {'median_ms':>10} {'p95_ms':>8} {'max_ms':>8}")
    for cid in sorted(wanted or stamps):
        ts = sorted(stamps.get(cid, []))
        if len(ts) < 2:
            print(f"{cid:>#6x} {len(ts):>7}   (not enough frames)")
            continue
        gaps = [(b - a) * 1000.0 for a, b in zip(ts, ts[1:])]
        gaps.sort()
        p95 = gaps[min(len(gaps) - 1, int(0.95 * len(gaps)))]
        print(f"{cid:>#6x} {len(ts):>7} {statistics.median(gaps):>10.1f} {p95:>8.1f} {gaps[-1]:>8.1f}")


if __name__ == "__main__":
    main()
