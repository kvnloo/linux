// SPDX-License-Identifier: GPL-2.0-only
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <xf86drm.h>

static uint64_t mono_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * 1000000000ull + (uint64_t) ts.tv_nsec;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr,
                "usage: %s /dev/dri/cardX <crtc-id> [seconds] [sample-us]\n",
                argv[0]);
        return 2;
    }

    const char *path = argv[1];
    uint32_t crtc_id = (uint32_t) strtoul(argv[2], NULL, 0);
    double seconds = argc > 3 ? strtod(argv[3], NULL) : 10.0;
    unsigned sample_us = argc > 4 ? (unsigned) strtoul(argv[4], NULL, 0) : 250;

    int fd = open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", path, strerror(errno));
        return 1;
    }

    uint64_t deadline = mono_ns() + (uint64_t)(seconds * 1000000000.0);

    puts("sample_ns,sequence,first_pixel_ns,age_ns");

    while (mono_ns() < deadline) {
        uint64_t seq = 0;
        uint64_t first_pixel_ns = 0;
        int ret = drmCrtcGetSequence(fd, crtc_id, &seq, &first_pixel_ns);
        uint64_t sample_ns = mono_ns();

        if (ret != 0) {
            fprintf(stderr, "drmCrtcGetSequence(%u): %s\n",
                    crtc_id, strerror(-ret));
            close(fd);
            return 1;
        }

        uint64_t age_ns = sample_ns >= first_pixel_ns
            ? sample_ns - first_pixel_ns
            : 0;

        printf("%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n",
               sample_ns, seq, first_pixel_ns, age_ns);

        if (sample_us)
            usleep(sample_us);
    }

    close(fd);
    return 0;
}
