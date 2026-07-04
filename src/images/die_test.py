import cv2
import random
import sys

# ==========================================
#                 配置区域
# ==========================================
IMAGE_PATH = 'image.png'
ORIGINAL_SPRITE_SIZE = 128  # 原图中单个骰子的尺寸 128x128
TARGET_SIZE = 64            # 目标设备分辨率 64x64

# 1. 静止结果帧配置：字典映射 {点数: (行号, 列号)}，索引从 0 开始
STATIC_FRAMES = {
    1: (7, 8),
    2: (15, 4),
    3: (11, 0),
    4: (12, 0),
    5: (7, 4),
    6: (7, 0)
}

# 2. 滚动过程帧配置：列表存放 (行号, 列号)
# 选择处于倾角、非正交状态的帧，以制造剧烈翻滚的视觉假象
ROLLING_FRAMES = [
    (2, 2), 
    (5, 9), 
    (10, 4), 
    (14, 11), 
    (3, 14), 
    (11, 6)
]

# 3. 动画控制参数
ROLL_LOOPS = 3          # 滚动过程帧循环播放的次数
FRAME_DELAY_MS = 66     # 滚动动画单帧停留时间 (毫秒)。66ms 约等于 15 FPS
TARGET_DICE_VALUE = 0   # 最终停留的骰子点数 (1-6)。设为 0 则为随机摇点

# ==========================================
#                 执行逻辑
# ==========================================

def extract_and_resize_frame(image, row, col):
    """根据行列号提取单个骰子并缩小到目标分辨率"""
    y1 = row * ORIGINAL_SPRITE_SIZE
    y2 = y1 + ORIGINAL_SPRITE_SIZE
    x1 = col * ORIGINAL_SPRITE_SIZE
    x2 = x1 + ORIGINAL_SPRITE_SIZE
    
    # 裁剪
    sprite = image[y1:y2, x1:x2]
    # 双线性插值缩放，保留抗锯齿过渡
    resized_sprite = cv2.resize(sprite, (TARGET_SIZE, TARGET_SIZE), interpolation=cv2.INTER_LINEAR)
    return resized_sprite

def main():
    # 读取原始精灵图 (保留 Alpha 通道如果存在的话，OpenCV 中用 IMREAD_UNCHANGED)
    img = cv2.imread(IMAGE_PATH, cv2.IMREAD_UNCHANGED)
    if img is None:
        print(f"错误: 找不到图片文件 '{IMAGE_PATH}'。")
        sys.exit(1)

    print("正在提取并处理帧...")
    
    # 提取滚动帧
    anim_frames = []
    for r, c in ROLLING_FRAMES:
        anim_frames.append(extract_and_resize_frame(img, r, c))
        
    # 提取结果帧
    result_frames = {}
    for val, (r, c) in STATIC_FRAMES.items():
        result_frames[val] = extract_and_resize_frame(img, r, c)

    print("开始模拟摇骰子。按 'q' 退出，按其他任意键重新摇骰子。")
    
    # 创建一个窗口用于模拟设备屏幕
    window_name = "ESP32-S3 Dice Simulation"
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    # 将窗口大小放大一些以便在 PC 上看清 64x64 的效果
    cv2.resizeWindow(window_name, TARGET_SIZE * 4, TARGET_SIZE * 4)

    while True:
        # 1. 播放滚动动画
        for _ in range(ROLL_LOOPS):
            for frame in anim_frames:
                cv2.imshow(window_name, frame)
                # 等待指定延迟，如果期间按下 'q' 则直接退出
                if cv2.waitKey(FRAME_DELAY_MS) & 0xFF == ord('q'):
                    cv2.destroyAllWindows()
                    return

        # 2. 决定最终点数
        if TARGET_DICE_VALUE in STATIC_FRAMES:
            final_val = TARGET_DICE_VALUE
        else:
            final_val = random.randint(1, 6)
            
        print(f"摇出点数: {final_val}")

        # 3. 显示结果帧
        cv2.imshow(window_name, result_frames[final_val])
        
        # 停留在结果帧，等待用户按键
        key = cv2.waitKey(0) & 0xFF
        if key == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()