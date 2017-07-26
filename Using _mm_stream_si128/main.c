// SYSTEM BUS RADIO
// https://github.com/fulldecent/system-bus-radio
// Copyright 2016 William Entriken

#include <stdio.h>
#include <emmintrin.h>
#include <inttypes.h>
#include <time.h>
#ifdef __MACH__
#include <mach/mach_traps.h>
#include <mach/mach_time.h>
#endif
#include <math.h>

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000ull
#endif

#ifndef __MACH__
#define TIME_ABSOLUTE CLOCK_REALTIME
typedef struct timespec mach_timespec_t;
typedef unsigned int mach_port_t;

static inline uint64_t mach_absolute_time(void) {
    mach_timespec_t tp;
    int res = clock_gettime(CLOCK_REALTIME, &tp);
    if (res < 0) {
        perror("clock_gettime");
        exit(1);
    }
    uint64_t result = tp.tv_sec * NSEC_PER_SEC;
    result += tp.tv_nsec;
    return result;
}

// non-conformant wrapper just for the purposes of this application
static inline void clock_sleep_trap(mach_port_t clock_port, int sleep_type, time_t sec, long nsec, mach_timespec_t *remain) {
    mach_timespec_t req = { sec, nsec };
    int res = clock_nanosleep(sleep_type, TIMER_ABSTIME, &req, remain);
    if (res < 0) {
        perror("clock_nanosleep");
        exit(1);
    }
}
#endif // __MACH__

__m128i reg;
__m128i reg_zero;
__m128i reg_one;
mach_port_t clock_port;
mach_timespec_t remain;

static inline void square_am_signal(float time, float frequency) {
    printf("Playing / %0.3f seconds / %4.0f Hz\n", time, frequency);
    uint64_t period = NSEC_PER_SEC / frequency;

    uint64_t start = mach_absolute_time();
    uint64_t end = start + (uint64_t)(time * NSEC_PER_SEC);

    while (mach_absolute_time() < end) {
        uint64_t mid = start + period / 2;
        uint64_t reset = start + period;
        while (mach_absolute_time() < mid) {
            _mm_stream_si128(&reg, reg_one);
            _mm_stream_si128(&reg, reg_zero);
        }
        clock_sleep_trap(clock_port, TIME_ABSOLUTE, reset / NSEC_PER_SEC, reset % NSEC_PER_SEC, &remain);
        start = reset;
    }
}

int main()
{
#ifdef __MACH__
    mach_timebase_info_data_t theTimeBaseInfo;
    mach_timebase_info(&theTimeBaseInfo);
    puts("TESTING TIME BASE: the following should be 1 / 1");
    printf("  Mach base: %u / %u nanoseconds\n\n", theTimeBaseInfo.numer, theTimeBaseInfo.denom);
#endif

    uint64_t start = mach_absolute_time();
    uint64_t end = mach_absolute_time();
    printf("TESTING TIME TO EXECUTE mach_absolute_time()\n  Result: %"PRIu64" nanoseconds\n\n", end - start);

    reg_zero = _mm_set_epi32(0, 0, 0, 0);
    reg_one = _mm_set_epi32(-1, -1, -1, -1);

    while (1) {
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2349);
        square_am_signal(0.400, 2093);
        square_am_signal(0.400, 2349);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2673);
        square_am_signal(0.790, 2673);
        square_am_signal(0.400, 2349);
        square_am_signal(0.400, 2349);
        square_am_signal(0.790, 2349);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 3136);
        square_am_signal(0.790, 3136);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2349);
        square_am_signal(0.400, 2093);
        square_am_signal(0.400, 2349);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2349);
        square_am_signal(0.400, 2349);
        square_am_signal(0.400, 2673);
        square_am_signal(0.400, 2349);
        square_am_signal(0.790, 2093);
    }
}
