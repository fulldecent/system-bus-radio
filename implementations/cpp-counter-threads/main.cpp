// SYSTEM BUS RADIO
// https://github.com/fulldecent/system-bus-radio
// Copyright 2016 William Entriken
// C++11 port by Ryou Ezoe

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstdio>  // For FILE, fopen, fgets, etc.
#include <cstdlib> // For exit()

std::mutex m;
std::condition_variable cv;
std::chrono::high_resolution_clock::time_point mid;
std::chrono::high_resolution_clock::time_point reset;

void boost_song() {
    using namespace std::chrono;
    while (true) {
        std::unique_lock<std::mutex> lk{m};
        cv.wait(lk);

        std::atomic<unsigned> x{0};
        while (high_resolution_clock::now() < mid) {
            ++x;
        }
        std::this_thread::sleep_until(reset);
    }
}

void square_am_signal(float time, float frequency) {
    using namespace std::chrono;
    std::cout << "Playing / " << time << " seconds / " << frequency << " Hz\n";

    seconds const sec{1};
    nanoseconds const nsec{sec};
    using rep = nanoseconds::rep;
    auto nsec_per_sec = nsec.count();

    nanoseconds const period(static_cast<rep>(nsec_per_sec / frequency));

    auto start = high_resolution_clock::now();
    auto const end = start + nanoseconds(static_cast<rep>(time * nsec_per_sec));

    while (high_resolution_clock::now() < end) {
        mid = start + period / 2;
        reset = start + period;

        cv.notify_all();
        std::this_thread::sleep_until(reset);
        start = reset;
    }
}

int main(int argc, char* argv[]) {
    // Launch as many threads as supported by hardware
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        std::thread t(boost_song);
        t.detach();
    }

    // File reading logic
    FILE* fp;
    if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("Error opening file");
            exit(1);
        }
    } else {
        std::cerr << "No song file given!\nUsage: " << argv[0] << " file.song\n";
        exit(1);
    }

    char buffer[64] = {0}; // Buffer for reading lines from file
    int time_ms;
    int freq_hz;

    while (true) {
        if (fgets(buffer, sizeof(buffer) - 1, fp)) {
            if (sscanf(buffer, "%d %d", &time_ms, &freq_hz) == 2) {
                square_am_signal(1.0 * time_ms / 1000, freq_hz); // Convert ms to seconds
            }
        }

        // Handle end of file and rewind for looping the song
        if (feof(fp)) {
            rewind(fp);
        }
    }

    fclose(fp); // Close the file (though unreachable in this infinite loop)
    return 0;
}