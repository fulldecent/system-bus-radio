// SYSTEM BUS RADIO
// https://github.com/fulldecent/system-bus-radio
// Copyright 2016 William Entriken
// C++11 port by Ryou Ezoe

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <atomic>

void square_am_signal(float time, float frequency)
{
    using namespace std::chrono ;
    
    std::cout << "Playing / " << time << " seconds / " << frequency << " Hz\n" ;

    seconds const sec{1} ;
    nanoseconds const nsec{ sec } ;
    using rep = nanoseconds::rep ;
    auto nsec_per_sec = nsec.count() ;

    nanoseconds const period( static_cast<rep>( nsec_per_sec / frequency) ) ;

    auto start = high_resolution_clock::now() ;
    auto const end = start + nanoseconds( static_cast<rep>(time * nsec_per_sec) ) ;

    while (high_resolution_clock::now() < end)
    {
        auto mid = start + period / 2 ;
        auto reset = start + period ;
        while (high_resolution_clock::now() < mid)
        {
            std::atomic<unsigned> x{0} ;
            ++x ; 
        }
        std::this_thread::sleep_until( reset ) ;
        start = reset;
    }
}

int main()
{
    while (1)
    {
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
