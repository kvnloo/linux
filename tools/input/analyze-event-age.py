#!/usr/bin/env python3
import csv
import statistics
import sys

if len(sys.argv) != 2:
    raise SystemExit(f"usage: {sys.argv[0]} capture.csv")

with open(sys.argv[1], newline="") as f:
    rows = list(csv.DictReader(f))

ages = sorted(int(row["age_ns"]) for row in rows)
batches = [int(row["events_in_read"]) for row in rows]

if not ages:
    raise SystemExit("no SYN_REPORT rows")

def pct(values, q):
    i = round((len(values) - 1) * q)
    return values[i]

print(f"samples={len(ages)}")
print(f"age_p50_us={pct(ages, 0.50) / 1000:.3f}")
print(f"age_p95_us={pct(ages, 0.95) / 1000:.3f}")
print(f"age_p99_us={pct(ages, 0.99) / 1000:.3f}")
print(f"age_p999_us={pct(ages, 0.999) / 1000:.3f}")
print(f"age_max_us={ages[-1] / 1000:.3f}")
print(f"batch_mean_events={statistics.fmean(batches):.3f}")
print(f"batch_max_events={max(batches)}")
