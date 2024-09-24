# RTL STR Basic Setup

## Test subject

For this guide, we are inspecting a **MacBook Pro M1 (13-inch, 2020)** for electromagnetic radiation.

This subject is tough because:

- Low power (wattage)
- System is tightly integrated (no user-replaceable RAM)
- Aluminum casing

If we are successful with the approaches in this paper, you should get even better results with other kinds of laptops.

## Hardware

*Other setups will work too. But this guide is explored with the following hardware:*

* Computer running macOS (to run SDR)
* RTL-SDR Blog V3 / [buy from manufacturer](https://www.rtl-sdr.com/buy-rtl-sdr-dvb-t-dongles/) / [data sheet](https://www.rtl-sdr.com/wp-content/uploads/2018/02/RTL-SDR-Blog-V3-Datasheet.pdf)
* Ham It Up Plus / [buy from manufacturer](https://nooelec.com/store/ham-it-up-plus.html) / [data sheet](https://www.nooelec.com/store/downloads/dl/file/id/99/product/284/ham_it_up_plus_datasheet_revision_2.pdf)
  * Requires a USB A/B cable and a USB power source (do not use the same device running your SDR)
* Balun One Nine / [buy from manufacturer](https://www.nooelec.com/store/balun-one-nine-v2.html) / could not find data sheet
* An AM loop antenna
* Affiliate link to buy everything / [from Amazon](https://www.amazon.com/ideas/amzn1.account.AHUITP6B2VTROJ7IMNP2LCUA5QDA/18H46X17FDG76?type=explore&_encoding=UTF8&tag=phornetandrel-20&linkCode=ur2&linkId=dedd255a129c5ac7415a9dcb713ae618&camp=1789&creative=9325)

You may have seen that the RTL-SDR Blog V3 above [already includes support for lower frequencies](https://www.rtl-sdr.com/rtl-sdr-blog-v-3-dongles-user-guide/), obviating the upconverter (Ham It Up Plus). General advice on the scene has been that you want to use an upconverter rather than the built-in bias tee. If I can get better results with the bias tee approach I will update this guide to recommend that simpler and more ecoromical approach.

## Setup

Your equipment is plugged in as:

```
Computer --M/M-plug-- RTL-SDR --M/M-plug-- Ham It Up Plus --optional-long-wire-- Balun One Nine --spring-terminal-- antenna
```

![RF test setup](https://user-images.githubusercontent.com/382183/115773678-98d16a80-a37e-11eb-9f4b-4dabb9d401a2.jpg)

Note that the upconverter means you will be tuning your radio to +125 MHz offset versus the frequencies you want.

## Software

**Test subject**

For quick browsing and playing, I used the [counter and threads](https://github.com/fulldecent/system-bus-radio/tree/master/Using%20counter%20and%20threads) implementation running on the M1 test subject. This allowed me to quickly edit the code and rerun it at different frequencies. (I ran the compiler command by hand as the Makefile did not work on the M1. Not sure if this needs fixing.)

**Radio**

This was an 2018 MacBook Pro (Intel). [CubicSDR](https://cubicsdr.com) was easy to set up. Also it claimed to let me try the bias tee approach, but I failed to make it work.

Alternately, I tried running [RTL Power](http://kmkeen.com/rtl-power/) to sweep various frequencies. Try it like this with the test subject off:

```sh
time rtl_power -f 125M:126M:20K -g 50 -i 1m -1 noise.csv; say done
```

And then run it again with `signal.csv`. And compare those two results.

Here is a quick Swift program to convert from RTL power into something you can use in Excel:

```swift
import Foundation
while let line = readLine() {
    //	date, time, Hz low, Hz high, Hz step, samples, dbm, dbm, ...
    let columns = line.components(separatedBy: ", ")

    let hzLow = Double(columns[2])!
    let hzStep = Double(columns[4])!
    var hzCurrent = hzLow
    for dbm in columns[6...] {
        print(Int(hzCurrent), dbm)
        hzCurrent += hzStep
    }
}
```

And of course:

```sh
paste -d, noise-transpose.csv signal-transpose.csv > merged.csv
```

Now you can plot the signal-to-noise ratio.

## Results

The antenna placed directly over the bottom left of the keyboard produces the best signal. Start here so you can clearly hear the signal and as your tuning improves, begin backing away the antenna.

I could clearly hear the signal between 63 kHz and 5.5 MHz using bandwidths between 10 kHz and 50 kHz. The best signal was 1.52 MHz at 40 kHz bandwidth. Using the AM demodulator.

With this approach I achieved audible signals up to only several inches away from the M1 Mac. Not very useful for spying/exfiltration. Possible ways to improve that are below.

## Further research

*These are a little more advanced idea I'd like to try with a mentor. If you'd like to pick up the ball and explain how to improve with these techniques, please feel free to open a new wiki page, and ping @fulldecent in a new issue.*

* It may be possible to improve the signal by adding a low-noise amplifier between the antenna and the upconverter. I'd like to test the signal levels around these compenents and read specs before considering that further.
* I could not get GNU Radio to connect to the RTL-SDR V3. It could certainly create a better custom receiver for this project:
  1. Wideband 2.5MHz (or, somehow, twin 5.0MHz) signal input
  2. A 1,000-band equalizer based on the envelope above (see RTL Power above)
  3. Demodulate with AM
  4. Tight bandpass at 440 Hz (for a 440 Hz system bus signal)
* Better hardware. If you can recommend a better RTL-SDR, antenna, LNA, upconverter that might help, please let me know and I can add to my [project tip jar](https://github.com/fulldecent/system-bus-radio#system-bus-radio).