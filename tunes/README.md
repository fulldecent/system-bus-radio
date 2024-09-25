# Tune File Format

This file defines the `.tune` music file format.

Following is a simple example of the beginning parts of the *Super Mario Brothers* theme song:

```
660 100
0 150
660 100
0 300
660 100
0 300
510 100
```

## Full specification

1. Each line in the file is `<frequency> <duration>`
   1. `<frequency>` is the frequency of the beep in Hz or `0` for silence
   2. `<duration>` is the duration of the beep in milliseconds
2. Line ending is unix format
3. File extension is `.tune`
4. Although not necessarily part of the tune, consider adding a silence at the end so that looped playback sounds good :-)

Note: this is compatible with the [GRUB_INIT_TUNE](https://wiki.archlinux.org/title/GRUB/Tips_and_tricks#Play_a_tune) when using a `TEMPO` of 60,000 (bpm).
