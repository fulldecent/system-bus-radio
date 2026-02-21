#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE 24000
#define DURATION_PER_TONE 2 
#define NUM_TONES 5

int frequencies[] = {500, 1000, 2000, 4000, 8000};

int main() {
    FILE *fp = fopen("sweep.wav", "wb");
    if (!fp) return 1;
    
    uint32_t total_samples = SAMPLE_RATE * DURATION_PER_TONE * NUM_TONES;
    uint32_t data_len = total_samples * 2;
    uint32_t riff_len = data_len + 36;
    
    fwrite("RIFF", 1, 4, fp);
    fwrite(&riff_len, 4, 1, fp);
    fwrite("WAVE", 1, 4, fp);
    
    fwrite("fmt ", 1, 4, fp);
    uint32_t fmt_len = 16;
    uint16_t audio_fmt = 1; 
    uint16_t num_channels = 1;
    uint32_t sample_rate = SAMPLE_RATE;
    uint32_t byte_rate = SAMPLE_RATE * 2;
    uint16_t block_align = 2;
    uint16_t bits_per_sample = 16;
    
    fwrite(&fmt_len, 4, 1, fp);
    fwrite(&audio_fmt, 2, 1, fp);
    fwrite(&num_channels, 2, 1, fp);
    fwrite(&sample_rate, 4, 1, fp);
    fwrite(&byte_rate, 4, 1, fp);
    fwrite(&block_align, 2, 1, fp);
    fwrite(&bits_per_sample, 2, 1, fp);
    
    fwrite("data", 1, 4, fp);
    fwrite(&data_len, 4, 1, fp);
    
    for (int t = 0; t < NUM_TONES; t++) {
        int freq = frequencies[t];
        printf("Generating %d Hz...\n", freq);
        for (int i = 0; i < SAMPLE_RATE * DURATION_PER_TONE; i++) {
            double time = (double)i / SAMPLE_RATE;
            double v = sin(2.0 * M_PI * freq * time);
            int16_t sample = (int16_t)(v * 32000.0);
            fwrite(&sample, 2, 1, fp);
        }
    }
    
    fclose(fp);
    printf("Generated sweep.wav (500, 1000, 2000, 4000, 8000 Hz)\n");
    return 0;
}
