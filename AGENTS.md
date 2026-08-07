# AGENTS.md

## Project purpose

เฟิร์มแวร์ ESP32 สำหรับเปลี่ยนเครื่องซักผ้า Haier ให้เป็นเครื่องบริการเชิงพาณิชย์ รับชำระด้วยเหรียญหรือ Payboard สั่งเริ่มโปรแกรม ติดตามเวลาคงเหลือ และบริหารจากระบบส่วนกลางผ่าน MQTT/HTTP

## Current baseline

- Platform: ESP32, Arduino framework, PlatformIO
- PlatformIO environment: `wemos_d1_mini32`
- Active machine: `HW100BP14826`
- Active board variant: `HW100BP14826ALLNEW_101x` (บอร์ดสีม่วง)
- Active indicator: RGB LED (`USE_RGBLED`)
- Shadow Payboard reporting is enabled (`SHADOWPAYBOARD`)
- Source firmware version: `1.0.12` in `src/config.cpp`
- Stored release artifact: `bin/HW100BP10829-1.0.1.bin`; its metadata is older than the source
- Custom partition table exists but is disabled in `platformio.ini`
- Last local build verification: successful on 2026-08-07
- Build size at latest verification: RAM 53,668/327,680 bytes (16.4%), flash 1,060,373/1,310,720 bytes (80.9%)

The project also contains support code for `HW100BP10829` and `HW150BP14896`, but they are not selected in the current build.

## Important files

- `src/main.cpp`: application lifecycle, MQTT callback, payment flow, timers, state machine, setup/loop
- `include/startup.h`: compile-time feature flags and selected machine model
- `include/HW100BP14826.h`, `src/HW100BP14826.cpp`: active model GPIO map and washer controls
- `include/hw10010829.h`, `src/hw10010829.cpp`: legacy/alternate 10 kg model
- `include/HW150BP14896.h`, `src/HW150BP14896.cpp`: alternate 15 kg model
- `include/config.h`, `src/config.cpp`: configuration structures, defaults, NVS Preferences access
- `include/payboardAPI.h`, `src/payboardAPI.cpp`: Payboard HTTP registration and transaction APIs
- `include/shadowPbAPI.h`, `src/shadowPbAPI.cpp`: shadow transaction reporting
- `include/esp32fota.h`, `src/esp32fota.cpp`: HTTPS OTA updater
- `include/Timer.h`, `src/Timer.cpp`: application timers
- `include/animation.h`, display source files: seven-segment/RGB display support
- `platformio.ini`: board, upload and library settings
- `payboard.json`: old MQTT protocol notes/example; treat as reference, not authoritative spec
- `MQTT_PROTOCOL.md`: canonical MQTT topics, connection parameters, command/response contract and current compatibility notes
- `.agents/skills/platformio-washer-check/SKILL.md`: repository-local read-only environment/build/review workflow
- Root `.ino` files and `interrupt.*`: legacy snapshots; normal PlatformIO build uses `src/`

## Runtime flow

1. Configure GPIO, interrupt queue, RGB/display, and coin input.
2. Connect to built-in and NVS-configured Wi-Fi networks.
3. Load defaults, then override them from ESP32 Preferences namespace `config`.
4. Register the device by MAC address if UUID is missing.
5. Connect and subscribe to Payboard MQTT.
6. Restore `stateflag` and `timeremain` after restart when possible.
7. Run the main state machine:
   - `1`: not configured/registration flow
   - `2`: registered but awaiting activation
   - `3`: Available; coin acceptor enabled
   - `4`: Booked; accumulating payment and waiting for the selected price
   - `5`: Busy; program running and time remaining tracked
   - `6`: failed to start job
   - `10`: Offline
8. On service completion, stop timers, notify the backend, clear transaction state, and return to state `3`.

## Payment and product behavior

- Up to three products are stored as `sku`, `price`, `stime`, and `unit`.
- Default products are P1/P2/P3 with prices 30/40/50 and nominal times 35/67/97 minutes; NVS or MQTT configuration may override them.
- Coin modules:
  - `SINGLE`: one pulse is treated as 10 currency units.
  - `MULTI`: one pulse is treated as 1 currency unit.
- `coinwaittimeout` delays starting after a valid price, allowing the customer to add more money and select a higher tier.
- Remote payment arrives through MQTT action `paid`; manual/backend jobs can arrive through `jobcreate`.
- `paymentby`: 1 coin, 2 QR, 3 kiosk, 4 free.

## Network interfaces

- MQTT publish: `payboard/backend/<merchantid>/<uuid>`
- MQTT subscribe: `payboard/<merchantid>/<uuid>`
- Common incoming actions include `config`, `paid`, `ping`, `reset/reboot`, `ota`, `setwifi`, `coinmodule`, `coinwaittimeout`, `spin`, `rinsespin`, `selfclean`, `startpause`, `turnon`, `turnoff`, `jobcancel`, `resetstate`, `jobcreate`, `offline`, and `online`.
- HTTP is used for device registration, coin transaction reporting, and device start/stop acknowledgement.
- MQTT state reports expose Available, Booked, Busy, or Offline plus RSSI, firmware and remaining time.

## Active HW100BP14826 board notes

- Power relay GPIO 26
- Start relay GPIO 18
- Temperature relay GPIO 32
- Rinse relay GPIO 27
- Speed relay GPIO 15
- Program selection outputs GPIO 22, 21, 33, 14
- Coin enable GPIO 4; coin input GPIO 35
- Door lock input GPIO 23
- Machine power sensing GPIO 19 on the selected all-new board
- Water relay GPIO 25 on the selected all-new board
- RGB/status LED GPIO 2 on the selected all-new board
- Seven-segment CLK/DIO GPIO 17/16

Confirm the physical board revision and wiring before changing model flags or GPIO definitions. Several pins change meaning between variants.

## Build and verification

Preferred commands:

```sh
/Users/teerin/.platformio/penv/bin/platformio run
/Users/teerin/.platformio/penv/bin/platformio run -t upload
/Users/teerin/.platformio/penv/bin/platformio device monitor -b 115200
```

`pio` is not currently on the shell PATH, so use the absolute executable above unless the environment is activated. Upload port is currently fixed to `/dev/cu.usbserial-10`; verify the actual device before upload. Never upload, erase NVS, or change a running machine without explicit user approval.

The current target builds successfully. A clean build has historically emitted warnings for missing return paths in `Timer`, backend and Payboard API functions, duplicate GPIO macro definitions in `hw10010829.h`, deprecated external LittleFS, and FastLED falling back to bit-banged SPI. Treat new warnings as regressions and reduce the existing set deliberately.

## Development rules

- Treat `src/main.cpp` as production-sensitive: payment, relays, machine state and backend reporting are tightly coupled.
- Preserve unrelated local edits. At the time this file was created, `include/startup.h` and `include/HW100BP14826.h` already had uncommitted changes.
- Do not use the legacy `.ino` files as the current implementation unless explicitly requested.
- Keep model-specific GPIO and machine behavior inside the model files; avoid adding more model conditionals to `main.cpp` where practical.
- Persist any state required after power loss before energizing the machine.
- Validate payment amount against a configured product; reject unknown prices instead of selecting a default program.
- Avoid long blocking delays or reconnect loops because they can prevent MQTT processing and service timers.
- When changing MQTT JSON, preserve compatibility or version the protocol explicitly.
- Treat `MQTT_PROTOCOL.md` as the primary communication contract and update it in the same change as MQTT code.
- Update source version, OTA metadata and binary metadata together for a release.
- Add a bench test checklist for relay polarity, power sensing, door lock sensing, coin debounce, interrupted jobs and backend outage before field deployment.

## Codex working workflow

For substantive changes, use this sequence:

1. `/plan`: define scope, risks, files and validation before editing.
2. Implement the smallest scoped change while preserving unrelated edits.
3. `/diff`: inspect the complete diff, credentials, model macros, MQTT payloads and GPIO changes.
4. `/review`: run an independent review and resolve blocker/high findings.
5. Run the repository Skill `platformio-washer-check` and `platformio run`.
6. Commit with an intentional message; upload firmware only after explicit hardware authorization.

The repository Skill is the only project-local Skill currently defined. Add another Skill only when a repeated workflow cannot be expressed clearly in the existing one.

## Security and known technical debt

- The repository contains hard-coded Wi-Fi credentials, merchant/API credentials, backend credentials and device admin credentials. Do not repeat them in documentation, logs, commits or responses. They should be rotated and moved to provisioning/NVS or build-time secrets.
- Configuration logging can print credentials to Serial; redact this before production diagnostics.
- MQTT currently uses port 1883 without TLS.
- HTTP/API error paths and JSON validation are inconsistent; some functions may exit without an explicit return value.
- State and timer logic is spread across callbacks and `loop()`, making edge cases difficult to verify.
- Several model/variant macro names are inconsistent (`ALLNEW`, `ALLNEW_101x`, and model-number spelling), so build every supported target after refactoring.
- There is no substantive automated test suite; `test/README` is only a placeholder.
- Wi-Fi and MQTT failure handling commonly restarts the controller; repeated outages may cause reboot loops.

## Recommended continuation priorities

1. Record a hardware bench baseline for the current build and resolve compiler warnings.
2. Rotate/remove committed secrets and introduce secure device provisioning.
3. Extract an explicit, testable state machine for payment and washer lifecycle.
4. Separate board profiles and create one PlatformIO environment per supported model/revision.
5. Add offline transaction queuing, bounded reconnect behavior and watchdog-safe non-blocking tasks.
6. Add unit/bench tests and CI builds for every target.
7. Establish a single release version and OTA manifest process.
