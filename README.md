System Bus Radio
==================
This program transmits radio on computers without radio transmitting hardware.

Why?
------------------
Some computers are intentionally disconnected from the rest of the world. This includes having their internet, wireless, bluetooth, USB, external file storage and audio capabilities removed. This is called "air gapping". Even in such a situation, this program can transmit radio.

Publicly available documents already discuss exfiltration from secured systems using various electromagnetic radiations. This is documented in the TEMPEST guidelines published by the US National Security Agency and the US Department of Defense. This project simply adds to that discussion.

How to Use It
------------------
You need a decent C++11 implementation(GCC or Clang)
Compile the program by make:

If you have GCC:

    make gmain

If you have Clang:

    make cmain

If you have both:

    make

And run it on your computer:

For GCC:

    make grun

For Clang:

    make crun


Then use a Sony STR-K670P radio receiver with the included antenna and tune it to 1580 kHz on AM.

You should hear the "Mary Had a Little Lamb" song playing repeatedly. Other equipment and tuning may work as well. On the equipment above, the author has achieved clear transmission over two meters of open air or one meter through drywall. Different results will be achievable with different equipment.

Please see results for other hardware at [HARDWARE-INFO.md] and add your own or mail to sbr@phor.net

Technical Explanation
------------------
Instructions in this program cause electromagnetic radiation to emit from the computer. The emissions are of a broad frequency range. To be accepted by the radio, those frequencies must:

 * Be emitted by the computer processor and other subsystems
 * Escape the computer shielding
 * Pass through the air or other obstructions
 * Be accepted by the antenna
 * Be selected by the receiver

By trial and error, the above frequency was found to be ideal for that equipment. If somebody would like to send me a SDR that is capable of receiving 100 kHz and up then I could test other frequencies.

The actual emissions are caused by the `_mm_stream_si128` instruction that writes through to a memory address. Inspiration for using this instruction was provided in:

> Guri, M., Kachlon, A., Hasson, O., Kedma, G., Mirsky, Y. and Elovici, Y., 2015. GSMem: data exfiltration from air-gapped computers over GSM frequencies. In 24th USENIX Security Symposium (USENIX Security 15) (pp. 849-864).

Please note that replacing `_mm_stream_si128` with a simple `x++;` will work too. My experience has been that  `_mm_stream_si128` produces a stronger signal. There may be other ideas that work even better, and it would be nice to improve this to be more portable (not require SSE extensions).

The program uses square wave modulation, which is depicted below:

```
|<--------------------TIME-------------------->|
|                                              |
|‾|_|‾|_|‾|_____________|‾|_|‾|_|‾|_____________
|                       |   |   |
|<------SIGNAL--------->|   |   |
                            |   |
                            |<->| CARRIER
```

Notes on high precision time APIs for Mac:

* Get current time
  * mach_absolute_time() gives time in int64_t of nanoseconds
    * Converting to nanoseconds https://developer.apple.com/library/mac/qa/qa1398/_index.html
    * Declared https://opensource.apple.com/source/xnu/xnu-1456.1.26/osfmk/mach/mach_time.h
    * Definition https://opensource.apple.com/source/Libc/Libc-320/i386/mach/mach_absolute_time.c
  * clock_get_time() gives a mach_timespec_t time
    * Called from mach_absolute_time()
  * mach_timespec_t
    * Type documentation https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/KernelProgramming/services/services.html
    * Declaration https://opensource.apple.com/source/xnu/xnu-1456.1.26/osfmk/mach/clock_types.h
  * http://stackoverflow.com/a/21352348/300224
  * https://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
* Sleep
  * mach_wait_until()
    * Notes https://developer.apple.com/library/ios/technotes/tn2169/_index.html
  * nanosleep()
    * Apple doc https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man2/nanosleep.2.html
    * Definition https://opensource.apple.com/source/Libc/Libc-320.1.3/gen/nanosleep.c?txt
  * clock_sleep_trap()
    * Used from nanosleep()
    * Declared https://opensource.apple.com/source/xnu/xnu-1456.1.26/osfmk/mach/mach_traps.h
    * Definition http://unix.superglobalmegacorp.com/xnu/newsrc/osfmk/kern/clock.c.html
    * Uses clock_sleep_internal()
    * Uses ADD_MACH_TIMESPEC
  * clock type constants https://opensource.apple.com/source/xnu/xnu-1456.1.26/osfmk/mach/clock_types.h?txt
    * TIME_ABSOLUTE
    * TIME_RELATIVE
    * Defines ADD_MACH_TIMESPEC(t1, t2) // t1  += t2
    * Defines CMP_MACH_TIMESPEC(t1, t2) // t1 <=> t2, also (t1 - t2) in nsec with max of +- 1 sec
  * msleep() https://developer.apple.com/library/mac/documentation/Darwin/Conceptual/KernelProgramming/services/services.html
    * time/timer.c /  http://lxr.free-electrons.com/source/kernel/time/timer.c#L1673
  * kern/clock.h https://opensource.apple.com/source/xnu/xnu-1456.1.26/osfmk/kern/clock.h
