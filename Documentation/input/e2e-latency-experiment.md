# Linux input event-age probe

This branch adds a userspace probe for the evdev boundary. It does not change kernel behavior.

## Build

```sh
cc -O2 tools/input/evdev-event-age.c -o /tmp/evdev-event-age
```

Find the mouse event node, then run:

```sh
sudo /tmp/evdev-event-age /dev/input/eventX 15 > capture.csv
python3 tools/input/analyze-event-age.py capture.csv
```

The probe requests `CLOCK_MONOTONIC` timestamps with `EVIOCSCLOCKID` and records the age of each `SYN_REPORT` when userspace receives the read batch.

## Matrix

Sweep supported report rates, then repeat under controlled CPU contention and under the scheduler configurations from the sched_ext wave.

Capture `perf stat` counters in parallel when practical.

## Interpretation

This measures the kernel-event to userspace-read boundary. It does not include sensor/firmware/USB time before the kernel timestamp.

If this distribution stays flat while SDL/engine event age grows, the bottleneck is above evdev. If it grows under contention, scheduler/wakeup behavior is part of the input-latency budget.
