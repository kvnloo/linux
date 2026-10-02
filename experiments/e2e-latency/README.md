# DRM scanout timing lab

This branch adds a userspace-only probe. It does not modify kernel DRM behavior.

Linux DRM's CRTC sequence timestamp is defined around the time the first pixel of a refresh cycle leaves the display engine for the display. That gives us a useful software timestamp to compare against a photodiode.

## Build

Requires libdrm headers:

```sh
cc -O2 -Wall -Wextra -Werror \
  experiments/e2e-latency/drm-first-pixel.c \
  -o /tmp/drm-first-pixel \
  $(pkg-config --cflags --libs libdrm)
```

Find the active CRTC ID with `drm_info` or `modetest`.

Then sample:

```sh
/tmp/drm-first-pixel /dev/dri/card0 <crtc-id> 15 250 > drm.csv
python3 experiments/e2e-latency/analyze-drm.py drm.csv
```

## What it measures

Each row contains:

- userspace sampling time;
- CRTC refresh sequence;
- kernel-reported first-pixel timestamp;
- age of that timestamp when read.

The analyzer collapses repeated polling of the same CRTC sequence and reports the physical refresh-interval distribution implied by DRM.

## Wave 4 correlation

Run this simultaneously with:

- frameprobe photodiode capture;
- gamescope/Hyprland presentation trace;
- Vulkan present timing.

Then correlate by monotonic time.

## VRR

Under VRR, refresh intervals should vary. Do not treat that variance as jitter by default. Compare it to requested present timing and photon timing.

## Caveat

The DRM timestamp is at the display-engine boundary, not the panel photon. Cable/link transport, scanout position, panel electronics, pixel response, and strobe phase still remain downstream.
