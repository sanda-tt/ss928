### skills/ss928-max98357-audio-playback

- Goal: preserve the verified flow from user MP3 to SS928 MPP sample-compatible audio, board upload, 40Pin I2S pinmux, and MAX98357 speaker playback.
- Key files: `skills/ss928-max98357-audio-playback/SKILL.md`; `scripts/prepare_audio.py` generates `audio_chn0.aac`, 48k/16bit/stereo PCM, and `play_hint.txt`; UI metadata is in `agents/openai.yaml`.
- Verified path: local `test.mp3` was about 9 seconds at 16 kHz mono. After conversion and upload to `/root/max98357_i2s_test`, running the Pin12 `I2S_BCLK`, Pin38 `I2S_WS`, Pin40 `I2S_SD_TX`, and Pin7 `I2S_MCLK` pinmux commands plus `/opt/sample/audio/sample_audio 2` produced audible output on the MAX98357 speaker.
- Pitfalls: missing ALSA soundcards and missing `aplay` are not blockers for this path. The official sample may print `set inner audio codec ok`; that was present in the successful run. Do not upload MP3 directly to `sample_audio 2`; it expects `audio_chn0.aac` in the current directory.
