---
name: ss928-max98357-audio-playback
description: Use when the user provides an MP3 or other audio file to play through the MAX98357 I2S amplifier on the SS928/HiEuler Pi board, asks to prepare audio_chn0.aac for the board sample_audio player, tests 40Pin I2S speaker output, or mentions MAX98357 audio playback.
---

# SS928 MAX98357 Audio Playback

## Overview

Convert user audio into the exact file shape accepted by the SS928 MPP audio sample, then play it on the wired MAX98357 speaker path over 40Pin I2S.

## Required Pairing

If the task includes board connection, upload, or playback, first read `skills/ss928-direct-board-debug/SKILL.md` and use its SSH/SFTP workflow. This skill owns the MAX98357 audio specifics; direct-board-debug owns safe board access.

## Validated Hardware Facts

- Board target: `root@192.168.1.168`, password as defined in `ss928-direct-board-debug`.
- MAX98357 module is I2S digital input, not analog input. Do not route this flow through the round `J5` analog audio jack.
- 40Pin I2S mapping:
  - Pin12 -> `I2S_BCLK` -> MAX98357 `BCLK`
  - Pin38 -> `I2S_WS` -> MAX98357 `LRC`
  - Pin40 -> `I2S_SD_TX` -> MAX98357 `DIN`
  - Pin7 -> `I2S_MCLK`, not required by MAX98357
- Power: MAX98357 `VIN` can use 5V for louder output or 3.3V for lower power. `GND` must share board ground.
- Speaker connects only to `OUT1` and `OUT2`; do not tie either speaker terminal to GND.

## Workflow

1. Prepare the board-playable audio folder from the user-supplied audio file:

```powershell
python skills/ss928-max98357-audio-playback/scripts/prepare_audio.py "C:\path\to\input.mp3" --out-dir work/max98357_i2s_test
```

This creates:

- `audio_chn0.aac`: the filename `/opt/sample/audio/sample_audio 2` expects in its current directory.
- `<stem>_48k_s16le_stereo.pcm`: raw PCM backup for custom AO tests.
- `play_hint.txt`: duration-derived sleep/timeout values for the sample player.

2. Upload the folder:

```powershell
python skills/ss928-direct-board-debug/scripts/board_debug.py upload --local C:\Users\ASUS\Desktop\ss928\work\max98357_i2s_test --remote /root/max98357_i2s_test
```

3. Set I2S pinmux and play:

```powershell
$cmd = "cd /root/max98357_i2s_test || exit 1; bspmm 0x102F010C 0x1202; bspmm 0x102F0108 0x1102; bspmm 0x102F0104 0x1202; bspmm 0x102F0110 0x1102; { sleep 11; printf '\n\n'; } | timeout 20 /opt/sample/audio/sample_audio 2"
python skills/ss928-direct-board-debug/scripts/board_debug.py run $cmd --timeout 35
```

For longer files, use the `sleep_seconds` and `timeout_seconds` shown in `play_hint.txt`.

## Expected Output

The player normally prints:

```text
set inner audio codec ok: sample_rate = 48000.
open stream file:"audio_chn0.aac" for adec ok
bind adec:0 to ao(0,0) ok
please press twice ENTER to exit this sample
```

This message was observed during the validated MAX98357 test and does not mean the run failed. The user confirmed audible speaker output with this exact flow.

## Common Mistakes

- Do not debug missing ALSA (`--- no soundcards ---`, no `aplay`) as a blocker. This board path uses the MPP sample, not normal Linux ALSA playback.
- Do not upload an arbitrary `test.mp3` and expect `sample_audio 2` to decode it. Convert to `audio_chn0.aac` first.
- Do not use `I2S_SD_RX`; playback uses board TX: `I2S_SD_TX` on Pin40.
- Do not stop after the command exits successfully if the user says there was no sound. Recheck wiring, `SD` enable, `VIN/GND`, speaker on `OUT1/OUT2`, and the 40Pin pinmux commands.
