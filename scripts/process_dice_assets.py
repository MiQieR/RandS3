import os
import cv2
import sys
import numpy as np

# Configuration
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMAGE_PATH = os.path.join(PROJECT_ROOT, '../Dice', '360die2048transparent.png')
AUDIO_PATH = os.path.join(PROJECT_ROOT, '../Dice', 'dice-sound.wav')
OUTPUT_IMG_DIR = os.path.join(PROJECT_ROOT, 'src', 'images')
OUTPUT_WAV_H = os.path.join(PROJECT_ROOT, 'src', 'core', 'dice_wav.h')

ORIGINAL_SPRITE_SIZE = 128
TARGET_SIZE = 64

STATIC_FRAMES = {
    1: (7, 8),
    2: (15, 4),
    3: (11, 0),
    4: (3, 0),
    5: (7, 4),
    6: (7, 0)
}

ROLLING_FRAMES = [
    (8, 1), (12, 9), (2, 10), (12, 5), (15,11),
    (3, 3), (12, 3), (8, 15), (11, 9), (7, 6),
    (4, 5), (0, 13), (2, 10)
]

def extract_and_save_frame(img, row, col, out_name):
    y1 = row * ORIGINAL_SPRITE_SIZE
    y2 = y1 + ORIGINAL_SPRITE_SIZE
    x1 = col * ORIGINAL_SPRITE_SIZE
    x2 = x1 + ORIGINAL_SPRITE_SIZE
    
    sprite = img[y1:y2, x1:x2]
    # Resize with antialiasing
    resized = cv2.resize(sprite, (TARGET_SIZE, TARGET_SIZE), interpolation=cv2.INTER_LINEAR)
    
    out_path = os.path.join(OUTPUT_IMG_DIR, out_name)
    cv2.imwrite(out_path, resized)
    print(f"Saved {out_path}")

def process_images():
    if not os.path.exists(IMAGE_PATH):
        print(f"Error: Cannot read {IMAGE_PATH}")
        sys.exit(1)
    img = cv2.imread(IMAGE_PATH, cv2.IMREAD_UNCHANGED)
    if img is None:
        print(f"Error: cv2.imread failed for {IMAGE_PATH}")
        sys.exit(1)
        
    for i, (r, c) in enumerate(ROLLING_FRAMES):
        extract_and_save_frame(img, r, c, f"dice_anim_{i}.png")
        
    for val, (r, c) in STATIC_FRAMES.items():
        extract_and_save_frame(img, r, c, f"dice_res_{val}.png")

def process_audio():
    if not os.path.exists(AUDIO_PATH):
        print(f"Error: Cannot read {AUDIO_PATH}")
        sys.exit(1)
        
    with open(AUDIO_PATH, 'rb') as f:
        data = f.read()
        
    hex_data = ', '.join([f'0x{b:02x}' for b in data])
    header_content = f"""#pragma once
#include <stdint.h>

const uint8_t dice_wav[] = {{
    {hex_data}
}};
const uint32_t dice_wav_len = {len(data)};
"""
    with open(OUTPUT_WAV_H, 'w') as f:
        f.write(header_content)
    print(f"Saved {OUTPUT_WAV_H} ({len(data)} bytes)")

if __name__ == '__main__':
    process_images()
    process_audio()
