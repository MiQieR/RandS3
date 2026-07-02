#!/bin/bash
# build_and_merge.sh - Build firmware and create merged binary for M5Burner
#
# Usage:
#   ./build_and_merge.sh          # build + merge
#   ./build_and_merge.sh flash    # build + flash via USB
#
set -e
cd "$(dirname "$0")"

ENV_NAME="m5stack-sticks3"
BUILD_DIR=".pio/build/${ENV_NAME}"
OUTPUT="M5StickS3_Firmware.bin"

echo "=== Building PlatformIO project ==="
pio run

echo ""
echo "=== Merging binaries for M5Burner ==="
# Find esptool from PlatformIO packages
ESPTOOL=$(find ~/.platformio/packages -name "esptool.py" -type f 2>/dev/null | head -1)
if [ -z "$ESPTOOL" ]; then
    echo "ERROR: esptool.py not found in PlatformIO packages"
    exit 1
fi

python3 "$ESPTOOL" --chip esp32s3 merge_bin \
    -o "$OUTPUT" \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size 8MB \
    0x0000  "${BUILD_DIR}/bootloader.bin" \
    0x8000  "${BUILD_DIR}/partitions.bin" \
    0x10000 "${BUILD_DIR}/firmware.bin"

echo ""
echo "=== Done ==="
echo "Merged binary: $(pwd)/${OUTPUT}"
echo "Flash via M5Burner, or run: ./build_and_merge.sh flash"

# Optional: flash directly
if [ "$1" = "flash" ]; then
    echo ""
    echo "=== Flashing via USB ==="
    pio run -t upload
fi
