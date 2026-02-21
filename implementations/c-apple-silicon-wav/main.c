// SYSTEM BUS RADIO - M1 WAV PLAYER (STOCHASTIC DENSITY MODULATION)
// Uses probability to modulate bus density, achieving high-resolution AM.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
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
#define SAMPLE_RATE 24000.0
#define PACKET_MS 100

uint8_t *g_mem;
static mach_timebase_info_data_t timebase_info;
volatile int g_running = 1;

int16_t *g_audio_data = NULL;
size_t g_audio_len = 0; 
double g_sample_rate = SAMPLE_RATE;
double g_density_exp = 0.55;
double g_density_depth = 0.44;
double g_agc_target_env = 0.36;
double g_agc_makeup = 2.6;
double g_agc_max_gain = 96.0;
double g_packet_ms = PACKET_MS;

// Synchronization
volatile size_t g_packet_start_sample = 0;
volatile size_t g_packet_num_samples = 0;

// Custom Barrier
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
    
    // Per-thread sigma-delta state. Stagger the phase across threads.
    double pdm_error = (double)id / (double)NUM_THREADS;

    // Timebase conversion constants
    uint64_t numer = timebase_info.numer;
    uint64_t denom = timebase_info.denom;
    
    // 1サンプルの長さ (mach time)
    uint64_t sample_duration_mach = (uint64_t)((1000000000.0 / g_sample_rate) * (double)denom / (double)numer);

    uint64_t next_sample_time = mach_absolute_time();

    while (1) {
        barrier_wait(&g_barrier_start);
        if (!g_running) break;

        size_t start_idx = g_packet_start_sample;
        size_t num_samples = g_packet_num_samples;
        uint64_t now = mach_absolute_time();
        if (next_sample_time < now) next_sample_time = now;

        for (size_t i = 0; i < num_samples; i++) {
            int16_t sample = g_audio_data[start_idx + i];
            
            // Strong speech-oriented companding to make low-level voice audible.
            double sample_norm = (double)sample / 32768.0; // -1.0 ~ +1.0
            double shaped = copysign(pow(fabs(sample_norm), g_density_exp), sample_norm);
            double density = 0.5 + (shaped * g_density_depth);
            if (density < 0.01) density = 0.01;
            if (density > 0.99) density = 0.99;
            next_sample_time += sample_duration_mach;
            
            // 次のサンプル時刻まで sigma-delta 的に密度を実現する
            while (mach_absolute_time() < next_sample_time) {
                pdm_error += density;
                if (pdm_error >= 1.0) {
                    bus_poke(current);
                    current += 64;
                    if (current >= end_offset) current = offset;
                    pdm_error -= 1.0;
                } else {
                    asm volatile("yield");
                }
            }
        }

        barrier_wait(&g_barrier_end);
    }
    return NULL;
}

static uint16_t read_le16(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_le32(const uint8_t* p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static double get_env_double(const char* name, double default_value) {
    const char* v = getenv(name);
    if (!v || !*v) return default_value;
    char* end = NULL;
    double parsed = strtod(v, &end);
    if (end == v) return default_value;
    return parsed;
}

int load_wav(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) { perror("fopen"); return 0; }

    uint8_t riff_header[12];
    if (fread(riff_header, 1, sizeof(riff_header), fp) != sizeof(riff_header)) {
        fprintf(stderr, "Invalid WAV: header too short\n");
        fclose(fp);
        return 0;
    }
    if (memcmp(riff_header, "RIFF", 4) != 0 || memcmp(riff_header + 8, "WAVE", 4) != 0) {
        fprintf(stderr, "Invalid WAV: missing RIFF/WAVE\n");
        fclose(fp);
        return 0;
    }

    int have_fmt = 0;
    int have_data = 0;
    uint16_t fmt_audio_format = 0;
    uint16_t fmt_channels = 0;
    uint32_t fmt_sample_rate = 0;
    uint16_t fmt_bits_per_sample = 0;
    long data_offset = 0;
    uint32_t data_size = 0;

    while (1) {
        uint8_t chunk_header[8];
        if (fread(chunk_header, 1, sizeof(chunk_header), fp) != sizeof(chunk_header)) break;

        uint32_t chunk_size = read_le32(chunk_header + 4);
        long chunk_data_pos = ftell(fp);
        if (chunk_data_pos < 0) {
            fclose(fp);
            return 0;
        }

        if (memcmp(chunk_header, "fmt ", 4) == 0) {
            if (chunk_size < 16) {
                fprintf(stderr, "Invalid WAV: fmt chunk too short\n");
                fclose(fp);
                return 0;
            }
            uint8_t fmt_buf[16];
            if (fread(fmt_buf, 1, sizeof(fmt_buf), fp) != sizeof(fmt_buf)) {
                fprintf(stderr, "Invalid WAV: truncated fmt chunk\n");
                fclose(fp);
                return 0;
            }
            fmt_audio_format = read_le16(fmt_buf + 0);
            fmt_channels = read_le16(fmt_buf + 2);
            fmt_sample_rate = read_le32(fmt_buf + 4);
            fmt_bits_per_sample = read_le16(fmt_buf + 14);
            have_fmt = 1;
        } else if (memcmp(chunk_header, "data", 4) == 0) {
            data_offset = chunk_data_pos;
            data_size = chunk_size;
            have_data = 1;
        }

        long next_chunk = chunk_data_pos + (long)chunk_size + (chunk_size & 1u);
        if (fseek(fp, next_chunk, SEEK_SET) != 0) break;
    }

    if (!have_fmt || !have_data) {
        fprintf(stderr, "Invalid WAV: missing fmt/data chunk\n");
        fclose(fp);
        return 0;
    }
    if (fmt_audio_format != 1 || fmt_bits_per_sample != 16) {
        fprintf(stderr, "Unsupported WAV: only PCM 16-bit is supported\n");
        fclose(fp);
        return 0;
    }
    if (fmt_channels == 0) {
        fprintf(stderr, "Unsupported WAV: invalid channel count\n");
        fclose(fp);
        return 0;
    }

    if (fseek(fp, data_offset, SEEK_SET) != 0) {
        perror("fseek");
        fclose(fp);
        return 0;
    }

    size_t bytes_per_frame = (size_t)fmt_channels * sizeof(int16_t);
    if (bytes_per_frame == 0 || data_size < bytes_per_frame) {
        fprintf(stderr, "Unsupported WAV: invalid data size\n");
        fclose(fp);
        return 0;
    }

    size_t total_frames = data_size / bytes_per_frame;
    int16_t* raw = (int16_t*)malloc(total_frames * bytes_per_frame);
    g_audio_data = (int16_t*)malloc(total_frames * sizeof(int16_t));
    if (!raw || !g_audio_data) {
        fprintf(stderr, "malloc failed\n");
        fclose(fp);
        free(raw);
        free(g_audio_data);
        g_audio_data = NULL;
        return 0;
    }
    if (fread(raw, bytes_per_frame, total_frames, fp) != total_frames) {
        fprintf(stderr, "Invalid WAV: truncated data chunk\n");
        fclose(fp);
        free(raw);
        free(g_audio_data);
        g_audio_data = NULL;
        return 0;
    }
    fclose(fp);

    // Downmix multi-channel PCM to mono.
    for (size_t i = 0; i < total_frames; i++) {
        int32_t sum = 0;
        for (uint16_t ch = 0; ch < fmt_channels; ch++) {
            sum += raw[i * fmt_channels + ch];
        }
        g_audio_data[i] = (int16_t)(sum / (int32_t)fmt_channels);
    }
    free(raw);
    g_audio_len = total_frames;
    g_sample_rate = (double)fmt_sample_rate;

    // Normalize audio peak safely with 32-bit abs.
    int32_t max_val = 0;
    for (size_t i = 0; i < g_audio_len; i++) {
        int32_t v = g_audio_data[i];
        int32_t a = (v < 0) ? -v : v;
        if (a > max_val) max_val = a;
    }

    if (max_val > 0 && max_val < 32000) {
        double scale = 32000.0 / (double)max_val;
        printf("Normalizing audio (Peak: %d, Scale: %.2f)...\n", max_val, scale);
        for (size_t i = 0; i < g_audio_len; i++) {
            int32_t scaled = (int32_t)lrint((double)g_audio_data[i] * scale);
            if (scaled > 32767) scaled = 32767;
            if (scaled < -32768) scaled = -32768;
            g_audio_data[i] = (int16_t)scaled;
        }
    } else {
        printf("Audio peak is %d (already loud enough or silence).\n", max_val);
    }

    // Speech-focused pre-EQ: DC/high-pass + low-pass + slight presence boost.
    {
        const double hp_fc = 180.0;
        const double lp_fc = 3400.0;
        const double dt = 1.0 / g_sample_rate;
        const double rc_hp = 1.0 / (2.0 * M_PI * hp_fc);
        const double hp_a = rc_hp / (rc_hp + dt);
        const double lp_a = exp(-2.0 * M_PI * lp_fc / g_sample_rate);
        const double presence = 0.35;

        double hp_prev_x = 0.0;
        double hp_prev_y = 0.0;
        double lp_state = 0.0;
        double prev_lp = 0.0;

        for (size_t i = 0; i < g_audio_len; i++) {
            double x = (double)g_audio_data[i] / 32768.0;
            double hp = hp_a * (hp_prev_y + x - hp_prev_x);
            hp_prev_x = x;
            hp_prev_y = hp;

            double lp = (1.0 - lp_a) * hp + lp_a * lp_state;
            lp_state = lp;

            double y = lp + presence * (lp - prev_lp);
            prev_lp = lp;
            if (y > 1.0) y = 1.0;
            if (y < -1.0) y = -1.0;
            g_audio_data[i] = (int16_t)lrint(y * 32767.0);
        }
    }

    // AGC + limiter for low-RMS speech.
    double attack = exp(-1.0 / (g_sample_rate * 0.002));   // 2ms attack
    double release = exp(-1.0 / (g_sample_rate * 0.120));  // 120ms release
    double env = 1e-4;
    const double limiter_drive = 2.8;
    const double limiter_norm = 1.0 / tanh(limiter_drive);

    for (size_t i = 0; i < g_audio_len; i++) {
        double x = (double)g_audio_data[i] / 32768.0;
        double ax = fabs(x);
        double coeff = (ax > env) ? attack : release;
        env = coeff * env + (1.0 - coeff) * ax;

        double gain = g_agc_target_env / (env + 1e-5);
        if (gain < 1.0) gain = 1.0;
        if (gain > g_agc_max_gain) gain = g_agc_max_gain;

        double y = x * gain * g_agc_makeup;
        y = tanh(y * limiter_drive) * limiter_norm;

        int32_t out = (int32_t)lrint(y * 32767.0);
        if (out > 32767) out = 32767;
        if (out < -32768) out = -32768;
        g_audio_data[i] = (int16_t)out;
    }

    printf("Loaded WAV: %u Hz, %u ch, %u bytes, %zu frames\n",
           fmt_sample_rate, fmt_channels, data_size, g_audio_len);
    return 1;
}

int main(int argc, char* argv[]) {
    mach_timebase_info(&timebase_info);
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.wav\n", argv[0]);
        return 1;
    }

    g_density_exp = get_env_double("SBR_DENSITY_EXP", 0.55);
    g_density_depth = get_env_double("SBR_DENSITY_DEPTH", 0.44);
    g_agc_target_env = get_env_double("SBR_AGC_TARGET", 0.36);
    g_agc_makeup = get_env_double("SBR_AGC_MAKEUP", 2.6);
    g_agc_max_gain = get_env_double("SBR_AGC_MAX_GAIN", 96.0);
    g_packet_ms = get_env_double("SBR_PACKET_MS", (double)PACKET_MS);
    if (g_density_exp < 0.05) g_density_exp = 0.05;
    if (g_density_exp > 1.0) g_density_exp = 1.0;
    if (g_density_depth < 0.05) g_density_depth = 0.05;
    if (g_density_depth > 0.495) g_density_depth = 0.495;
    if (g_agc_target_env < 0.05) g_agc_target_env = 0.05;
    if (g_agc_target_env > 0.8) g_agc_target_env = 0.8;
    if (g_agc_makeup < 0.2) g_agc_makeup = 0.2;
    if (g_agc_makeup > 8.0) g_agc_makeup = 8.0;
    if (g_agc_max_gain < 1.0) g_agc_max_gain = 1.0;
    if (g_agc_max_gain > 128.0) g_agc_max_gain = 128.0;
    if (g_packet_ms < 5.0) g_packet_ms = 5.0;
    if (g_packet_ms > 2000.0) g_packet_ms = 2000.0;

    if (!load_wav(argv[1])) return 1;

    g_mem = malloc(MEM_SIZE);
    memset(g_mem, 0xFF, MEM_SIZE);

    barrier_init(&g_barrier_start, NUM_THREADS + 1);
    barrier_init(&g_barrier_end, NUM_THREADS + 1);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker_thread, (void*)(size_t)i);
    }

    set_realtime(0);
    printf("M1 WAV Player (Stochastic Density). Playing...\n");
    printf("Mod params: exp=%.3f depth=%.3f agc_target=%.3f makeup=%.2f max_gain=%.1f packet_ms=%.0f\n",
           g_density_exp, g_density_depth, g_agc_target_env, g_agc_makeup, g_agc_max_gain, g_packet_ms);

    size_t current_idx = 0;
    size_t samples_per_packet = (size_t)(g_sample_rate * g_packet_ms / 1000.0);

    while (current_idx < g_audio_len && g_running) {
        size_t remaining = g_audio_len - current_idx;
        size_t chunk = (remaining > samples_per_packet) ? samples_per_packet : remaining;
        
        g_packet_start_sample = current_idx;
        g_packet_num_samples = chunk;
        
        barrier_wait(&g_barrier_start);
        barrier_wait(&g_barrier_end);
        
        current_idx += chunk;
        printf("\rProgress: %zu / %zu samples", current_idx, g_audio_len);
        fflush(stdout);
    }
    
    printf("\nDone.\n");
    g_running = 0;
    barrier_wait(&g_barrier_start); 
    
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
    free(g_mem);
    free(g_audio_data);
    return 0;
}
