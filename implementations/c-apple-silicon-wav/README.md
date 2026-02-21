# c-apple-silicon-wav

This is a **delta implementation** based on the Apple Silicon version in
`implementations/c-apple-silicon`, extended to play arbitrary WAV files.

For hardware assumptions and receiver setup (such as AM radio placement),
see `implementations/c-apple-silicon/README.md`.
This README summarizes **only the differences**.

## Changes from c-apple-silicon

1. Input format
- Changed input from `.tune` to WAV
- Run format: `./main input.wav`

2. WAV loader
- Added `RIFF/WAVE` validation
- Scans and reads `fmt ` / `data` chunks
- Handles chunk padding (even-byte alignment)
- Downmixes multi-channel audio to mono
- Applies input sample rate to playback timing

3. Audio preprocessing
- 32-bit-safe peak normalization
- Voice-oriented pre-EQ (HP/LP + presence)
- Compression and loudness boost via AGC + limiter

4. Modulation method
- Replaced random-sampling-based density generation with a Sigma-Delta
  (error-accumulation) method
- Improves intelligibility by reducing fine-grained random on/off artifacts

5. Packet handling
- Keeps sample time continuity across packets
- Sets default `PACKET_MS` to `100ms`
- Adjustable at runtime via `SBR_PACKET_MS`


## Supported WAV

- `RIFF/WAVE`
- PCM 16-bit (`audio_format=1`, `bits_per_sample=16`)
- 1 channel or more (internally downmixed to mono)
- Any sample rate (tracked by internal timing)

Not supported:
- float PCM
- 24/32-bit PCM
- WAV with compressed codecs

## Build

```bash
cd /Users/cho45/tmp/system-bus-radio/implementations/c-apple-silicon-wav
make
```

## Run

```bash
./main input.wav
```

Examples:

```bash
./main hello.wav
./main sweep.wav
```

## Main runtime parameters

- `SBR_DENSITY_EXP`: Exponent of the modulation curve
- `SBR_DENSITY_DEPTH`: Modulation depth
- `SBR_AGC_TARGET`: Target envelope level for AGC
- `SBR_AGC_MAKEUP`: Makeup gain after AGC
- `SBR_AGC_MAX_GAIN`: Maximum AGC gain
- `SBR_PACKET_MS`: Packet length (ms)

## Tuning guidelines

- Still too quiet: increase `SBR_AGC_TARGET`, `SBR_AGC_MAKEUP`,
  and `SBR_AGC_MAX_GAIN`
- Choppy output: increase `SBR_PACKET_MS` (for example `200` to `500`)
- Distortion: decrease `SBR_AGC_MAKEUP` or `SBR_DENSITY_DEPTH`
