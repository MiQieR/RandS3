# RandS3

M5Stack StickS3 Duplication of Rand/0 - The Pocket Prophet

## Features

- **Global Status Bar** – Immersive status bar showing time, WiFi status, and battery, consistent across all screens.
- **Vertical Orientation UX** – 135x240 optimized vertical layout with long-press scrolling gesture support on lengthy texts.
- **Answer Book** (答案之书) – random bilingual answers with Fisher-Yates non-repeating shuffle.
- **MBTI Advice** – browse 16 personality types with growth advice.
- **Lucky Slots** – slot machine game with 3/4/5 reels, built-in IMU screen-rotation, and speaker sound effects.
- **WiFi Provisioning** – AP mode (RandS3, no password), modern web-based WiFi setup with error status reporting.
- **Power Management** - 3-stage adaptive sleep (Dim -> Screen Off -> Light Sleep) configurable via settings.
- **Settings** – language (EN/ZH), MBTI type, WiFi, lock screen timeout, scrolling button config, and dedicated Lucky Slots settings (difficulty, volume).

## Hardware

- ESP32-S3 (8MB Flash, 8MB PSRAM)
- ST7789P3 135×240 LCD (SPI: G39 MOSI, G40 SCK, G45 DC, G41 CS, G21 RST, G38 BL)
- Front button (G11) = Confirm; Side button (G12) = Switch / Long-press = Back
- BMI270 IMU, ES8311 audio codec, IR TX/RX

## Controls

| Action | Button |
|--------|--------|
| Confirm / Select | Front (short press) |
| Next item | Side (short press) |
| Back / Exit | Side (long press) |

## Build & Flash

### Prerequisites

Install [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation.html):

```bash
# macOS
brew install platformio
# or
pip3 install platformio

# Required for esptool merge/flash
pip3 install pyserial
```

### Build

```bash
cd firmware
pio run
```

### Flash via USB

```bash
cd firmware
pio run -t upload
```

### Build merged binary for M5Burner

```bash
cd firmware
./build_and_merge.sh
# → creates M5StickS3_Firmware.bin
```

### Flash merged binary directly (when M5Burner is unavailable)

```bash
cd firmware
# Replace PORT with your serial port, e.g. /dev/tty.usbmodem201201
ESPTOOL=$(find ~/.platformio/packages -name "esptool.py" -type f | head -1)
python3 "$ESPTOOL" \
    --chip esp32s3 \
    --port PORT \
    --baud 921600 \
    --before default_reset \
    --after hard_reset \
    write_flash \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 8MB \
    0x0 M5StickS3_Firmware.bin
```

## Project Structure

```
firmware/
├── platformio.ini          # PlatformIO config
├── build_and_merge.sh      # Build + merge for M5Burner
├── src/
│   ├── lv_conf.h           # LVGL config
│   ├── main.cpp            # Arduino entry (M5Unified + LVGL init)
│   ├── core/
│   │   ├── i18n.c/h        # English + Chinese strings
│   │   ├── settings.c/h    # NVS persistent settings
│   │   ├── answer_data.c/h # Answer book data
│   │   ├── mbti_data.c/h   # 16 MBTI types data
│   │   ├── slot_game.c/h   # Slot machine game logic
│   │   └── wifi_manager.c/h# AP + web server WiFi provisioning
│   └── ui/
│       ├── app_manager.c/h # Screen manager + input dispatch
│       ├── ui_theme.c/h    # Colors, layout constants
│       └── screens/
│           ├── screen_menu.c/h
│           ├── screen_answer.c/h
│           ├── screen_mbti.c/h
│           ├── screen_slots.c/h
│           ├── screen_settings.c/h
│           └── screen_wifi.c/h
├── simulator/              # PC simulator (SDL2)
│   ├── CMakeLists.txt
│   ├── main_sim.c
│   └── test_auto.c
├── components/
│   ├── lvgl/               # LVGL v8.3 source (for ESP-IDF build)
│   └── lv_conf.h           # LVGL config (for ESP-IDF build)
├── main/                   # ESP-IDF main component (alternative build)
M5StickS3_Firmware.bin      # Merged binary (flash with M5Burner or esptool @ 0x0)
```
