// SYSTEM BUS RADIO - M1 PULSE-PACKET TRANSMITTER (4 CORES)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arm_neon.h>
#include <time.h>
#include <mach/mach_time.h>
#include <mach/thread_policy.h>
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#include <pthread.h>
#include <pthread/qos.h>

#define NUM_THREADS 4
#define MEM_SIZE (64 * 1024 * 1024)
#define PACKET_MS 20 

uint8_t *g_mem;
static mach_timebase_info_data_t timebase_info;
volatile int g_running = 1;

// Shared Parameters
volatile uint64_t g_current_freq = 0;
volatile uint64_t g_current_duration_ms = 0;

// Custom Barrier Structure
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int crossing;
    int total;
} my_barrier_t;

my_barrier_t g_barrier_start;
my_barrier_t g_barrier_end;

void barrier_init(my_barrier_t *b, int total) {
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
    b->count = 0;
    b->crossing = 0;
    b->total = total;
}

void barrier_wait(my_barrier_t *b) {
    pthread_mutex_lock(&b->mutex);
    b->count++;
    if (b->count >= b->total) {
        b->crossing++;
        b->count = 0;
        pthread_cond_broadcast(&b->cond);
    } else {
        int my_crossing = b->crossing;
        while (my_crossing == b->crossing) {
            pthread_cond_wait(&b->cond, &b->mutex);
        }
    }
    pthread_mutex_unlock(&b->mutex);
}

static inline void bus_poke(size_t offset) {
    // Write 0xFFFFFFFFFFFFFFFF to the bus
    uint64_t val = 0xFFFFFFFFFFFFFFFFull;
    asm volatile (
        "stnp %0, %0, [%1]\n" "stnp %0, %0, [%1, #16]\n"
        "stnp %0, %0, [%1, #32]\n" "stnp %0, %0, [%1, #48]\n"
        : : "r"(val), "r"(&g_mem[offset]) : "memory"
    );
}

void set_realtime(int affinity_tag) {
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
    thread_affinity_policy_data_t affinity;
    affinity.affinity_tag = affinity_tag;
    thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, (thread_policy_t)&affinity, THREAD_AFFINITY_POLICY_COUNT);
}

void* worker_thread(void* arg) {
    int id = (int)(size_t)arg;
    set_realtime(id + 1);

    size_t segment = MEM_SIZE / NUM_THREADS;
    size_t offset = id * segment;
    size_t end_offset = offset + segment - 64;
    size_t current = offset;

    while (g_running) {
        // 1. Wait for start signal
        barrier_wait(&g_barrier_start);
        if (!g_running) break;

        uint64_t freq = g_current_freq;
        uint64_t duration = g_current_duration_ms;

        if (freq > 0) {
            uint64_t start = mach_absolute_time();
            uint64_t end = start + (duration * 1000000ull * timebase_info.denom / timebase_info.numer);
            uint64_t period = (1000000000ull / freq) * timebase_info.denom / timebase_info.numer;
            uint64_t half_period = period / 2;

            uint64_t now = start;
            while (now < end) {
                uint64_t next_edge = now + half_period;
                // ON Phase
                while (mach_absolute_time() < next_edge) {
                    bus_poke(current);
                    current += 64;
                    if (current >= end_offset) current = offset;
                }
                next_edge += half_period;
                // OFF Phase
                while (mach_absolute_time() < next_edge) {
                    asm volatile("yield");
                }
                now = next_edge;
            }
        }

        // 2. Report completion
        barrier_wait(&g_barrier_end);
    }
    return NULL;
}

void play_tone(uint64_t freq_hz, uint64_t time_ms) {
    uint64_t remaining = time_ms;
    while (remaining > 0) {
        uint64_t chunk = (remaining > PACKET_MS) ? PACKET_MS : remaining;
        
        g_current_freq = freq_hz;
        g_current_duration_ms = chunk;
        
        // START
        barrier_wait(&g_barrier_start);
        
        // END WAIT
        barrier_wait(&g_barrier_end);
        
        // Short pause (0.5ms) to reset throttling counters
        struct timespec ts = {0, 500000};
        nanosleep(&ts, NULL);
        
        remaining -= chunk;
    }
}

int main(int argc, char* argv[]) {
    mach_timebase_info(&timebase_info);
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file.tune\n", argv[0]);
        return 1;
    }
    FILE* fp = fopen(argv[1], "r");
    if (!fp) { perror("fopen"); return 1; }

    g_mem = malloc(MEM_SIZE);
    memset(g_mem, 0, MEM_SIZE);

    barrier_init(&g_barrier_start, NUM_THREADS + 1);
    barrier_init(&g_barrier_end, NUM_THREADS + 1);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker_thread, (void*)(size_t)i);
    }

    set_realtime(0);
    printf("M1 Pulse-Packet Mode (Custom Barrier). Playing...\n");

    char buffer[128];
    int freq_hz, time_ms;
    while (1) {
        if (fgets(buffer, sizeof(buffer), fp) == NULL) {
            if (feof(fp)) {
                rewind(fp);
                continue;
            }
            break;
        }
        if (buffer[0] == '\n' || buffer[0] == '#' || buffer[0] == ' ') continue;

        if (sscanf(buffer, "%d %d", &freq_hz, &time_ms) == 2) {
            printf("\r%4d Hz ", freq_hz);
            fflush(stdout);
            play_tone(freq_hz, time_ms);
        }
    }

    g_running = 0;
    barrier_wait(&g_barrier_start); // Release waiting threads
    
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
    
    fclose(fp);
    return 0;
}
