import os
import re

files_to_read = [
    'src/core/i18n.c',
    'src/core/answer_data.c',
    'src/core/mbti_data.c'
]

chars = set()

# Add basic english and symbols
basic_chars = set(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")
for c in basic_chars:
    chars.add(c)

for fpath in files_to_read:
    if os.path.exists(fpath):
        with open(fpath, 'r', encoding='utf-8') as f:
            text = f.read()
            for char in text:
                # Basic check for printable or Chinese
                if ord(char) > 31 and char not in ['\n', '\r', '\t']:
                    chars.add(char)

# Extra common characters just in case
extra = "设置返回退出老虎机答案之书建议语言连接中电量旋转停止按键右侧确认侧边长按下一项难度简单中等困难轮盘硬币长按向下滚动长按BtnAB"
for c in extra:
    chars.add(c)

sorted_chars = "".join(sorted(list(chars)))
with open('scripts/chars.txt', 'w', encoding='utf-8') as f:
    f.write(sorted_chars)

print(f"Extracted {len(sorted_chars)} unique characters.")
