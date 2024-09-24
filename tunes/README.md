# Tune File Format

This file defines the `.tune` music file format.

Following is a simple example of the beginning parts of the *Super Mario Brothers* theme song:

```
100 660
150 0
100 660
300 0
100 660
300 0
100 510
```

## Full specification

1. Simple text file
2. Each line represents a beep or a pause
   1. Column one is a positive integer number of milliseconds
   2. Column two is a positive integer frequency in Hz, or `0` which represents silence
   3. Columns are separated by a space
3. Line ending is unix format
4. File extension is `.tune`
5. Although not necessarily part of the tune, consider adding a silence at the end so that looped playback sounds good :-)

Note: this is compatible with the [GRUB_INIT_TUNE](https://wiki.archlinux.org/title/GRUB/Tips_and_tricks#Play_a_tune) when using a `TEMPO` of 60,000 (bpm).
