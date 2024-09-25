#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <arm_neon.h>
#include <time.h>
#include <string.h>
#include <mach/mach_time.h>
#include <pthread.h>

#define NUM_THREADS 24 // Number of threads (adjust to your needs)
// Define a large array to perform cache-busting operations
#define ARRAY_SIZE (1024 * 1024 * 16) // 16 MB

uint8_t data_array[ARRAY_SIZE] __attribute__((aligned(16)));

// Timebase info for converting between Mach time units and nanoseconds
static mach_timebase_info_data_t timebase_info;

// Thread function to perform high bus activity with NEON intrinsics
void* thread_high_bus_activity_neon(void* arg) {
    size_t start = (size_t)arg;
    size_t segment_size = ARRAY_SIZE / NUM_THREADS;

    uint8x16_t vec_zero = vdupq_n_u8(0);  // 128-bit register filled with 0x00
    uint8x16_t vec_one = vdupq_n_u8(0xFF); // 128-bit register filled with 0xFF

    for (size_t i = start; i < start + segment_size; i += 16) {
        // Introduce a random access pattern to increase cache miss rate
        size_t random_offset = (rand() % (segment_size / 16)) * 16;
        
        // Use NEON stores with random offset within the segment
        vst1q_u8(&data_array[random_offset], vec_one);
        vst1q_u8(&data_array[random_offset], vec_zero);
    }

    return NULL;
}

// Function to perform high bus activity across multiple threads
void perform_high_bus_activity(void) {
    pthread_t threads[NUM_THREADS];

    // Create NUM_THREADS threads to spread the workload across the memory array
    for (int i = 0; i < NUM_THREADS; i++) {
        size_t start = i * (ARRAY_SIZE / NUM_THREADS);
        pthread_create(&threads[i], NULL, thread_high_bus_activity_neon, (void*)start);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

static inline void square_am_signal(uint64_t freq_hz, uint64_t time_ms) {
    uint64_t start = mach_absolute_time();
    uint64_t end = start + time_ms * 1000000 * timebase_info.denom / timebase_info.numer;

    if (freq_hz == 0) {
        // If frequency is 0, perform silence by sleeping for the entire duration
        mach_wait_until(end);
        return;
    }

    // Calculate the period for non-zero frequency
    uint64_t period = 1000000000 / freq_hz * timebase_info.denom / timebase_info.numer;

    while (mach_absolute_time() < end) {
        uint64_t mid = start + period / 2;
        uint64_t next_period = start + period;

        // High activity phase
        while (mach_absolute_time() < mid) {
            perform_high_bus_activity();
        }

        // Sleep until the next period to modulate the signal
        mach_wait_until(next_period);
        start = next_period;
    }
}

int main(int argc, char* argv[]) {
    // Initialize the timebase info
    mach_timebase_info(&timebase_info);

    // Seed the random number generator
    srand(time(NULL));

    if (argc != 2) {
        fprintf(stderr, "No song file given!\nUsage: %s file.song\n", argv[0]);
        return(1);
    }

    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        perror("fopen");
        return(EXIT_FAILURE);
    }

    char buffer[64];
    int freq_hz, time_ms;

    while (1) {
        if (!fgets(buffer, sizeof(buffer), fp)) {
            if (feof(fp)) {
                rewind(fp);
                continue;
            } else {
                perror("fgets");
                break;
            }
        }

        if (sscanf(buffer, "%d %d", &freq_hz, &time_ms) == 2) {
            square_am_signal(freq_hz, time_ms);
        }
    }

    fclose(fp);
    return 0;
}