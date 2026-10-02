#!/usr/bin/env python3
import csv
import statistics
import sys

if len(sys.argv) != 2:
    raise SystemExit(f"usage: {sys.argv[0]} samples.csv")

with open(sys.argv[1], newline="") as f:
    rows = list(csv.DictReader(f))

if not rows:
    raise SystemExit("no samples")

# Collapse repeated polling of the same refresh sequence.
by_seq = {}
for row in rows:
    seq = int(row["sequence"])
    by_seq.setdefault(seq, int(row["first_pixel_ns"]))

ordered = sorted(by_seq.items())
times = [ts for _, ts in ordered]
intervals = [b - a for a, b in zip(times, times[1:]) if b > a]

def pct(values, q):
    values = sorted(values)
    if not values:
        return 0
    return values[round((len(values) - 1) * q)]

print(f"unique_sequences={len(ordered)}")
if intervals:
    print(f"refresh_interval_p50_ms={pct(intervals, .50)/1e6:.6f}")
    print(f"refresh_interval_p95_ms={pct(intervals, .95)/1e6:.6f}")
    print(f"refresh_interval_p99_ms={pct(intervals, .99)/1e6:.6f}")
    print(f"refresh_interval_min_ms={min(intervals)/1e6:.6f}")
    print(f"refresh_interval_max_ms={max(intervals)/1e6:.6f}")
    print(f"refresh_hz_from_median={1e9/pct(intervals, .50):.3f}")
    print(f"interval_stddev_ms={statistics.pstdev(intervals)/1e6:.6f}")
