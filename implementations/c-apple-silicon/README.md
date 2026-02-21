# System Bus Radio for Apple Silicon (M1/M2/M3)

This implementation is specifically optimized for Apple Silicon (M1/M2/M3) chips, which have strict power management and thermal throttling that can interfere with traditional memory bus noise generation.

## Why a specialized version?

Standard implementations that hammer the memory bus continuously are quickly detected by the macOS power management unit (PMU) as "runaway processes." This results in:
*   Thermal throttling (clock speed reduction).
*   Process prioritization penalties (moving to efficiency cores).
*   Signal loss after a few seconds of playback.

## Solution: Pulse-Packet Modulation

This implementation uses a **"Pulse-Packet" strategy**:
1.  **Burst Transmission:** It blasts the memory bus with 4 parallel threads (utilizing all P-Cores) for short durations (e.g., 20ms).
2.  **Micro-Sleep:** It forces a tiny sleep (e.g., 0.5ms) between bursts.


This intermittent load tricks the PMU into thinking the process is behaving normally, allowing sustained high-power transmission without throttling.

## Hardware Setup (Tested Configuration)

*   **Computer:** MacBook Air (M1, 2020)
*   **Radio:** SONY ICF-B99 (AM Receiver)
*   **Frequency:** ~1100 kHz (1.1 MHz) - *Note: This differs from the original 1580 kHz recommendation for Intel Macs.*
*   **Antenna Position:** Bottom center of the laptop (near the logic board).

## Compilation & Usage

1.  Compile the program:
    ```sh
    gcc -O3 main.c -o main
    ```

2.  Run with a tune file:
    ```sh
    ./main ../../tunes/mary_had_a_little_lamb.tune
    ```

3.  **Important:** For best results:
    *   Keep your Mac plugged into power.
    *   Close other heavy applications.
    *   Tune your AM radio to **~1100 kHz** (explore nearby frequencies for the strongest signal).
