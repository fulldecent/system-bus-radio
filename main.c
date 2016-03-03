// SYSTEM BUS RADIO
// https://github.com/fulldecent/system-bus-radio
// Copyright 2016 William Entriken

#include <stdio.h>
#include <mach/mach_traps.h>
#include <mach/mach_time.h>

#include <pthread.h> 

volatile uint64_t mid;
volatile uint64_t reset;

pthread_cond_t cv; 

pthread_mutex_t mutex; 


mach_port_t clock_port;
mach_timespec_t remain;

volatile uint64_t x = 0;

static void* boost_song(void* arg) {
    while (1) {
        pthread_cond_wait(&cv, &mutex);
        while (mach_absolute_time() < mid) {
            x += mid * mid;
        }
        clock_sleep_trap(clock_port, TIME_ABSOLUTE, reset / NSEC_PER_SEC, reset % NSEC_PER_SEC, &remain);
    }
}


static inline void square_am_signal(float time, float frequency) {
    printf("Playing / %0.3f seconds / %4.0f Hz\n", time, frequency);
    uint64_t period = NSEC_PER_SEC / frequency;

    uint64_t start = mach_absolute_time();
    uint64_t end = start + time * NSEC_PER_SEC;

    

    while (mach_absolute_time() < end) {
        mid = start + period / 2;
        reset = start + period;
        pthread_cond_broadcast(&cv);
        clock_sleep_trap(clock_port, TIME_ABSOLUTE, reset / NSEC_PER_SEC, reset % NSEC_PER_SEC, &remain);
        start = reset;
    }
}

int main()
{
    int i;
    mach_timebase_info_data_t theTimeBaseInfo;
    mach_timebase_info(&theTimeBaseInfo);
    puts("TESTING TIME BASE: the following should be 1 / 1");
    printf("  Mach base: %u / %u nanoseconds\n\n", theTimeBaseInfo.numer, theTimeBaseInfo.denom);

    uint64_t start = mach_absolute_time();
    uint64_t end = mach_absolute_time();
    printf("TESTING TIME TO EXECUTE mach_absolute_time()\n  Result: %lld nanoseconds\n\n", end - start);

    for (i=0;i<8;++i) {
        pthread_t t;
        pthread_create(&t,0,&boost_song,0);
    }
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
