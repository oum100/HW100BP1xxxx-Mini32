---
name: platformio-washer-check
description: Safely inspect, build, and review this ESP32/PlatformIO washer-controller repository. Use when asked to check the environment, validate a change, run a PlatformIO build, review MQTT/payment/hardware-sensitive code, or prepare a safe development handoff. Never upload firmware, erase NVS, change live hardware, or expose credentials.
---

# PlatformIO Washer Check

Use this skill for read-only validation and code review of `HW100BP1XXXX-Mini32`.

## Workflow

1. Establish scope:
   - Run `git status -sb`, `git branch --show-current`, and `git diff --stat`.
   - Read the repository `AGENTS.md` and `MQTT_PROTOCOL.md` when present.
   - Preserve existing user edits; do not stage, reset, checkout, or delete files.
2. Check environment:
   - Prefer `/Users/teerin/.platformio/penv/bin/platformio`; fall back to `pio` only if available.
   - Run `platformio --version` and inspect `platformio.ini`.
   - Confirm the active environment, board, framework, upload port, and monitor speed.
3. Verify source selection:
   - Read `include/startup.h` and the selected model header.
   - Confirm exactly one machine model is active and report variant-dependent GPIO definitions.
   - Treat `src/` as the current implementation; root `.ino` files are legacy snapshots.
4. Build without hardware mutation:
   - Run `platformio run` for the active environment.
   - Run `platformio test -e native` when the native test environment is present.
   - Report pass/fail, compiler warnings, RAM/flash use, and the exact command used.
   - Do not run `platformio run -t upload`, `erase`, monitor commands, or NVS operations unless the user explicitly requests and authorizes them.
5. Review risk areas:
   - Payment amount/product matching and duplicate commands.
   - MQTT topic, payload, reconnect, acknowledgement, and credential handling.
   - State transitions, timers, reboot recovery, and offline behavior.
   - GPIO polarity, relay safety, door-lock checks, and model macro collisions.
   - OTA, secrets, hard-coded Wi-Fi, API keys, and serial logging.
6. Report findings:
   - Classify each finding as `blocker`, `high`, `medium`, or `low`.
   - Include file/line evidence, impact, and a minimal next action.
   - Separate confirmed failures from inferred risks and from items requiring physical bench testing.

## Safe defaults

- Never print or copy credential values. Redact keys, passwords, tokens, and Wi-Fi values.
- Do not change the selected board model or GPIO map during a review.
- Do not assume a successful compile proves relay polarity, payment correctness, or safe machine operation.
- Treat native unit tests as software-only checks; they do not validate GPIO electrical behavior or washer programs.
- Do not make network/API calls beyond what the build or explicitly requested read-only inspection requires.
- If a requested change affects payment, OTA, NVS, MQTT authorization, relays, or machine power, stop before execution if scope or authorization is unclear.

## Expected output

Return a compact validation record:

```text
Scope: <branch and changed files>
Environment: <PlatformIO / board / framework>
Build: PASS|FAIL (+ memory and warnings)
Findings: <severity, file:line, impact, next action>
Bench tests still required: <physical checks>
Mutations performed: none unless explicitly authorized
```

For protocol-specific details, use the repository `MQTT_PROTOCOL.md` as the source of truth and note any discrepancy between that document and the implementation.
