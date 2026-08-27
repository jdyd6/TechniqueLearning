import numpy as np


# 一条窗口包含连续采样点，每个采样点包含三个轴。
window = np.array(
    [
        [0.1, 0.0, 1.0],
        [0.2, 0.1, 0.9],
        [0.0, -0.1, 1.1],
        [-0.1, 0.0, 1.0],
    ],
    dtype=np.float32,
)

print("window:")
print(window)
print("shape:", window.shape)
print("dtype:", window.dtype)
print("第一个采样点:", window[0])
print("全部 X 轴:", window[:, 0])
print("每个轴的平均值:", window.mean(axis=0))

# 模型一次接收一批窗口，最前面的维度表示样本数量。
batch = np.stack([window, window * 2], axis=0)

print("batch shape:", batch.shape)
print("样本数量:", batch.shape[0])
print("窗口长度:", batch.shape[1])
print("通道数量:", batch.shape[2])