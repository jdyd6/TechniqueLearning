import numpy as np
from PIL import Image

def generate_dark_12bit(save_path="dark_12bit.tif", size=(2048, 2048), base_noise=80, noise_std=15):
    """生成12bit暗场TIFF图，模拟传感器底噪"""
    # 生成高斯噪声底，模拟暗电流热噪声
    dark_data = np.random.normal(loc=base_noise, scale=noise_std, size=size)
    # 限制在12bit范围内，杜绝负数和超量程
    dark_data = np.clip(dark_data, 0, 4095)
    # 转为uint16存储，无损TIFF
    dark_img = Image.fromarray(dark_data.astype(np.uint16), mode="I;16")
    dark_img.save(save_path, format="TIFF", compression=None)
    print(f"暗场图已生成：{save_path}，平均灰度：{dark_data.mean():.1f}")

def generate_light_12bit(save_path="light_12bit.tif", size=(2048, 2048), center_val=3600, edge_val=2800):
    """生成12bit亮场TIFF图，模拟中心高、四周低的平滑光场分布"""
    h, w = size
    # 生成坐标网格
    y, x = np.mgrid[0:h, 0:w]
    # 计算到中心的归一化距离
    cx, cy = w / 2, h / 2
    dist = np.sqrt((x - cx)**2 + (y - cy)**2)
    max_dist = np.sqrt(cx**2 + cy**2)
    norm_dist = dist / max_dist

    # 平滑渐变：中心亮，四周暗，模拟真实LCD光场
    light_data = center_val - (center_val - edge_val) * norm_dist
    # 加入少量随机噪声，模拟真实成像颗粒感
    light_data += np.random.normal(loc=0, scale=12, size=size)
    # 限制12bit量程
    light_data = np.clip(light_data, 0, 4095)

    light_img = Image.fromarray(light_data.astype(np.uint16), mode="I;16")
    light_img.save(save_path, format="TIFF", compression=None)
    print(f"亮场图已生成：{save_path}，中心灰度：{center_val}，边角灰度：{edge_val}")

if __name__ == "__main__":
    # 生成两张标准测试图，分辨率可自行修改
    generate_dark_12bit()
    generate_light_12bit()
    print("\n✅ 两张12bit无损TIFF测试图已生成在当前目录，可直接运行标定代码测试")