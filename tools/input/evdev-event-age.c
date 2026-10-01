// SPDX-License-Identifier: GPL-2.0-only
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static uint64_t event_ns(const struct input_event *ev)
{
    return (uint64_t)ev->time.tv_sec * 1000000000ull +
           (uint64_t)ev->time.tv_usec * 1000ull;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s /dev/input/eventX [seconds]\n", argv[0]);
        return 2;
    }

    double seconds = argc > 2 ? atof(argv[2]) : 15.0;
    int fd = open(argv[1], O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    int clock_id = CLOCK_MONOTONIC;
    if (ioctl(fd, EVIOCSCLOCKID, &clock_id) < 0) {
        fprintf(stderr, "warning: EVIOCSCLOCKID failed: %s\n", strerror(errno));
    }

    struct input_event events[128];
    uint64_t deadline = now_ns() + (uint64_t)(seconds * 1000000000.0);

    puts("event_ns,read_ns,age_ns,events_in_read");

    while (now_ns() < deadline) {
        ssize_t bytes = read(fd, events, sizeof(events));
        uint64_t read_time = now_ns();

        if (bytes < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "read: %s\n", strerror(errno));
            close(fd);
            return 1;
        }

        size_t count = (size_t)bytes / sizeof(events[0]);
        for (size_t i = 0; i < count; i++) {
            if (events[i].type == EV_SYN && events[i].code == SYN_REPORT) {
                uint64_t ts = event_ns(&events[i]);
                uint64_t age = read_time >= ts ? read_time - ts : 0;
                printf("%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%zu\n",
                       ts, read_time, age, count);
            }
        }
    }

    close(fd);
    return 0;
}
