# TinyML 零基础手把手教程：从第一个模型到 Renode

> 面向读者：熟悉 STM32 和 FreeRTOS，但没有 Machine Learning（机器学习）与 TinyML 基础。
>
> 配套简版路线：[tinyml-minimum-learning-loop.md](./tinyml-minimum-learning-loop.md)
>
> 信息核查日期：2026-08-27。本文不会覆盖或替换上一版文档。

---

## 0. 先说清楚：这份教程怎么用

这不是一篇要求一次读完的“大百科”。正确用法是：

1. 从第 1 课开始。
2. 先理解“本课只需要知道什么”。
3. 在 PowerShell 中逐条执行命令。
4. 对照“你应该看到什么”。
5. 完成本课检查点后再继续。
6. 遇到错误就停在当前课，不跨层排查。

整个过程分成三个里程碑：

```text
里程碑 A：PC 上理解并训练模型
    ↓
里程碑 B：得到可验证的 Full INT8 模型
    ↓
里程碑 C：模型进入 STM32 固件并在 Renode 中通过测试
```

首轮应用仍然是“离线 IMU 人体活动识别”：

- 输入：一段三轴加速度数据。
- 输出：走路、坐下、站立等活动类别。
- 训练发生在 PC。
- MCU 只执行 inference（推理），不训练模型。
- 没有硬件时，传感器数据由 Flash 中的测试向量代替。

### 0.1 当前电脑的实际状态

已经检查到：

```text
Python 3.12.10                    已安装
Git 2.54.0                       已安装
Renode 1.16.1                    已安装
CMake                            尚未加入 PATH
Ninja                            尚未加入 PATH
arm-none-eabi-gcc                尚未加入 PATH
WSL                              尚未安装
```

现在**不要急着安装全部工具**。前半段只需要 Python 和 Git。等模型量化成功后，再进入固件工具链，避免一上来就和交叉编译器“深情对视”。

### 0.2 最终会产生什么

完成教程后，项目中应该出现：

```text
TinyML/
├─ .venv/                       Python 独立环境
├─ data/                        原始数据与处理后数据
├─ lessons/                     入门练习
├─ model/                       训练、量化和检查脚本
├─ artifacts/                   模型与 Golden Test
├─ firmware/                    STM32 + FreeRTOS + TFLM 固件
├─ renode/                      Renode 与 Robot 测试脚本
└─ reports/                     准确率、混淆矩阵和内存报告
```

### 0.3 学完之前可以不懂什么

首轮不要求：

- 推导反向传播公式。
- 掌握矩阵微积分。
- 理解复杂 CNN（Convolutional Neural Network，卷积神经网络）。
- 学会 PyTorch、Edge Impulse 或 Zephyr。
- 优化 CMSIS-NN。
- 测量真实功耗。
- 追求论文级准确率。

这些内容并非不重要，只是它们不应该挡住第一次闭环。

---

# 第一部分：先建立 TinyML 的正确直觉

## 第 1 课：TinyML 到底是什么

### 1.1 一句话定义

TinyML 是让受限设备执行 Machine Learning inference 的工程方法。

这里有三个关键词：

- **受限设备**：RAM、Flash、算力和功耗有限的 MCU。
- **Machine Learning**：算法中的部分规则不由人手写，而是从数据中学习。
- **inference**：使用已经训练好的模型，根据新输入产生输出。

### 1.2 它和普通 STM32 程序有什么区别

传统算法通常由人写规则：

```c
if (temperature > threshold) {
    alarm_on();
}
```

Machine Learning 模型则从大量样本中学习参数：

```text
输入数据 → 带有大量参数的计算函数 → 分类结果
```

部署到 MCU 后，模型本质上仍是一段确定性计算：

```text
输入 tensor
  → 乘法、加法、激活函数
  → 输出 tensor
```

区别在于，计算中的权重参数由训练得到，而不是工程师逐个填写。

你可以把模型类比为：

- `.tflite` 文件：一份包含计算图和参数的数据包。
- TFLM：解释并执行这份数据包的嵌入式 Runtime。
- Tensor Arena：TFLM 的静态工作内存。
- inference task：调用模型的 FreeRTOS 任务。

### 1.3 训练和推理必须分开理解

训练阶段：

```text
大量已标注数据
  → 多次计算
  → 比较预测与正确答案
  → 调整模型参数
  → 得到模型
```

推理阶段：

```text
一条新数据
  → 固定模型
  → 得到预测
```

本项目中：

- PC 负责训练。
- STM32 负责推理。
- Renode 模拟 STM32 执行推理固件。

### 1.4 第一次接触的核心术语

#### sample（样本）

模型的一条输入。在活动识别中，一条样本不是一个加速度值，而是一段连续时间窗口。

#### feature（特征）

模型用于判断的输入信息。本项目中，原始特征是每个时刻的 X、Y、Z 三轴加速度。

#### label（标签）

样本的正确答案，例如 `WALKING`。

#### model（模型）

从输入映射到输出的数学函数，以及函数中的参数。

#### parameter / weight（参数 / 权重）

训练时被调整的数字。部署时它们会随模型进入 Flash。

#### tensor（张量）

多维数组。别被名字吓到：

```text
标量          一个数
向量          一维数组
矩阵          二维数组
tensor        维度更多的数组
```

一条 IMU 窗口可以表示为：

```text
shape = (128, 3)
```

含义：

- 有 128 个采样时刻。
- 每个时刻有 X、Y、Z 三个值。

如果一次训练送入 32 条窗口，shape 会变为：

```text
(32, 128, 3)
```

最前面的 32 称为 batch dimension（批次维度）。

### 1.5 本课检查点

如果下面三句话都能解释，就可以继续：

- 训练为什么放在 PC，而不是 STM32。
- 一条 IMU 样本为什么是一段窗口，而不是单个采样点。
- `.tflite` 模型和 TFLM Runtime 分别扮演什么角色。

**小测试：模型部署到 STM32 后，设备每收到一条新数据，还会继续修改权重吗？**

---

## 第 2 课：建立 Python 独立环境

### 2.1 为什么需要虚拟环境

Python package（包）有不同版本。直接安装到系统 Python，项目之间容易互相污染。

virtual environment（虚拟环境）相当于项目自己的 Python 工具箱：

```text
系统 Python
├─ 项目 A 的 .venv
└─ TinyML 的 .venv
```

删除 `.venv` 不会删除代码和数据，只会删除这个项目安装的 Python 包。

### 2.2 打开正确的终端

在 Cursor 中打开 PowerShell，确认当前位置：

```powershell
Get-Location
```

预期路径：

```text
E:\Codes\TinyML
```

如果不是，执行：

```powershell
Set-Location E:\Codes\TinyML
```

### 2.3 创建虚拟环境

执行：

```powershell
python -m venv .venv
```

这条命令通常没有输出。没有输出不代表失败，Python 在安静地干活。

检查目录：

```powershell
Test-Path .venv
```

预期：

```text
True
```

### 2.4 激活虚拟环境

执行：

```powershell
.\.venv\Scripts\Activate.ps1
```

终端提示符前面应出现：

```text
(.venv)
```

如果 PowerShell 阻止脚本执行，只为当前终端临时放宽：

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\.venv\Scripts\Activate.ps1
```

这里使用 `Process` scope，关闭终端后设置自动失效。

### 2.5 检查使用的是哪个 Python

执行：

```powershell
python -c "import sys; print(sys.executable)"
```

输出路径中应该包含：

```text
E:\Codes\TinyML\.venv
```

如果仍指向系统 Python，说明虚拟环境没有激活。

### 2.6 安装首轮需要的包

先升级安装工具：

```powershell
python -m pip install --upgrade pip
```

再安装固定版本：

```powershell
python -m pip install tensorflow==2.21.0 numpy matplotlib scikit-learn pytest
```

各包作用：

- `tensorflow`：定义、训练并转换模型。
- `numpy`：处理多维数组。
- `matplotlib`：绘制信号和混淆矩阵。
- `scikit-learn`：计算指标和划分数据。
- `pytest`：执行自动化单元测试。

Windows Native 上 TensorFlow 2.21 使用 CPU 即可。本教程的数据和模型很小，不需要 GPU。TensorFlow 2.10 之后不再支持 Windows Native GPU，但这不影响当前路线。

### 2.7 验证安装

执行：

```powershell
python -c "import tensorflow as tf; import numpy as np; print('TensorFlow:', tf.__version__); print('NumPy:', np.__version__)"
```

TensorFlow 应显示：

```text
2.21.0
```

启动时可能出现 CPU 优化提示，它通常不是错误。判断失败要看末尾是否出现 traceback（异常堆栈）。

### 2.8 记录依赖

执行：

```powershell
python -m pip freeze > requirements-lock.txt
```

以后恢复环境可以执行：

```powershell
python -m pip install -r requirements-lock.txt
```

### 2.9 常见错误

#### `python` 找不到

当前电脑已经安装 Python；如果新终端找不到，重启 Cursor，让 PATH 重新加载。

#### 激活后没有 `(.venv)`

执行：

```powershell
Get-Command python
```

检查 Source 是否指向 `.venv\Scripts\python.exe`。

#### TensorFlow import 失败

先确认：

```powershell
python --version
python -m pip show tensorflow
```

不要使用单独的 `pip` 命令排查，因为它可能属于另一个 Python。始终使用 `python -m pip`。

### 2.10 本课检查点

- [x] 提示符包含 `(.venv)`。
- [x] `sys.executable` 指向项目 `.venv`。
- [x] Tent`。
      sorFlow 能成功 import。
- [x] 已生成 `requirements-lock.tx

**小测试：为什么本教程坚持使用 `python -m pip`，而不是直接使用 `pip`？**

---

## 第 3 课：用 NumPy 看懂 tensor 和 shape

这一课不训练模型，只学习模型输入到底长什么样。

### 3.1 创建练习目录和脚本

创建目录：

```powershell
New-Item -ItemType Directory -Force lessons
```

新建 `lessons/01_tensor_basics.py`，内容如下：

```python
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
```

运行：

```powershell
python lessons/01_tensor_basics.py
```

### 3.2 逐行理解

`dtype=np.float32` 表示每个元素使用 32-bit floating point（32 位浮点数）。训练阶段通常使用它。

`window.shape` 应为：

```text
(4, 3)
```

`window[0]` 取第一个采样时刻：

```text
[0.1, 0.0, 1.0]
```

`window[:, 0]` 表示：

- `:`：取全部行。
- `0`：取第一列。

因此它取得整段窗口的 X 轴。

`mean(axis=0)` 表示沿时间方向汇总，得到每个轴的平均值。

### 3.3 增加 batch dimension

在脚本末尾加入：

```python
# 模型一次接收一批窗口，最前面的维度表示样本数量。
batch = np.stack([window, window * 2], axis=0)

print("batch shape:", batch.shape)
print("样本数量:", batch.shape[0])
print("窗口长度:", batch.shape[1])
print("通道数量:", batch.shape[2])
```

重新运行。

预期 shape：

```text
(2, 4, 3)
```

### 3.4 为什么 shape 如此重要

模型输入 shape、训练数据 shape 和固件写入 tensor 的元素数量必须一致。

常见错误：

```text
训练模型要求 (128, 3)
固件却只写入 128 个值
```

这意味着固件只提供了三分之一的数据。模型不会替你脑补另外两个轴。

### 3.5 本课检查点

- [ ] 能解释 `(batch, time, channel)` 三个维度。
- [x] 能用 `window[:, 1]` 取得全部 Y 轴。
- [x] 理解 Float32 是元素类型，不是 shape。

**小测试：shape 为 `(10, 128, 3)` 的 tensor 中，共有多少条窗口，每条窗口有多少个数？**

---

## 第 4 课：亲手训练第一个极小模型

真实数据先放一放。本课用人工生成的“静止”和“周期运动”信号，让你完整看到：

```text
数据 → 模型 → 训练 → 评估 → 预测
```

### 4.1 先理解分类任务

我们定义两个类别：

```text
0 = 静止
1 = 周期运动
```

每条样本是一段三轴窗口。静止信号接近常量，周期运动信号包含波动。

这不是实际产品模型，它只负责把 Machine Learning 的黑箱掀开一条缝。

### 4.2 创建训练脚本

新建 `lessons/02_first_model.py`：

```python
from pathlib import Path

import numpy as np
import tensorflow as tf


SEED = 42
SAMPLE_COUNT = 1200
WINDOW_SIZE = 32
CHANNEL_COUNT = 3

np.random.seed(SEED)
tf.random.set_seed(SEED)


def create_rest_sample() -> np.ndarray:
    """生成接近静止状态的三轴窗口。"""
    base = np.array([0.0, 0.0, 1.0], dtype=np.float32)
    noise = np.random.normal(
        loc=0.0,
        scale=0.03,
        size=(WINDOW_SIZE, CHANNEL_COUNT),
    )
    return (base + noise).astype(np.float32)


def create_motion_sample() -> np.ndarray:
    """生成带有周期变化的三轴窗口。"""
    time_axis = np.linspace(0.0, 2.0 * np.pi, WINDOW_SIZE, dtype=np.float32)
    phase = np.random.uniform(0.0, 2.0 * np.pi)
    signal = np.stack(
        [
            0.7 * np.sin(time_axis + phase),
            0.4 * np.sin(2.0 * time_axis + phase),
            1.0 + 0.3 * np.cos(time_axis + phase),
        ],
        axis=1,
    )
    noise = np.random.normal(
        loc=0.0,
        scale=0.05,
        size=(WINDOW_SIZE, CHANNEL_COUNT),
    )
    return (signal + noise).astype(np.float32)


def create_dataset() -> tuple[np.ndarray, np.ndarray]:
    """生成平衡的二分类数据集。"""
    samples = []
    labels = []

    for index in range(SAMPLE_COUNT):
        is_motion = index % 2 == 1
        sample = create_motion_sample() if is_motion else create_rest_sample()
        samples.append(sample)
        labels.append(int(is_motion))

    features = np.asarray(samples, dtype=np.float32)
    targets = np.asarray(labels, dtype=np.int32)

    order = np.random.permutation(len(features))
    return features[order], targets[order]


features, targets = create_dataset()

train_end = int(len(features) * 0.7)
validation_end = int(len(features) * 0.85)

x_train = features[:train_end]
y_train = targets[:train_end]
x_validation = features[train_end:validation_end]
y_validation = targets[train_end:validation_end]
x_test = features[validation_end:]
y_test = targets[validation_end:]

model = tf.keras.Sequential(
    [
        tf.keras.layers.Input(shape=(WINDOW_SIZE, CHANNEL_COUNT)),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(16, activation="relu"),
        tf.keras.layers.Dense(2, activation="softmax"),
    ]
)

model.compile(
    optimizer="adam",
    loss="sparse_categorical_crossentropy",
    metrics=["accuracy"],
)

model.summary()

model.fit(
    x_train,
    y_train,
    validation_data=(x_validation, y_validation),
    epochs=12,
    batch_size=32,
    verbose=2,
)

test_loss, test_accuracy = model.evaluate(x_test, y_test, verbose=0)
print(f"test loss: {test_loss:.4f}")
print(f"test accuracy: {test_accuracy:.4f}")

prediction = model.predict(x_test[:1], verbose=0)[0]
print("prediction probabilities:", prediction)
print("predicted class:", int(np.argmax(prediction)))
print("expected class:", int(y_test[0]))

artifact_dir = Path("artifacts")
artifact_dir.mkdir(exist_ok=True)
model.save(artifact_dir / "first_model.keras")
```

运行：

```powershell
python lessons/02_first_model.py
```

### 4.3 你应该看到什么

首先是模型结构：

```text
Flatten
Dense
Dense
```

随后每个 epoch 会输出：

```text
accuracy
loss
val_accuracy
val_loss
```

最后测试准确率通常应该很高，因为这批人工数据刻意设计得容易区分。具体小数可能不同，不要追求与文档逐位一致。

还会生成：

```text
artifacts/first_model.keras
```

### 4.4 把训练术语翻译成人话

#### layer（层）

模型中的一段计算。

`Flatten` 把二维窗口摊平成一维：

```text
(32, 3) → (96,)
```

`Dense` 表示每个输出都连接所有输入，也称 Fully Connected（全连接层）。

#### activation（激活函数）

如果模型只有线性运算，多层叠加仍然只是线性函数。`ReLU` 引入非线性：

```text
ReLU(x) = max(0, x)
```

#### softmax

把最终输出变成各类别的相对概率。两个输出的和接近 1。

#### loss（损失）

预测与正确答案之间的误差。训练的目标是降低 loss。

#### optimizer（优化器）

根据 loss 调整模型参数的方法。此处使用 Adam。

#### epoch

模型完整看完**一次训练集**。

#### batch

一次送入模型的一**小组样本**。它与 FreeRTOS batch 没关系，只是训练术语。

#### accuracy（准确率）

分类正确的样本数除以样本总数。

### 4.5 你暂时不必推导的部分

你现在只需要知道：

```text
预测错误
  → loss 变大
  → optimizer 调整权重
  → 下一轮预测可能更好
```

权重如何通过微积分更新，等首个闭环完成后再学更高效。

### 4.6 常见错误

#### `ModuleNotFoundError`

虚拟环境可能没有激活：

```powershell
.\.venv\Scripts\Activate.ps1
python -m pip show tensorflow
```

#### 训练很慢

先确认模型没有被误改得很大。这个模型在 CPU 上也应较快完成。

#### 准确率不完全等于 1

这是正常的。模型指标是统计结果，不是编译器的 PASS/FAIL。

### 4.7 本课检查点

- [x] 模型能够训练完成。
- [x] 能解释 epoch、batch、loss 和 accuracy。
- [ ] 能说出 `Flatten` 和 `Dense` 做了什么。
- [ ] 能在 `artifacts/` 找到 `.keras` 文件。

**小测试：训练 accuracy 很高，但 test accuracy 很低，通常说明模型可能发生了什么？**

---

# 第二部分：学会处理真实数据

## 第 5 课：训练集、验证集和测试集

### 5.1 为什么不能用同一批数据完成所有工作

如果让模型在背过的题目上考试，它可能得高分，但不能证明它理解了规律。

三类数据的职责：

- **train set（训练集）**：用于调整模型权重。
- **validation set（验证集）**：用于选择模型结构和观察 overfitting（过拟合）。
- **test set（测试集）**：最后一次评估泛化能力。

测试集不能反复用于调参，否则它会逐渐变成验证集。

### 5.2 什么是过拟合

模型记住训练数据中的细节，却没有学到可推广的规律。

典型现象：

```text
train accuracy 持续提高
validation accuracy 停止提高甚至下降
```

在 TinyML 中，模型不是越大越好：

- 大模型更容易过拟合。
- 大模型占用更多 Flash。
- 中间 tensor 可能占用更多 RAM。
- 推理计算量通常更大。

### 5.3 IMU 数据为什么不能随便随机打散

假设同一个人在一分钟内连续走路。相邻窗口非常相似。

如果把相邻窗口随机分到训练集和测试集：

```text
训练集看到这个人的第一个窗口
测试集看到同一个人的下一个窗口
```

测试结果会过分乐观。这称为 data leakage（数据泄漏）。

本项目按 subject（受试者）划分：

```text
训练受试者
验证受试者
测试受试者
```

这样测试的是“对没见过的人是否有效”。

### 5.4 什么是窗口化

MCU 连续收到：

```text
t0: x, y, z
t1: x, y, z
t2: x, y, z
...
```

模型需要固定形状，所以把连续数据切成窗口：

```text
窗口 A：采样点 0 到 127
窗口 B：采样点 64 到 191
```

第二个窗口从中间开始，称为 overlap（重叠）。

窗口长度的影响：

- 太短：看不到完整动作模式。
- 太长：响应慢、输入大、参数多。

首轮沿用 UCI 数据常见设置：

- 采样率：50 Hz。
- 窗口长度：128 个采样点。
- 重叠：50%。

这对应约 2.56 秒数据。

### 5.5 normalization（归一化）

训练数据不同通道的范围可能差异很大。归一化让数值尺度更稳定。

常见标准化：

```text
normalized = (value - mean) / standard_deviation
```

关键规则：

- `mean` 和 `standard_deviation` 只能从训练集计算。
- 验证集、测试集和设备端必须使用同一组参数。

如果设备端预处理与训练端不同，模型再聪明也会被喂错饭。

### 5.6 混淆矩阵

总体准确率无法告诉你“错在哪里”。

混淆矩阵的行通常表示真实类别，列表示预测类别：

```text
真实 SITTING 经常被预测为 STANDING
```

这比“准确率不够”更有行动价值，因为它提示：

- 两类信号本来就相似。
- 可能需要保留重力方向。
- 可能需要陀螺仪通道。
- 标签或窗口边界可能有问题。

### 5.7 本课检查点

- [ ] 能解释 train、validation、test 的不同职责。
- [ ] 理解为什么按受试者划分。
- [ ] 能解释窗口长度和重叠。
- [ ] 知道归一化参数只能从训练集计算。

**小测试：如果先用全部数据计算 mean，再划分测试集，为什么这也属于轻微的数据泄漏？**

---

## 第 6 课：认识并下载真实 UCI HAR 数据

### 6.1 数据集包含什么

使用官方数据集：

<https://archive.ics.uci.edu/dataset/341/smartphone+based+recognition+of+human+activities+and+postural+transitions>

原始数据包含：

- 三轴加速度。
- 三轴角速度。
- 实验编号。
- 受试者编号。
- 活动标签及起止采样点。

首轮只使用三轴加速度，并只保留六类稳定活动：

```text
WALKING
WALKING_UPSTAIRS
WALKING_DOWNSTAIRS
SITTING
STANDING
LAYING
```

姿态转换类暂时排除，因为它们持续时间短、边界更难处理。

### 6.2 创建数据目录

执行：

```powershell
New-Item -ItemType Directory -Force data\raw
New-Item -ItemType Directory -Force data\processed
```

### 6.3 下载官方 ZIP

执行：

```powershell
$datasetUrl = "https://archive.ics.uci.edu/static/public/341/smartphone+based+recognition+of+human+activities+and+postural+transitions.zip"
Invoke-WebRequest -Uri $datasetUrl -OutFile data\raw\ucihar.zip
```

检查文件：

```powershell
Get-Item data\raw\ucihar.zip
```

再解压：

```powershell
Expand-Archive -Path data\raw\ucihar.zip -DestinationPath data\raw\ucihar -Force
```

检查：

```powershell
Get-ChildItem data\raw\ucihar -Recurse -Filter labels.txt
```

应该能找到 `RawData/labels.txt`。

### 6.4 先不要训练，先打开 README

执行：

```powershell
Get-ChildItem data\raw\ucihar -Recurse -Filter README.txt
```

打开找到的 README，重点确认：

- 加速度文件命名规则。
- 标签文件各列含义。
- 采样频率。
- activity ID 与名称的映射。

先理解数据格式，再写 parser（解析器）。靠猜文件格式训练出来的不是 AI，是随机数生成器的远房亲戚。

### 6.5 parser 应该完成什么

后续 `model/prepare_data.py` 需要：

1. 读取 `labels.txt`。
2. 根据 experiment ID 找到对应加速度文件。
3. 根据标签中的起止位置切出活动片段。
4. 只保留六类稳定活动。
5. 按受试者分组。
6. 把活动片段切成 `(128, 3)` 窗口。
7. 保存：
   - `x_train.npy`
   - `y_train.npy`
   - `x_validation.npy`
   - `y_validation.npy`
   - `x_test.npy`
   - `y_test.npy`
   - `metadata.json`

### 6.6 窗口函数长什么样

核心逻辑可以先单独理解：

```python
import numpy as np


def make_windows(
    signal: np.ndarray,
    window_size: int,
    step_size: int,
) -> np.ndarray:
    """把连续多轴信号切分为固定长度窗口。"""
    windows = []

    for start in range(0, len(signal) - window_size + 1, step_size):
        end = start + window_size
        windows.append(signal[start:end])

    return np.asarray(windows, dtype=np.float32)
```

当：

```text
window_size = 128
step_size = 64
```

就得到 50% 重叠。

### 6.7 数据处理后的强制检查

处理脚本必须主动检查：

```python
assert x_train.ndim == 3
assert x_train.shape[1:] == (128, 3)
assert x_train.dtype == np.float32
assert len(x_train) == len(y_train)
assert not np.isnan(x_train).any()
```

还应打印每个集合：

- 样本数量。
- shape。
- dtype。
- 每类样本数。
- 受试者列表。
- 最小值、最大值、平均值和标准差。

### 6.8 本课检查点

- [ ] 官方 ZIP 已下载并解压。
- [ ] 找到了原始加速度文件和 `labels.txt`。
- [ ] 能解释标签文件为什么需要起止采样点。
- [ ] 理解窗口函数输出 shape。

**小测试：一个活动片段只有 100 个采样点，窗口长度为 128 时，当前窗口函数会生成几条窗口？**

---

## 第 7 课：训练真实 Float32 基线

这一课才开始训练真实应用。

### 7.1 什么叫 baseline（基线）

baseline 是第一个足够简单、可重复的参考结果。

它的作用不是赢比赛，而是回答：

- 数据管道是否正确？
- 模型是否确实能学到东西？
- 后续量化损失了多少？
- 后续优化有没有真实收益？

没有 baseline 就直接优化，等于不知道起点却开始计算进步幅度。

### 7.2 首轮模型为什么使用 MLP

MLP（Multilayer Perceptron，多层感知机）在这里由：

```text
Flatten
Dense + ReLU
Dense + Softmax
```

组成。

选择原因：

- 概念简单。
- TFLM 算子少。
- 内存关系容易理解。
- CMSIS-NN 支持 Fully Connected。
- 便于先验证部署链路。

它不一定是活动识别的最佳模型。首轮最重要的是可部署、可解释、可调试。

### 7.3 训练脚本应该按什么顺序工作

`model/train.py` 建议严格按下面顺序：

```text
加载处理后数据
  → 检查 shape 和 dtype
  → 只用训练集计算 normalization 参数
  → 对三个集合应用相同 normalization
  → 创建模型
  → compile
  → fit
  → evaluate
  → 生成混淆矩阵
  → 保存模型和预处理参数
```

### 7.4 一个足够小的模型

```python
import tensorflow as tf


def create_model(window_size: int, channel_count: int, class_count: int):
    """创建便于 MCU 部署的活动分类模型。"""
    return tf.keras.Sequential(
        [
            tf.keras.layers.Input(shape=(window_size, channel_count)),
            tf.keras.layers.Flatten(),
            tf.keras.layers.Dense(32, activation="relu"),
            tf.keras.layers.Dense(class_count, activation="softmax"),
        ]
    )
```

模型 summary 中最值得关注的是 `Param #`：

- 参数越多，模型权重通常越大。
- INT8 权重通常每个参数约占一个 byte。
- 这只是模型主体的粗略直觉，不等于最终 `.tflite` 文件大小。

### 7.5 如何读训练曲线

重点观察四个值：

```text
loss
accuracy
val_loss
val_accuracy
```

常见情况：

#### train 和 validation 都很差

可能原因：

- 数据或标签解析错误。
- 归一化错误。
- 模型太小。
- 输入本身不足以区分类别。

#### train 很好，validation 很差

可能过拟合，也可能有数据分布差异。

#### accuracy 很高，但某些类几乎全错

可能类别不平衡。必须查看混淆矩阵和每类指标。

### 7.6 首轮不要设死准确率目标

第一次真实训练的通过条件是：

- loss 确实下降。
- 结果明显优于随机猜测。
- 混淆矩阵能够解释。
- 重复运行结果基本稳定。
- 模型足够小，可以继续部署。

六分类随机猜测准确率约为 `1 / 6`。如果模型只比随机略好，不应马上加大网络，先检查数据。

### 7.7 应保存哪些产物

```text
artifacts/activity_float.keras
artifacts/normalization.json
reports/training_history.png
reports/confusion_matrix_float.png
reports/metrics_float.json
```

`normalization.json` 必须与模型一起版本化，因为设备端输入必须使用同样的预处理规则。

### 7.8 本课检查点

- [ ] 真实模型训练完成。
- [ ] 有独立 test 结果。
- [ ] 有混淆矩阵。
- [ ] 保存了 normalization 参数。
- [ ] 能指出最容易混淆的类别。

**小测试：如果 SITTING 和 STANDING 经常混淆，为什么“再训练更多 epoch”不一定能解决？**

---

# 第三部分：从 Float32 进入 TinyML

## 第 8 课：理解 Full INT8 量化

### 8.1 为什么要量化

训练时使用 Float32：

- 表达范围大。
- 训练工具支持好。
- 每个值占 4 byte。

MCU 推理更偏好 INT8：

- 每个值占 1 byte。
- 模型通常更小。
- Cortex-M 上整数 kernel 更容易优化。
- 可使用 CMSIS-NN 等优化实现。

量化不是简单地把小数强转为整数。必须保存实数与整数之间的映射。

### 8.2 scale 和 zero-point

量化关系：

```text
real_value ≈ scale × (quantized_value - zero_point)
```

把实数转成 INT8：

```text
quantized_value = round(real_value / scale) + zero_point
```

最后把结果限制在：

```text
[-128, 127]
```

其中：

- `scale` 决定相邻整数间隔代表多大实数变化。
- `zero_point` 表示实数零对应哪个整数。

设备端不能自己猜这两个值。它们存放在模型输入输出 tensor 的量化参数中。

### 8.3 什么是 representative dataset

Post-Training Quantization（训练后量化）需要观察一小部分具有代表性的输入，估算 activation（激活值）的范围。

这批数据：

- 应来自训练集。
- 应覆盖主要类别和典型范围。
- 不应用 test set。
- 不需要包含全部训练数据。

如果 representative dataset 只包含静止数据，量化器可能无法正确覆盖剧烈运动范围。

### 8.4 Full INT8 与只量化权重的区别

只量化权重时：

- 模型变小。
- 输入输出可能仍为 Float32。
- 部分中间计算仍可能使用浮点。

Full INT8 要求：

- 权重 INT8。
- activation INT8。
- 输入 INT8。
- 输出 INT8。
- 算子都支持整数实现。

我们的目标是 Full INT8。

### 8.5 转换代码的关键部分

`model/quantize.py` 的核心：

```python
import numpy as np
import tensorflow as tf


def representative_dataset(x_train: np.ndarray):
    """向转换器提供覆盖训练分布的校准样本。"""
    for sample in x_train[:300]:
        yield [sample[np.newaxis, ...].astype(np.float32)]


model = tf.keras.models.load_model("artifacts/activity_float.keras")
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = lambda: representative_dataset(x_train)
converter.target_spec.supported_ops = [
    tf.lite.OpsSet.TFLITE_BUILTINS_INT8
]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

model_bytes = converter.convert()

with open("artifacts/activity_int8.tflite", "wb") as model_file:
    model_file.write(model_bytes)
```

这里的 `x_train` 必须是与训练模型一致、完成相同预处理的数据。

### 8.6 转换成功不等于部署成功

转换后还必须检查：

- 输入 dtype 是否为 `int8`。
- 输出 dtype 是否为 `int8`。
- 输入 shape 是否正确。
- scale 是否大于零。
- 使用了哪些算子。
- TFLM 是否支持并注册这些算子。

检查输入输出：

```python
import tensorflow as tf


interpreter = tf.lite.Interpreter(
    model_path="artifacts/activity_int8.tflite"
)
interpreter.allocate_tensors()

print(interpreter.get_input_details())
print(interpreter.get_output_details())
```

### 8.7 比较 Float 与 INT8

使用同一 test set 分别执行：

```text
Float Keras 模型
INT8 LiteRT/TFLite Interpreter
```

记录：

- 两者总体准确率。
- 每类准确率。
- 混淆矩阵变化。
- 模型文件大小。

量化后略有精度变化是正常的。若大幅下降，优先检查：

- representative dataset 是否正确。
- 输入是否重复归一化。
- INT8 输入量化公式是否正确。
- 是否发生大量 clipping（截断）。

### 8.8 本课检查点

- [ ] 已生成 `activity_int8.tflite`。
- [ ] 输入输出 dtype 均为 INT8。
- [ ] 能读取 input scale 和 zero-point。
- [ ] 已比较 Float 与 INT8 结果。
- [ ] 能解释 representative dataset 的作用。

**小测试：为什么把 Float32 数组直接调用 `astype(np.int8)`，不等于正确量化？**

---

## 第 9 课：建立 Golden Test

### 9.1 Golden Test 是什么

Golden Test 使用固定输入和已知正确输出，验证不同运行环境是否一致。

本项目要比较：

```text
PC INT8 Interpreter
        与
STM32 TFLM Interpreter
```

两端必须使用：

- 完全相同的 `.tflite` 字节。
- 完全相同的 INT8 输入。
- 相同类别顺序。

### 9.2 为什么先在 Python 中把输入量化好

首轮让固件直接接收 INT8 测试向量：

```text
normalized Float32 window
  → Python 按模型参数量化
  → 固定 INT8 window
  → 编入固件 Flash
```

这样暂时排除设备端浮点预处理差异，先验证模型 Runtime。

### 9.3 正确量化输入

```python
import numpy as np


def quantize_input(
    values: np.ndarray,
    scale: float,
    zero_point: int,
) -> np.ndarray:
    """按照模型输入参数把实数转换为 INT8。"""
    quantized = np.round(values / scale) + zero_point
    quantized = np.clip(quantized, -128, 127)
    return quantized.astype(np.int8)
```

不要省略 `clip`。超出 INT8 范围后直接转换会发生 wraparound（回绕），结果可能从很大的正数变成负数。

### 9.4 Golden Test 应保存什么

建议保存：

```text
artifacts/golden_inputs.npy
artifacts/golden_expected_classes.npy
artifacts/golden_outputs.npy
artifacts/golden_metadata.json
```

metadata 至少包含：

```json
{
  "input_shape": [1, 128, 3],
  "input_dtype": "int8",
  "input_scale": "从模型读取",
  "input_zero_point": "从模型读取",
  "class_names": [
    "WALKING",
    "WALKING_UPSTAIRS",
    "WALKING_DOWNSTAIRS",
    "SITTING",
    "STANDING",
    "LAYING"
  ]
}
```

### 9.5 选哪些 Golden 样本

首轮不需要很多：

- 每个类别至少一条预测正确的典型样本。
- 至少一条模型容易混淆的样本。
- 输入范围应覆盖常见最小值和最大值。

Golden Test 不是为了重新统计准确率，而是为了检查运行环境一致性。

### 9.6 PC 端验收

PC 脚本应逐条打印：

```text
case id
expected class
actual class
raw INT8 output
PASS / FAIL
```

只有 PC Golden Test 稳定通过，才把测试向量转成 C 数组。

### 9.7 把二进制转为 C 数组

模型需要编入 Flash。可使用 `xxd`：

```bash
xxd -i activity_int8.tflite > model_data.cc
```

Windows 若没有 `xxd`，使用 Python 生成更稳定：

```python
from pathlib import Path


def bytes_to_cpp_array(source_path: Path, output_path: Path) -> None:
    """把模型字节生成为只读 C++ 数组。"""
    data = source_path.read_bytes()
    items = [f"0x{value:02x}" for value in data]

    lines = []
    for start in range(0, len(items), 12):
        lines.append("    " + ", ".join(items[start : start + 12]) + ",")

    content = "\n".join(
        [
            '#include "model_data.h"',
            "",
            "alignas(16) const unsigned char g_model_data[] = {",
            *lines,
            "};",
            "",
            f"const unsigned int g_model_data_len = {len(data)};",
            "",
        ]
    )
    output_path.write_text(content, encoding="utf-8")
```

头文件：

```cpp
#pragma once

extern const unsigned char g_model_data[];
extern const unsigned int g_model_data_len;
```

数组声明为 `const`，链接器才有机会把它放入只读 Flash section。

### 9.8 本课检查点

- [ ] PC Golden Test 可重复通过。
- [ ] INT8 输入由 scale 和 zero-point 正确生成。
- [ ] 已保存类别顺序。
- [ ] 模型和测试向量能生成 C/C++ 数组。

**小测试：如果 Python 与固件中的 class name 顺序不同，即使输出 tensor 完全一致，会发生什么？**

---

# 第四部分：理解设备端 Runtime

## 第 10 课：TFLM 在 MCU 中如何工作

### 10.1 TFLM 不负责什么

TFLM 不负责：

- 采集 IMU。
- 创建 FreeRTOS 任务。
- 管理 UART。
- 自动选择任意大小的内存。
- 训练模型。

它主要负责：

```text
读取模型
  → 检查 schema
  → 准备算子
  → 在 Tensor Arena 中规划内存
  → 接收输入
  → 执行 Invoke
  → 提供输出
```

### 10.2 `.tflite` 模型里有什么

模型文件使用 FlatBuffers 格式，包含：

- 模型 schema 版本。
- tensor 形状和类型。
- 算子计算图。
- 权重和 bias。
- 量化参数。

模型文件是数据，不是可直接执行的 ARM 指令。

### 10.3 `MicroInterpreter`

`MicroInterpreter` 是设备端的执行器。

核心对象关系：

```text
Model
OpResolver
Tensor Arena
    ↓
MicroInterpreter
    ↓
AllocateTensors()
    ↓
input tensor / output tensor
    ↓
Invoke()
```

### 10.4 `MicroMutableOpResolver`

TFLM 不会默认把所有算子都链接进固件。你需要注册模型实际使用的算子：

```cpp
tflite::MicroMutableOpResolver<3> resolver;
resolver.AddReshape();
resolver.AddFullyConnected();
resolver.AddSoftmax();
```

好处：

- 减小固件体积。
- 缺失算子时更容易定位。
- 明确模型与 Runtime 的依赖关系。

模板参数表示 resolver 最多容纳多少种算子，不是模型层数。

### 10.5 Tensor Arena

TFLM 避免动态内存分配，使用一块预先提供的连续 RAM：

```cpp
alignas(16) static std::uint8_t tensor_arena[TENSOR_ARENA_SIZE];
```

Arena 用于：

- input/output tensor。
- 中间 activation。
- 算子临时 buffer。
- Runtime 的持久结构。

模型权重通常仍直接从 Flash 中读取，不需要把整个模型复制进 Arena。

Arena 太小：

```text
AllocateTensors() failed
```

Arena 太大：

- 浪费 RAM。
- 可能导致链接失败。
- 可能挤压 FreeRTOS task stack 和其他缓冲区。

首轮可以先给出保守容量，成功后再通过内存记录逐步缩小。

### 10.6 schema 检查

模型转换器和 TFLM Runtime 对 schema 的理解必须兼容：

```cpp
const tflite::Model* model = tflite::GetModel(g_model_data);

if (model->version() != TFLITE_SCHEMA_VERSION) {
    // 模型格式与当前 Runtime 不兼容。
    return;
}
```

这就是为什么 TFLM 应固定 commit，而不能今天更新一半、明天更新另一半。

### 10.7 inference 的固定步骤

```cpp
// 把测试窗口复制到模型输入 tensor。
std::memcpy(
    input_tensor->data.int8,
    test_window,
    input_element_count
);

if (interpreter.Invoke() != kTfLiteOk) {
    // 推理执行失败时终止当前测试。
    return;
}

const std::int8_t* scores = output_tensor->data.int8;
```

随后寻找最大输出值对应的 class index。

### 10.8 本课检查点

- [ ] 能解释 Model、Resolver、Arena 和 Interpreter。
- [ ] 知道模型文件不是 ARM 指令。
- [ ] 知道权重主要在 Flash，Arena 主要在 RAM。
- [ ] 能解释缺失算子为什么会导致初始化或执行失败。

**小测试：把 Tensor Arena 扩大，会让模型权重文件自动变小吗？**

---

## 第 11 课：先准备固件工具链

这一课现在只阅读。等第 9 课通过后再执行安装。

### 11.1 本机缺少的三个工具

#### CMake

读取 `CMakeLists.txt`，生成具体构建系统。

#### Ninja

执行 CMake 生成的构建任务。

#### Arm GNU Toolchain

把 C/C++ 编译成 Cortex-M 指令，包括：

```text
arm-none-eabi-gcc
arm-none-eabi-g++
arm-none-eabi-ld
arm-none-eabi-size
arm-none-eabi-objdump
arm-none-eabi-gdb
```

`arm-none-eabi` 的含义：

- `arm`：目标架构。
- `none`：不依赖特定厂商操作系统。
- `eabi`：Embedded Application Binary Interface。

### 11.2 安装 CMake

在 PowerShell 中执行：

```powershell
winget install --id Kitware.CMake --exact
```

当前 WinGet 源显示 CMake 4.4.2。

### 11.3 安装 Ninja

```powershell
winget install --id Ninja-build.Ninja --exact
```

当前 WinGet 源显示 Ninja 1.13.2。

### 11.4 安装 Arm GNU Toolchain

WinGet 当前提供：

```powershell
winget show --id Arm.GnuArmEmbeddedToolchain --exact
```

核查到的 WinGet 版本为 14.2.Rel1，可满足 Cortex-M4 首轮构建。Arm 的最新 Release 已从 15.3.rel1 起迁移到官方 GitLab，因此：

- 希望安装方便：使用 WinGet 14.2.Rel1，并在项目中固定版本。
- 希望使用当前最新官方 Release：从 Arm GitLab 手动下载 15.3.rel1 的 Windows `arm-none-eabi` 包。

“不是最新”不自动等于“已经过时”。本项目更看重 TFLM、编译选项和 ABI 的一致性。

WinGet 安装命令：

```powershell
winget install --id Arm.GnuArmEmbeddedToolchain --exact
```

### 11.5 重新打开终端

安装完成后关闭并重新打开 Cursor 终端，然后执行：

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
```

任何一个命令找不到，都先检查 PATH，不要开始修改工程代码。

### 11.6 TFLM 官方 Makefile 与 Windows

TFLM 官方 Cortex-M generic 构建路径主要使用 Make 和 shell script。Windows Native 直接执行会遇到 Unix 工具依赖。

有两个策略：

#### 策略 A：首轮推荐

- Windows 运行 Python、CMake 和 Renode。
- 固件工程直接编译固定 TFLM source tree。
- 项目提供明确的 CMake source list。

优点：运行环境单一。缺点：需要维护 TFLM source list。

#### 策略 B：使用 WSL

- 在 WSL 中运行 TFLM 官方 Makefile。
- 生成 Cortex-M generic static library 或 source tree。
- Windows 或 WSL 构建最终固件。

优点：更接近 TFLM 官方构建路径。缺点：增加一套环境和路径映射。

本教程首轮采用策略 A；在真正创建固件工程时固定 TFLM commit 和 source list。若维护成本变高，再切换官方 project generation。

### 11.7 本课检查点

- [ ] CMake 能显示版本。
- [ ] Ninja 能显示版本。
- [ ] `arm-none-eabi-gcc` 能显示版本。
- [ ] 理解 CMake、Ninja、编译器各自职责。

**小测试：CMake 自己会把 C++ 直接编译成 Cortex-M 机器码吗？**

---

## 第 12 课：不要直接上模型，先让 UART 在 Renode 说话

这是嵌入式部署最重要的分层调试原则：

```text
先证明固件能启动
  → 再证明 FreeRTOS 能调度
  → 再证明 TFLM 能初始化
  → 最后执行模型
```

否则模型没有输出时，你无法判断是时钟、UART、RTOS、链接、Arena 还是算子问题。

### 12.1 固件目标

第一版固件只做：

```text
Reset
  → SystemInit
  → 初始化 UART2
  → 输出 BOOT_OK
  → 循环等待
```

目标平台：

- Renode `stm32f4_discovery-kit.repl`。
- Cortex-M4 STM32F4。
- UART2 对应 Renode `sysbus.usart2`。

### 12.2 为什么先用 UART2

Renode 的 STM32F4 platform description 已建模：

```text
usart1
usart2
usart3
uart4
uart5
```

使用 UART2 后，可通过：

```text
showAnalyzer sysbus.usart2
```

直接查看输出。

### 12.3 最小 Renode 脚本

创建 `renode/boot.resc`：

```text
$bin?=@../firmware/build/tinyml.elf

mach create
machine LoadPlatformDescription @platforms/boards/stm32f4_discovery-kit.repl
sysbus LoadELF $bin
showAnalyzer sysbus.usart2
start
```

逐句解释：

- `$bin?=`：定义默认 ELF 路径，也允许外部覆盖。
- `mach create`：创建虚拟机器。
- `LoadPlatformDescription`：加载 CPU、内存和外设模型。
- `LoadELF`：加载固件 section 和 symbol。
- `showAnalyzer`：打开 UART 查看器。
- `start`：开始执行。

### 12.4 运行方式

PowerShell：

```powershell
renode renode\boot.resc
```

预期 UART Analyzer 出现：

```text
BOOT_OK
```

### 12.5 如果没有输出，按顺序排查

1. ELF 路径是否正确。
2. Reset_Handler 是否进入。
3. vector table 地址是否正确。
4. linker script 是否匹配 STM32F4 memory map。
5. UART 实例是否为 USART2。
6. RCC 与 UART 寄存器写入是否被平台模型支持。
7. 程序是否进入 HardFault。

不要先怀疑模型，因为此时模型根本不存在。

### 12.6 使用 GDB

在 `start` 前加入：

```text
machine StartGdbServer 3333
```

使用：

```powershell
arm-none-eabi-gdb firmware\build\tinyml.elf
```

进入 GDB 后：

```gdb
target remote localhost:3333
break main
continue
```

这样可验证程序是否进入 `main`。

### 12.7 本课检查点

- [ ] Renode 能加载自定义 ELF。
- [ ] UART2 输出 `BOOT_OK`。
- [ ] 知道如何让 GDB 连接 Renode。
- [ ] 还没有加入 FreeRTOS 和 TFLM。

**小测试：如果程序没有输出 `BOOT_OK`，为什么此时不应该调整 Tensor Arena？**

---

## 第 13 课：加入 FreeRTOS，但仍不加入模型

你已经熟悉 FreeRTOS，因此这里只强调与 TinyML 有关的结构。

### 13.1 为什么保留 FreeRTOS

真实 TinyML 应用通常同时处理：

- 传感器采样。
- 推理。
- 通信。
- 日志。
- 低功耗状态。

首轮只创建一个 inference task，但保留未来结构。

### 13.2 使用静态任务

推荐：

```cpp
namespace {

StaticTask_t inference_task_control_block;
StackType_t inference_task_stack[INFERENCE_TASK_STACK_WORDS];

}  // namespace

extern "C" void InferenceTask(void* argument) {
    (void)argument;

    for (;;) {
        UartWriteLine("TASK_OK");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

创建：

```cpp
xTaskCreateStatic(
    InferenceTask,
    "Inference",
    INFERENCE_TASK_STACK_WORDS,
    nullptr,
    tskIDLE_PRIORITY + 1,
    inference_task_stack,
    &inference_task_control_block
);
```

选择静态任务的原因：

- task stack 在 Map 文件中可见。
- 不依赖 heap 配置。
- 更容易区分 task stack 与 Tensor Arena。

### 13.3 UART 输出顺序

建议：

```text
BOOT_OK
TASK_CREATED
SCHEDULER_START
TASK_OK
```

如果只看到 `SCHEDULER_START`，说明 scheduler 启动后的任务调度存在问题。

### 13.4 中断优先级

STM32F4 使用 NVIC priority bits。FreeRTOS 配置中的：

```text
configPRIO_BITS
configLIBRARY_LOWEST_INTERRUPT_PRIORITY
configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
```

必须与 MCU 和 Renode 平台一致。

如果 scheduler 异常，检查：

- SysTick handler。
- PendSV handler。
- SVC handler。
- FreeRTOS port 文件。
- NVIC priority 配置。

### 13.5 本课检查点

- [ ] 启动后能进入静态任务。
- [ ] UART 能周期输出 `TASK_OK`。
- [ ] task stack 在 Map 文件中可定位。
- [ ] 此时仍没有 TFLM。

**小测试：为什么把 Tensor Arena 放在 inference task 的局部变量中通常很危险？**

---

## 第 14 课：把 TFLM 接入 FreeRTOS 任务

### 14.1 初始化与推理分开

任务结构：

```text
InferenceTask 启动
  → InitializeModel()
  → 输出 MODEL_OK
  → 逐条运行 Golden Test
  → 输出 ALL_PASS
  → 阻塞或等待下一窗口
```

初始化只执行一次，`Invoke()` 可以执行多次。

### 14.2 建议模块边界

```text
firmware/app/tinyml_runtime.h
firmware/app/tinyml_runtime.cpp
firmware/generated/model_data.h
firmware/generated/model_data.cc
firmware/generated/golden_vectors.h
firmware/generated/golden_vectors.cc
```

generated 文件由 Python 生成，不手工维护。

### 14.3 Runtime 初始化骨架

```cpp
#include <cstdint>

#include "model_data.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

namespace {

constexpr std::size_t kTensorArenaSize = 64 * 1024;
alignas(16) std::uint8_t tensor_arena[kTensorArenaSize];

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;
TfLiteTensor* output_tensor = nullptr;

}  // namespace

bool InitializeModel() {
    model = tflite::GetModel(g_model_data);

    if (model->version() != TFLITE_SCHEMA_VERSION) {
        return false;
    }

    static tflite::MicroMutableOpResolver<3> resolver;

    if (resolver.AddReshape() != kTfLiteOk ||
        resolver.AddFullyConnected() != kTfLiteOk ||
        resolver.AddSoftmax() != kTfLiteOk) {
        return false;
    }

    static tflite::MicroInterpreter static_interpreter(
        model,
        resolver,
        tensor_arena,
        kTensorArenaSize
    );
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        return false;
    }

    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    return input_tensor->type == kTfLiteInt8 &&
           output_tensor->type == kTfLiteInt8;
}
```

说明：

- Arena 使用全局静态存储，避免 task stack 爆炸。
- `static_interpreter` 生命周期覆盖整个程序。
- 初始化后再次检查 tensor dtype，避免模型被替换后静默出错。
- 真实模型转换结果可能不需要 `RESHAPE`，最终以算子检查脚本为准。

### 14.4 运行一条窗口

```cpp
bool RunInference(
    const std::int8_t* input_data,
    std::size_t input_size,
    int* predicted_class
) {
    if (input_tensor == nullptr || output_tensor == nullptr) {
        return false;
    }

    const std::size_t expected_size = input_tensor->bytes;
    if (input_size != expected_size) {
        return false;
    }

    std::memcpy(input_tensor->data.int8, input_data, input_size);

    if (interpreter->Invoke() != kTfLiteOk) {
        return false;
    }

    int best_index = 0;
    std::int8_t best_score = output_tensor->data.int8[0];

    for (int index = 1; index < output_tensor->dims->data[1]; ++index) {
        const std::int8_t score = output_tensor->data.int8[index];
        if (score > best_score) {
            best_score = score;
            best_index = index;
        }
    }

    *predicted_class = best_index;
    return true;
}
```

### 14.5 C 和 C++ 边界

STM32 HAL 与 FreeRTOS 常以 C 编译，TFLM 使用 C++17。

从 C 调用 C++ 函数时，在头文件使用：

```cpp
#ifdef __cplusplus
extern "C" {
#endif

void TinyMlTask(void* argument);

#ifdef __cplusplus
}
#endif
```

否则 C++ name mangling（名称修饰）会导致链接器找不到函数。

### 14.6 首次失败最常见的地方

#### schema 不匹配

模型转换工具与 TFLM source 不兼容。

#### `AllocateTensors()` 失败

- Arena 太小。
- 算子缺失。
- 模型损坏。
- 内存对齐错误。

#### `Invoke()` 失败

- 输入 shape 或 dtype 错误。
- 未注册算子。
- 某个 kernel 不支持模型参数组合。

#### HardFault

- task stack 太小。
- Arena 与其他内存冲突。
- ABI 或 FPU 编译选项不一致。
- 未正确初始化 C++ static object。

### 14.7 本课检查点

- [ ] UART 输出 `MODEL_OK`。
- [ ] input/output tensor 均为 INT8。
- [ ] 单条 Golden 输入可执行。
- [ ] 预测 class 与 PC 一致。
- [ ] Map 文件能定位模型、Arena 和 task stack。

**小测试：`AllocateTensors()` 成功后，为什么仍要检查 input tensor 的 dtype 和 bytes？**

---

# 第五部分：用 Renode 自动验收

## 第 15 课：让 UART 输出变成测试协议

### 15.1 人能看懂不等于脚本能稳定解析

不推荐：

```text
模型好像启动成功啦
这次预测大概是 walking
```

推荐固定格式：

```text
BOOT_OK
TASK_OK
MODEL_OK
CASE id=0 expected=3 actual=3 PASS
CASE id=1 expected=0 actual=0 PASS
ALL_PASS count=2
```

规则：

- 固定关键字。
- 一条事件一行。
- 不依赖颜色。
- 不输出随机地址。
- 数字使用稳定格式。
- 失败也输出明确标志。

### 15.2 为什么需要失败消息

Robot Framework 的 `Wait For Line On Uart` 在目标字符串没出现时只能等 timeout。

如果固件主动输出：

```text
MODEL_FAIL reason=ALLOCATE_TENSORS
```

人可以立刻定位问题。

### 15.3 建议错误码

```text
MODEL_FAIL reason=SCHEMA
MODEL_FAIL reason=OP_REGISTER
MODEL_FAIL reason=ARENA
INFERENCE_FAIL reason=INPUT_SIZE
INFERENCE_FAIL reason=INVOKE
CASE_FAIL id=<value> expected=<value> actual=<value>
```

### 15.4 本课检查点

- [ ] UART 输出格式稳定。
- [ ] 成功和失败都有明确终止标志。
- [ ] 输出不依赖人工猜测。

**小测试：只有 `ALL_PASS` 而没有每个 CASE 的输出，会损失什么调试信息？**

---

## 第 16 课：编写第一个 Robot Framework 测试

### 16.1 Robot Framework 做什么

它启动 Renode、加载固件、监听 UART，并判断预期行是否出现。

它验证的是：

```text
固件能否在虚拟 MCU 上完成预期行为
```

不是：

```text
模型真实功耗和真机推理耗时
```

### 16.2 最小测试结构

创建 `renode/tinyml.robot`：

```robot
*** Settings ***
Suite Setup       Setup
Suite Teardown    Teardown
Test Teardown     Test Teardown
Resource          ${RENODEKEYWORDS}

*** Variables ***
${ELF}            ${CURDIR}/../firmware/build/tinyml.elf

*** Test Cases ***
Firmware Should Complete Golden Tests
    Execute Command                 mach create
    Execute Command                 machine LoadPlatformDescription @platforms/boards/stm32f4_discovery-kit.repl
    Execute Command                 sysbus LoadELF @${ELF}
    Create Terminal Tester          sysbus.usart2
    Start Emulation
    Wait For Line On Uart           BOOT_OK
    Wait For Line On Uart           TASK_OK
    Wait For Line On Uart           MODEL_OK
    Wait For Line On Uart           ALL_PASS count=6
```

注意：Windows 路径、Renode `@` 路径解析和当前目录需要实际运行时确认。首轮先用绝对路径验证，再收敛为相对路径。

### 16.3 运行测试

Renode 安装中若提供 `renode-test`：

```powershell
renode-test renode\tinyml.robot
```

如果 Windows 安装的命令名称或位置不同：

```powershell
Get-Command renode*
```

根据实际安装目录调用测试脚本，不要从网上复制另一个版本的路径。

### 16.4 测试结果

成功时：

```text
1 test, 1 passed, 0 failed
```

还会生成：

- `robot_output.xml`
- `log.html`
- `report.html`

失败时先看哪一条 `Wait For Line On Uart` 超时：

- `BOOT_OK` 缺失：启动层。
- `TASK_OK` 缺失：RTOS 层。
- `MODEL_OK` 缺失：TFLM 初始化层。
- `ALL_PASS` 缺失：推理结果层。

### 16.5 增加确定性检查

重复运行：

```powershell
renode-test -n 3 renode\tinyml.robot
```

目的不是证明真机可靠性，而是发现：

- 未初始化内存。
- 状态未复位。
- 输出时序不稳定。
- 测试依赖上一次运行。

### 16.6 本课检查点

- [ ] Robot 测试可以启动 Renode。
- [ ] 能验证分层 UART 标志。
- [ ] 失败报告能指出停在哪一层。
- [ ] 重复运行结果稳定。

**小测试：Robot 测试连续通过多次，为什么仍不能证明真实 IMU 输入一定工作？**

---

# 第六部分：把四层知识真正串起来

## 第 17 课：四层分别交付什么

### 17.1 知识层

你应该能解释：

- sample、feature、label、model。
- train、validation、test。
- window、shape、dtype。
- Float32 与 INT8。
- scale、zero-point、representative dataset。
- Flash、RAM、Tensor Arena。
- Renode 能验证和不能验证的内容。

知识层的目标不是背术语，而是能根据错误判断它属于哪一层。

### 17.2 工具层

你应该能独立执行：

```text
激活 Python 虚拟环境
运行训练脚本
运行量化脚本
检查 `.tflite`
生成 C 数组
构建 ELF
运行 Renode
运行 Robot 测试
查看 Map 文件
```

### 17.3 应用层

你应该拥有：

- 真实 UCI 数据处理流程。
- 可重复训练的 Float 模型。
- Full INT8 模型。
- PC Golden Test。
- FreeRTOS inference task。
- Renode 中运行的 STM32F4 ELF。

### 17.4 验证层

你应该记录：

- Float 测试准确率。
- INT8 测试准确率。
- 混淆矩阵。
- 模型文件大小。
- ELF section 大小。
- Tensor Arena 容量。
- task stack 容量。
- Golden Test 结果。
- 尚未验证的真机项目。

---

## 第 18 课：完整故障定位地图

### Python import 失败

检查工具层：

```powershell
python -c "import sys; print(sys.executable)"
python -m pip show tensorflow
```

### 模型完全学不会

检查知识和数据层：

- 标签是否与窗口对应。
- shape 是否正确。
- 是否含 NaN。
- 类别是否严重不平衡。
- 归一化是否正确。

### Float 准确率正常，INT8 大幅下降

检查量化层：

- representative dataset。
- input scale 与 zero-point。
- 是否重复归一化。
- clipping 比例。

### PC INT8 正确，固件 `AllocateTensors()` 失败

检查 Runtime 层：

- Arena。
- schema。
- resolver。
- 模型字节是否完整。

### `AllocateTensors()` 成功，`Invoke()` 失败

检查：

- input bytes。
- dtype。
- shape。
- 算子 kernel。
- 编译 ABI。

### Renode 没有 `BOOT_OK`

检查固件启动层：

- ELF。
- linker script。
- vector table。
- Reset_Handler。
- UART 和 RCC。

### 有 `BOOT_OK`，没有 `TASK_OK`

检查 FreeRTOS：

- handler。
- task 创建结果。
- scheduler。
- task stack。
- NVIC priority。

### 有 `MODEL_OK`，类别与 PC 不同

检查数据契约：

- 是否使用同一模型。
- INT8 输入字节是否一致。
- class order 是否一致。
- input tensor 写入长度。
- optimized kernel 是否引入允许范围内的数值差异。

---

## 第 19 课：首轮完成后如何迭代

一次只解决一个最有价值的问题。

### 如果准确率不足

顺序：

1. 看混淆矩阵。
2. 检查标签和窗口。
3. 检查按受试者划分。
4. 增加陀螺仪通道。
5. 再考虑小型 CNN。

不要第一反应就堆层数。

### 如果 Flash 太大

检查：

- Dense 参数数量。
- 是否链接了未使用算子。
- 是否开启 section garbage collection。
- 模型是否确实 Full INT8。

### 如果 RAM 太大

区分：

- Tensor Arena。
- FreeRTOS task stack。
- 全局输入 buffer。
- HAL buffer。
- linker section。

不要只看一个“总 RAM”数字。

### 如果需要性能优化

完成真机基线后再加入 CMSIS-NN：

```text
TFLM reference kernel
  → 真机测量
  → 启用与当前 TFLM commit 兼容的 CMSIS-NN
  → 使用同一输入重新测量
```

Renode 适合验证优化后功能是否仍正确，不适合得出真实加速倍数。

---

# 第七部分：学习节奏和验收

## 第 20 课：推荐执行顺序

不要按“每天必须几课”推进，按产物推进：

### 阶段 A：理解并运行第一个模型

完成：

- 第 1～4 课。
- Python 环境。
- synthetic（人工合成）二分类模型。

产物：

```text
artifacts/first_model.keras
```

### 阶段 B：真实数据与 Float 基线

完成：

- 第 5～7 课。
- UCI 数据下载、解析和窗口化。
- 真实活动分类模型。

产物：

```text
activity_float.keras
confusion_matrix_float.png
normalization.json
```

### 阶段 C：Full INT8 与 Golden Test

完成：

- 第 8～9 课。
- INT8 模型。
- PC Golden Test。

产物：

```text
activity_int8.tflite
golden_inputs.npy
golden_expected_classes.npy
```

### 阶段 D：固件基础

完成：

- 第 10～13 课。
- 构建工具链。
- UART 固件。
- FreeRTOS task。

产物：

```text
tinyml.elf
tinyml.map
BOOT_OK
TASK_OK
```

### 阶段 E：TFLM 与自动验收

完成：

- 第 14～16 课。
- TFLM 初始化和 inference。
- Robot Framework。

产物：

```text
MODEL_OK
ALL_PASS
Robot test passed
```

### 阶段 F：复盘

完成：

- 第 17～19 课。
- 指标报告。
- 未验证项。
- 下一轮单一目标。

---

## 第 21 课：首轮总验收清单

### 知识

- [ ] 能区分训练和推理。
- [ ] 能解释 sample、feature、label 和 tensor。
- [ ] 能读懂 shape 与 dtype。
- [ ] 能解释数据泄漏和按受试者划分。
- [ ] 能解释 Full INT8、scale 和 zero-point。
- [ ] 能解释 Flash、Arena 和 task stack。

### 工具

- [ ] 能创建并激活 `.venv`。
- [ ] 能运行训练、量化和检查脚本。
- [ ] 能使用 CMake、Ninja 和 Arm GCC 构建。
- [ ] 能加载 ELF 到 Renode。
- [ ] 能使用 Robot Framework 检查 UART。

### 应用

- [ ] 真实 UCI 数据能转换为 `(N, 128, 3)`。
- [ ] Float 模型有可解释结果。
- [ ] INT8 模型输入输出均为 INT8。
- [ ] PC Golden Test 稳定。
- [ ] STM32F4 固件执行相同模型和输入。

### 验证

- [ ] PC 与固件 Top-1 分类一致。
- [ ] Map 文件记录 Flash 和 RAM。
- [ ] Robot 测试可重复通过。
- [ ] 真机延迟、功耗和传感器噪声被标记为未验证。

---

## 22. 现在应该从哪里开始

不要从 UCI 数据或 TFLM 开始。

你的下一步只有一个：

```text
执行第 2 课
  → 创建 `.venv`
  → 安装 TensorFlow
  → 完成环境检查点
```

然后执行第 3、4 课，亲眼看到第一个模型训练完成。模型真正跑起来之后，再学习真实数据、量化和固件，认知负担会小很多。

最终自检：

**当 PC Float 模型、PC INT8 模型和 Renode 固件产生三个不同结果时，你会先比较哪两端的输入字节，为什么？**

---

## 23. 官方资料

- TensorFlow 2.21 Release  
  <https://github.com/tensorflow/tensorflow/releases/tag/v2.21.0>
- TensorFlow pip 安装  
  <https://www.tensorflow.org/install/pip>
- LiteRT for Microcontrollers  
  <https://ai.google.dev/edge/litert/microcontrollers/overview>
- TFLM 官方仓库  
  <https://github.com/tensorflow/tflite-micro>
- TFLM Cortex-M generic 构建说明  
  <https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/cortex_m_generic/README.md>
- TFLM Renode 说明  
  <https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/docs/renode.md>
- Renode STM32F4 demo  
  <https://renode.readthedocs.io/en/latest/introduction/demo.html>
- Renode Robot Framework 测试  
  <https://renode.readthedocs.io/en/latest/introduction/testing.html>
- UCI HAR 原始数据集  
  <https://archive.ics.uci.edu/dataset/341/smartphone+based+recognition+of+human+activities+and+postural+transitions>
- Arm GNU Toolchain Release 入口  
  <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>
