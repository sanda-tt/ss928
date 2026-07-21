# Hunch reminder, haptics, audio, and CloudBase history - 2026-07-22

## Goal

After a cumulative 15 seconds of hunching, allow brief good-posture gaps while
the wearer is otherwise still hunched, then provide a five-second light haptic
reminder, play `bad.mp3`, and retain the ten newest reminder records in the
real-time posture page.

## Final implementation

- `posture_reminder.py` uses a 20-second rolling window and triggers once after
  15 seconds of valid hunch samples; up to five seconds of good posture in that
  window does not cancel the reminder.
- BMI270 writes an atomic `/tmp/smartbag_posture_reminder.json` handoff and
  submits `posture_hunch_reminder` through the existing device-ingest route.
- The alert controller accepts only a fresh handoff (30 seconds), schedules a
  gentle effect-7 pulse every 0.5 seconds for five seconds on both TM6605s, and
  plays `/root/smartbag_alert/audio/bad/audio_chn0.aac`.
- The Mini Program real-time posture page filters `alarm_history` for this type
  and renders at most ten entries.

## Verification

- Local: 20 BMI/posture tests, CloudBase protocol tests, Python syntax checks,
  and a 15-second-with-four-second-good-gap rolling-window check passed.
- Deployed: both CloudBase functions were updated; the board services are
  active; a live handoff was accepted by the alert controller.
- Cloud proof: `alarm_history` contains `hunch-live-verify-20260722` with
  `durationS: 5`, `audioClip: bad`, and the Event Function returned it.

## Limits

- Controller logs prove the haptic/audio commands were accepted; audible sound
  and human-perceived vibration still require a wearer/listener confirmation.
- The Mini Program source is updated but was not uploaded/published.
