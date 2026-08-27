# TinyML 最小学习闭环方案

> 适用背景：熟悉 STM32 与 FreeRTOS，暂时没有真实硬件，希望利用本机 Renode 快速贯通知识、工具、应用和验证。
>
> 信息核查日期：2026-08-27。版本信息来自官方文档、官方仓库及 Release 页面。

## 1. 最终目标

完成一个“离线 IMU 人体活动识别器”：

1. 使用公开的三轴加速度数据训练小型分类模型。
2. 将模型转换成 Full INT8 `.tflite`。
3. 在 PC 端执行量化模型并生成 Golden Test。
4. 将模型和测试窗口编入 STM32F4 固件。
5. 由 FreeRTOS 推理任务调用 LiteRT for Microcontrollers。
6. 在 Renode 的 STM32F4 Discovery 虚拟平台运行同一个 ELF。
7. 使用 Robot Framework 检查 UART 启动信息和分类结果。

闭环完成的标志不是“准确率很高”，而是下面这条链路可重复运行：

```text
公开数据
  → 训练
  → Full INT8 量化
  → PC Golden Test
  → Cortex-M 固件
  → Renode 运行
  → 自动验收
  → 根据失败证据迭代
```

真实推理延迟、功耗、传感器噪声和电气行为不在无硬件阶段的验收范围内。

## 2. 为什么选择这个应用

选择 UCI 的 Smartphone-Based Recognition of Human Activities and Postural Transitions 数据集，首轮只使用其中六类稳定活动和三轴加速度，暂不处理姿态转换。

它适合作为第一个闭环，因为：

- 输入与未来 MCU 连接 IMU 的形态接近。
- 数据集提供原始三轴加速度，采样频率为 50 Hz。
- 数据规模足以学习窗口、划分、量化和混淆矩阵。
- 输入可直接编入 Flash，不依赖 Renode 是否提供特定 IMU 外设模型。
- 后续获得硬件后，只需把“测试向量输入”替换为“传感器采样窗口”。

首轮模型保持保守：

```text
Input: 128 × 3 int8
  → Flatten / Reshape
  → Dense + ReLU
  → Dense
  → Softmax
Output: 6 类活动
```

优先使用 `RESHAPE`、`FULLY_CONNECTED` 和 `SOFTMAX` 等常见算子，降低设备端算子缺失和内存估算的复杂度。CNN（Convolutional Neural Network，卷积神经网络）和 CMSIS-NN 优化放到下一轮。

## 3. 四个层面的最小范围

### 3.1 知识层

只掌握会阻塞当前闭环的知识：

- **监督分类**：输入、标签、交叉熵、Top-1 分类结果。
- **数据划分**：按受试者划分训练集、验证集和测试集，避免同一人的相邻窗口泄漏到不同集合。
- **窗口化**：固定长度、重叠比例，以及窗口与实时采样缓冲区的对应关系。
- **混淆矩阵**：判断模型把哪些活动混在一起，而不是只看总体准确率。
- **Full INT8 量化**：权重和激活均为 INT8；理解 representative dataset、scale 和 zero-point。
- **算子约束**：模型能转换不等于 TFLM 已注册所有算子。
- **静态内存**：模型存放于 Flash，Tensor Arena、任务栈和全局缓冲区占用 RAM。
- **仿真边界**：Renode 适合功能验证，不能直接代表真机延迟和功耗。

本层产物：

- 一页数据流说明。
- 一份模型输入输出规格。
- 一份“Renode 能证明什么、不能证明什么”的边界清单。

停止学习条件：

- 能解释一段三轴采样窗口如何经过量化变成分类结果。
- 能解释为什么 representative dataset 不应直接使用测试集。
- 能指出模型大小与 Tensor Arena 分别主要占用哪类存储器。

### 3.2 工具层

首轮只保留一条主工具链：

- **训练与转换**：Python 3.11 或 3.12、TensorFlow 2.21。
- **设备端 Runtime（运行时）**：LiteRT for Microcontrollers，社区与仓库中仍常称 TFLM（TensorFlow Lite for Microcontrollers）。
- **RTOS（Real-Time Operating System，实时操作系统）**：FreeRTOS Kernel 11.3.1。
- **MCU 支持包**：STM32CubeF4 1.28.3。
- **交叉编译**：Arm GNU Toolchain 15.3.rel1 的 `arm-none-eabi` 目标。
- **构建**：CMake + Ninja，具体版本写入环境清单。
- **虚拟平台**：Renode 1.16.1，使用 STM32F4 Discovery 平台描述。
- **自动验收**：Renode 集成的 Robot Framework。

版本策略：

1. TensorFlow 使用稳定 Release，不使用 nightly。
2. TFLM 缺少与常规库相同的稳定发版节奏，选择验证过的 commit 并锁定 SHA。
3. TFLM、CMSIS-NN 和 TensorFlow 模型格式存在兼容关系，不盲目拼接各自“最新版本”。
4. 首轮使用 TFLM reference kernels；CMSIS-NN 7.0.0 仅在闭环稳定后加入。
5. 每个依赖记录“版本或 commit、来源 URL、核查日期”。
6. 闭环开发过程中不随意升级；完成后再集中验证升级。

不加入首轮的工具：

- Edge Impulse：能更快做出 Demo，但会隐藏当前希望理解的转换和设备端集成过程。
- PyTorch：避免同时维护两条模型转换路径。
- Zephyr：已有 FreeRTOS 背景，没有必要为首轮额外学习 RTOS。
- STM32Cube.AI：后续可作为厂商工具链对照，不作为第一条主路径。

本层产物：

- 可复现的 Python 虚拟环境。
- 固定依赖的 manifest 或 lock 文件。
- 能生成 STM32F4 ELF 和 Linker Map 的构建命令。
- 能加载自定义 ELF 的 Renode `.resc` 脚本。

### 3.3 应用层

应用的数据契约先于模型代码确定：

- 输入：连续三轴加速度。
- 窗口：128 个采样点。
- 通道：X、Y、Z。
- 输出：六类稳定活动。
- 首轮输入来源：编入 Flash 的测试窗口。
- 后续输入来源：真实 IMU 的双缓冲或环形缓冲区。

推荐固件结构：

```text
Reset / HAL 初始化
  → UART 初始化
  → 创建静态 FreeRTOS 推理任务
  → TFLM 初始化并分配 Tensor Arena
  → 逐个读取 Flash 中的 INT8 测试窗口
  → Invoke
  → UART 输出类别、分数和 PASS/FAIL
```

为缩短首轮路径：

- Python 根据输入 tensor 的 scale 和 zero-point 提前生成 INT8 测试向量。
- 固件不实现浮点归一化，先验证纯 INT8 推理链路。
- 使用 `MicroMutableOpResolver` 只注册模型实际需要的算子。
- FreeRTOS 使用静态任务与静态缓冲区，资源更容易从 Map 文件审计。
- TFLM C++ 与 STM32 HAL/FreeRTOS C 接口保持清晰边界。
- UART 输出采用稳定、机器可解析的格式。

建议的 UART 协议：

```text
BOOT_OK
MODEL_OK schema=<value> arena=<bytes>
CASE id=<value> expected=<class> actual=<class> score=<value> PASS
ALL_PASS count=<value>
```

本层产物：

- `model_int8.tflite`。
- 模型 C 数组及元数据。
- 一组带期望标签的 INT8 Golden Test 窗口。
- 可在 STM32F4 Discovery 平台启动的 ELF。

### 3.4 验证与迭代层

验证分成四组：

#### 模型验证

- 固定随机种子、数据划分和预处理配置。
- 保存 Float 与 INT8 模型的混淆矩阵。
- 记录量化前后准确率差异，不因一次结果不理想立即扩大模型。
- 检查 `.tflite` 中的算子列表和输入输出 dtype。

#### PC Golden Test

- 对固定窗口保存预期类别。
- 保存输入和输出 tensor 的量化参数。
- 使用与固件相同的 `.tflite` 模型运行。
- 首轮要求 Top-1 结果一致；原始 INT8 输出差异应被记录。

#### 固件静态验证

- 构建成功且无未解析算子。
- 从 ELF 和 Map 文件记录 Flash、RAM、任务栈与 Tensor Arena。
- 检查模型数组位于只读段。
- 检查 Tensor Arena 和任务栈不存在明显重叠或溢出风险。

#### Renode 自动验证

- STM32F4 Discovery 平台能加载同一个 ELF。
- UART 出现 `BOOT_OK` 和 `MODEL_OK`。
- 每个测试窗口均输出预期类别。
- 固件最终输出 `ALL_PASS`。
- 重启后结果稳定。
- Robot Framework 失败报告能指出缺失或错误的 UART 行。

Renode 阶段不使用宿主机运行时长作为真机性能结论。当前只记录“功能正确”和“模拟时间行为是否满足测试脚本”，真机延迟与功耗留待硬件阶段。

## 4. 最短执行顺序

### 阶段一：建立 PC 数据基线

任务：

1. 下载并校验官方 UCI 数据集。
2. 只读取原始三轴加速度与六类稳定活动。
3. 按受试者划分数据，完成窗口化。
4. 训练小型 MLP。
5. 输出测试集准确率和混淆矩阵。

通过条件：

- 重复运行得到一致的数据数量、输入形状和近似指标。
- 能定位至少一类主要混淆，并用数据解释它。

不要做：

- 网格搜索大量超参数。
- 追求论文级准确率。
- 加入复杂滤波和频域特征。

### 阶段二：打通 Full INT8

任务：

1. 从训练集抽取 representative dataset。
2. 强制转换器只允许 `TFLITE_BUILTINS_INT8`。
3. 将输入和输出类型设为 `int8`。
4. 检查模型算子和量化参数。
5. 建立 PC 端 Golden Test。

通过条件：

- 模型输入输出均为 INT8。
- 模型不含首轮固件未计划支持的算子。
- Golden Test 能重复运行并输出固定结果。

### 阶段三：打通 Cortex-M 固件

任务：

1. 将 `.tflite` 转换为只读 C 数组。
2. 集成固定 commit 的 TFLM。
3. 注册最小算子集合。
4. 分配 Tensor Arena。
5. 创建静态 FreeRTOS 推理任务。
6. 编入若干 INT8 Golden Test 窗口。
7. 生成 ELF 和 Map 文件。

通过条件：

- 固件可链接。
- 模型 schema 检查通过。
- `AllocateTensors()` 成功。
- Map 文件中能明确找到模型、Tensor Arena 和任务栈。

### 阶段四：打通 Renode

任务：

1. 使用官方 `stm32f4_discovery.resc` 作为平台起点。
2. 通过 `$bin` 或 `sysbus LoadELF` 加载自定义 ELF。
3. 打开 UART Analyzer 检查启动与推理输出。
4. 排查 HardFault、非法指令、缺失外设或时钟配置问题。

通过条件：

- Renode 能稳定启动固件。
- UART 输出与 PC Golden Test 的 Top-1 结果一致。
- 不把 Renode 宿主耗时解释为真机延迟。

### 阶段五：自动验收并复盘

任务：

1. 编写 Robot Framework 测试。
2. 检查启动标志、模型初始化和全部分类结果。
3. 重复运行，确认结果确定性。
4. 汇总模型精度、量化差异、Flash、RAM 和未验证项。
5. 只选择一个最有价值的问题进入下一轮。

通过条件：

- 一条命令可以完成仿真测试。
- 任意一步失败时，日志能指出失败层次。
- 有一份明确的下一轮假设，而不是泛泛地“继续优化”。

## 5. 推荐目录结构

```text
TinyML/
├─ data/
│  ├─ raw/
│  └─ processed/
├─ model/
│  ├─ train.py
│  ├─ quantize.py
│  ├─ inspect_model.py
│  └─ golden_test.py
├─ artifacts/
│  ├─ model_int8.tflite
│  ├─ model_data.cc
│  ├─ model_data.h
│  └─ golden_vectors.*
├─ firmware/
│  ├─ app/
│  ├─ platform/
│  ├─ third_party/
│  ├─ linker/
│  └─ CMakeLists.txt
├─ renode/
│  ├─ tinyml_stm32f4.resc
│  └─ tinyml_stm32f4.robot
├─ reports/
│  ├─ confusion_matrix.*
│  ├─ model_summary.*
│  └─ memory_report.*
└─ versions.md
```

`artifacts/` 中的文件必须由脚本生成，避免人工复制导致模型、测试向量和固件不一致。

## 6. 首轮验收清单

- [ ] 数据按受试者划分，而不是随机打散相邻窗口。
- [ ] Float 模型具有可解释的基线结果。
- [ ] `.tflite` 输入输出 dtype 均为 INT8。
- [ ] representative dataset 仅来自训练数据。
- [ ] 固件只注册实际使用的 TFLM 算子。
- [ ] PC 与固件使用完全相同的模型字节。
- [ ] Golden Test 的量化参数被固定并可追溯。
- [ ] ELF 和 Map 文件由同一次构建产生。
- [ ] Renode 加载的是该次构建生成的 ELF。
- [ ] Robot Framework 能验证全部 UART 结果。
- [ ] Flash、RAM 和 Tensor Arena 已记录。
- [ ] 延迟、功耗、真实噪声被明确标为“尚未验证”。

## 7. 下一轮只选一个方向

首轮通过后，根据证据选择一个方向：

- **准确率不足**：分析混淆类别，再考虑滤波、更多通道或小型 CNN。
- **模型或 Arena 偏大**：缩小隐藏层，检查算子临时缓冲，再考虑 CMSIS-NN。
- **固件集成脆弱**：增加错误路径、版本检查和自动构建。
- **准备接入硬件**：实现真实 IMU 驱动、双缓冲采样和真机基准。
- **需要性能优化**：启用与当前 TFLM commit 验证兼容的 CMSIS-NN，并在真机测量。

不要同时优化多个方向，否则无法判断改动究竟解决了什么。

## 8. 当前版本与官方依据

- TensorFlow 2.21.0：2026-03-06 发布；官方支持 Python 3.10–3.13。本方案选择更稳妥的 Python 3.11 或 3.12。
  - <https://github.com/tensorflow/tensorflow/releases/tag/v2.21.0>
  - <https://www.tensorflow.org/install/source>
- LiteRT for Microcontrollers：Google 当前文档名称；官方代码仍在 `tensorflow/tflite-micro`。
  - <https://ai.google.dev/edge/litert/microcontrollers/overview>
  - <https://github.com/tensorflow/tflite-micro>
- TFLM 官方文档明确包含 Renode 软件仿真说明。
  - <https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/docs/renode.md>
- Renode 1.16.1：2026-02-16 发布；官方文档列出 STM32F4 Discovery 示例，并支持 Robot Framework。
  - <https://github.com/renode/renode/releases/tag/v1.16.1>
  - <https://renode.readthedocs.io/en/latest/introduction/demo.html>
  - <https://renode.readthedocs.io/en/latest/introduction/testing.html>
- FreeRTOS Kernel 11.3.1：2026-08-21 发布，并修复影响旧版本的安全问题。
  - <https://github.com/FreeRTOS/FreeRTOS-Kernel/releases/tag/V11.3.1>
- STM32CubeF4 1.28.3：2025-07-23 发布。
  - <https://github.com/STMicroelectronics/STM32CubeF4/blob/master/Release_Notes.html>
- CMSIS-NN 7.0.0：当前官方 Release；首轮不启用，避免优化路径干扰功能闭环。
  - <https://github.com/ARM-software/CMSIS-NN/releases/tag/v7.0.0>
  - <https://arm-software.github.io/CMSIS-NN/latest/>
- Arm GNU Toolchain：新的官方 Release 从 15.3.rel1 起迁移到 Arm GitLab。
  - <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>
- UCI 原始人体活动数据集：包含 50 Hz 三轴加速度、三轴角速度和活动标签。
  - <https://archive.ics.uci.edu/dataset/341/smartphone+based+recognition+of+human+activities+and+postural+transitions>

## 9. 自检问题

如果 PC 和 Renode 中的 INT8 分类结果一致，为什么仍然不能宣布这个模型已经适合真实 STM32 产品？

