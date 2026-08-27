# TinyML 核心术语表

> 面向 TinyML 初学者，按“硬件 → 数据与信号 → 机器学习 → 模型结构 → 评估 → 量化与部署 → 工具链”分类。
>
> 术语核查日期：2026-08-27。Google 当前将 TensorFlow Lite 品牌演进为 LiteRT，但微控制器社区和代码仓库仍广泛使用 TFLM 名称。

## 1. TinyML 与边缘计算

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| AI | Artificial Intelligence | 人工智能 | 让机器执行通常需要人类智能的任务，是范围最大的概念。 |
| ML | Machine Learning | 机器学习 | 从数据中学习规则或参数，而不是完全由人手写规则。 |
| DL | Deep Learning | 深度学习 | 使用多层 Neural Network（神经网络）的机器学习方法。 |
| TinyML | Tiny Machine Learning | 微型机器学习 | 在 MCU 等资源受限设备上运行机器学习推理的技术与工程方法。 |
| Edge AI | Edge Artificial Intelligence | 边缘人工智能 | 在数据产生地点附近执行 AI，减少云端依赖、延迟和数据上传。 |
| Embedded ML | Embedded Machine Learning | 嵌入式机器学习 | 将机器学习模型集成到嵌入式系统；覆盖范围通常比 TinyML 更宽。 |
| On-device ML | On-device Machine Learning | 设备端机器学习 | 模型直接在终端设备上运行，不依赖每次请求云端计算。 |
| Cloud inference | Cloud Inference | 云端推理 | 将输入发送到服务器，由云端模型产生预测。 |
| Offline inference | Offline Inference | 离线推理 | 推理时不需要网络连接；TinyML 的常见工作方式。 |
| Real-time inference | Real-time Inference | 实时推理 | 在规定 deadline（截止时间）内持续产生预测，不单指“运行得快”。 |

## 2. 嵌入式硬件与系统

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| MCU | Microcontroller Unit | 微控制器 | 集成 CPU、内存和外设的控制芯片，如 STM32；是 TinyML 的典型目标。 |
| MPU | Microprocessor Unit | 微处理器 | 通常需要外部内存并运行 Linux 等复杂系统，资源普遍多于 MCU。 |
| CPU | Central Processing Unit | 中央处理器 | 执行通用指令的计算核心。 |
| SoC | System on Chip | 片上系统 | 在单芯片中集成处理器、内存控制器、外设和加速器等模块。 |
| DSP | Digital Signal Processor | 数字信号处理器 | 针对乘加、滤波等信号运算优化的处理器或硬件扩展。 |
| NPU | Neural Processing Unit | 神经网络处理器 | 专门加速神经网络算子的硬件单元。 |
| FPU | Floating-Point Unit | 浮点运算单元 | 加速 Float32 等浮点运算；Cortex-M4F 中的 `F` 表示带 FPU。 |
| MAC / MACC | Multiply-Accumulate | 乘加运算 | 先乘后加，是 Dense 和 Convolution 等神经网络层的核心计算。 |
| SIMD | Single Instruction, Multiple Data | 单指令多数据 | 一条指令并行处理多个数据元素，提高向量和神经网络计算效率。 |
| MVE | M-Profile Vector Extension | M 系列向量扩展 | Arm Cortex-M 的向量指令扩展，也称 Helium。 |
| Helium | Arm Helium Technology | Arm Helium 技术 | Arm 对 MVE 向量扩展的品牌名称。 |
| IMU | Inertial Measurement Unit | 惯性测量单元 | 通常集成 accelerometer 和 gyroscope，用于测量运动与姿态。 |
| MEMS | Micro-Electro-Mechanical Systems | 微机电系统 | 用微型机械结构制造传感器的技术，常用于 IMU、麦克风和压力传感器。 |
| Accelerometer | Accelerometer | 加速度计 | 测量一个或多个方向的加速度，静止时也会感受到重力。 |
| Gyroscope | Gyroscope | 陀螺仪 | 测量绕各轴旋转的 angular velocity（角速度）。 |
| Magnetometer | Magnetometer | 磁力计 | 测量磁场，可辅助估计航向。 |
| Sensor fusion | Sensor Fusion | 传感器融合 | 综合多个传感器数据，提高状态或姿态估计的可靠性。 |
| ADC | Analog-to-Digital Converter | 模数转换器 | 把模拟电压转换为数字值。 |
| DMA | Direct Memory Access | 直接存储器访问 | 外设与内存之间直接搬运数据，减少 CPU 干预。 |
| ISR | Interrupt Service Routine | 中断服务程序 | 响应硬件中断的函数，应保持短小并避免耗时推理。 |
| RTOS | Real-Time Operating System | 实时操作系统 | 提供任务、调度、同步和时间管理，并保证可分析的实时行为。 |
| FreeRTOS | Free Real-Time Operating System | FreeRTOS 实时操作系统 | 常用于 MCU 的开源 RTOS；本项目用它组织采样和推理任务。 |
| HAL | Hardware Abstraction Layer | 硬件抽象层 | 用统一接口封装寄存器和外设操作。 |
| BSP | Board Support Package | 板级支持包 | 面向具体开发板的启动、时钟、引脚和外设支持代码。 |
| UART | Universal Asynchronous Receiver-Transmitter | 通用异步收发器 | 常用于串口日志、命令输入和 Renode 测试输出。 |
| SPI | Serial Peripheral Interface | 串行外设接口 | 常用于高速连接 IMU、Flash 和显示器等外设。 |
| I²C / I2C | Inter-Integrated Circuit | 集成电路总线 | 使用时钟线和数据线连接多个低速外设。 |
| GPIO | General-Purpose Input/Output | 通用输入输出 | 可配置为数字输入或输出的引脚。 |
| Flash | Flash Memory | 闪存 | 断电后保留数据，通常存放固件和模型权重。 |
| RAM | Random-Access Memory | 随机存取存储器 | 运行时读写内存，存放 tensor、stack 和临时缓冲区。 |
| SRAM | Static Random-Access Memory | 静态随机存取存储器 | MCU 中常见的 RAM 类型，不需要周期刷新。 |
| Stack | Stack Memory | 栈内存 | 保存函数调用、局部变量和任务上下文；FreeRTOS 每个任务通常有独立 stack。 |
| Heap | Heap Memory | 堆内存 | 运行时动态分配的内存区域；TinyML 常尽量减少动态分配。 |
| Memory footprint | Memory Footprint | 内存占用 | 程序、模型和运行时所消耗的 Flash、RAM 等资源总量。 |
| Clock frequency | Clock Frequency | 时钟频率 | 处理器每秒时钟周期数量；频率高不等于所有模型都按同比例加速。 |
| Latency | Latency | 延迟 | 从输入可用到产生一次结果所需的时间。 |
| Throughput | Throughput | 吞吐量 | 单位时间内可处理的样本或推理次数。 |
| Deadline | Deadline | 截止时间 | 实时任务必须完成处理的最晚时刻。 |
| Power | Power | 功率 | 单位时间消耗能量的速率，常用 W 或 mW 表示。 |
| Energy per inference | Energy per Inference | 单次推理能耗 | 完成一次推理消耗的能量，比单看瞬时功率更能反映模型成本。 |
| Duty cycle | Duty Cycle | 占空比 | 系统处于活动状态的时间比例，会显著影响平均功耗。 |

## 3. 数据与信号处理

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| Sensor reading | Sensor Reading | 传感器读数 | 传感器在某个时刻产生的一组值，例如一组三轴加速度。 |
| Sampling | Sampling | 采样 | 按时间把连续物理信号转换为离散数值。 |
| Sampling rate | Sampling Rate | 采样率 | 每秒采集多少次数据，单位通常为 Hz。 |
| Hz | Hertz | 赫兹 | 每秒发生一次为 1 Hz；50 Hz 表示每秒采样 50 次。 |
| Time series | Time Series | 时间序列 | 按时间顺序排列的数据，如连续 IMU、音频或温度数据。 |
| Channel | Channel | 通道 | 同一时刻的一类测量维度，如加速度 X、Y、Z 三个通道。 |
| Sample / example | Sample / Example | 机器学习样本 | 一条完整模型输入；活动识别中通常是一段窗口，不是单个传感器读数。 |
| Feature | Feature | 特征 | 模型用于判断的信息，可以是原始数据，也可以是人为计算的统计量。 |
| Feature vector | Feature Vector | 特征向量 | 把一条样本的多个特征排成一个向量。 |
| Label | Label | 标签 | 样本的已知正确答案，如 `WALKING`。 |
| Class | Class | 类别 | 分类任务中的候选结果，如走路、坐下和站立。 |
| Dataset | Dataset | 数据集 | 样本及其标签的集合。 |
| Ground truth | Ground Truth | 真实标注 | 被当作正确答案的参考值，用于训练或评估模型。 |
| Annotation | Annotation | 数据标注 | 为原始数据添加类别、位置或数值答案的过程。 |
| Window | Window | 时间窗口 | 从连续信号中截取的一段固定长度数据，作为模型输入。 |
| Window size | Window Size | 窗口长度 | 每个窗口包含的采样点数量或时间长度。 |
| Stride / step size | Stride / Step Size | 滑动步长 | 相邻窗口起点之间移动的采样点数量。 |
| Overlap | Overlap | 窗口重叠 | 相邻窗口共享的数据比例或采样点数量。 |
| Sliding window | Sliding Window | 滑动窗口 | 按固定步长沿时间序列连续产生窗口的方法。 |
| Ring buffer | Ring Buffer | 环形缓冲区 | 首尾相连的固定容量缓冲区，适合持续保存最新传感器数据。 |
| Double buffering | Double Buffering | 双缓冲 | 一个缓冲区采集数据时，另一个缓冲区供推理处理。 |
| Preprocessing | Preprocessing | 预处理 | 在模型前执行的清洗、缩放、滤波或窗口化操作。 |
| Normalization | Normalization | 归一化 | 把数据变换到统一尺度；广义上也常包含标准化。 |
| Min-max scaling | Min-Max Scaling | 最小最大缩放 | 使用最小值与最大值把数据映射到指定范围。 |
| Standardization | Standardization | 标准化 | 通常执行 `(x - mean) / standard deviation`，使数据接近零均值和单位方差。 |
| Mean | Mean | 均值 | 一组数的平均值。 |
| Variance | Variance | 方差 | 描述数据相对均值的离散程度。 |
| Standard deviation | Standard Deviation | 标准差 | 方差的平方根，与原数据具有相同单位。 |
| Noise | Noise | 噪声 | 与目标信息无关的随机或系统性扰动。 |
| Drift | Drift | 漂移 | 传感器输出随时间缓慢偏离真实值的现象。 |
| Bias error | Bias Error | 零偏误差 | 真实输入为零时，传感器仍存在的固定输出偏差；不同于神经网络的 bias 参数。 |
| Filter | Filter | 滤波器 | 选择性保留或衰减某些信号成分。 |
| LPF | Low-Pass Filter | 低通滤波器 | 保留低频成分并衰减高频成分。 |
| HPF | High-Pass Filter | 高通滤波器 | 保留高频成分并衰减低频或直流成分。 |
| FFT | Fast Fourier Transform | 快速傅里叶变换 | 高效计算频谱，把信号从时域转换到频域。 |
| Time domain | Time Domain | 时域 | 按时间观察信号幅值。 |
| Frequency domain | Frequency Domain | 频域 | 按频率观察信号能量或幅值。 |
| Data augmentation | Data Augmentation | 数据增强 | 对训练样本做合理变换，生成更多具有相同语义的数据。 |
| Class imbalance | Class Imbalance | 类别不平衡 | 不同类别样本数量差异明显，可能让模型偏向多数类。 |
| Data distribution | Data Distribution | 数据分布 | 数据值、类别和场景出现的统计规律。 |
| Distribution shift | Distribution Shift | 分布漂移 | 部署数据与训练数据的统计规律不同。 |
| Data leakage | Data Leakage | 数据泄漏 | 训练过程错误地获取了验证集或测试集信息，导致指标虚高。 |

## 4. 机器学习训练基础

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| Model | Machine Learning Model | 机器学习模型 | 从输入映射到输出的函数及其已学习参数。 |
| Training | Model Training | 模型训练 | 使用带标签数据反复调整模型参数的过程。 |
| Inference | Model Inference | 模型推理 | 使用固定模型对新输入产生预测；MCU 主要执行这一阶段。 |
| Prediction | Prediction | 预测 | 模型针对某条输入给出的输出结果。 |
| Supervised learning | Supervised Learning | 监督学习 | 使用带正确标签的数据训练模型。 |
| Unsupervised learning | Unsupervised Learning | 无监督学习 | 在没有显式标签的数据中寻找结构或规律。 |
| Classification | Classification | 分类 | 从有限类别中选择结果，如判断当前活动。 |
| Regression | Regression | 回归 | 预测连续数值，如温度或剩余寿命。 |
| Anomaly detection | Anomaly Detection | 异常检测 | 判断输入是否偏离正常模式。 |
| Parameter | Model Parameter | 模型参数 | 训练中自动学习、推理时固定的数值，包括 weight 和 bias。 |
| Weight | Weight | 权重 | 控制不同输入对输出影响程度的模型参数。 |
| Bias | Bias | 偏置参数 | 神经元在线性组合后额外加入的可训练常数。 |
| Hyperparameter | Hyperparameter | 超参数 | 训练前人为设定的配置，如 learning rate、batch size 和层宽。 |
| Train set | Training Set | 训练集 | 用于计算 loss 并更新模型参数的数据。 |
| Validation set | Validation Set | 验证集 | 用于选择模型和观察过拟合，不直接更新参数。 |
| Test set | Test Set | 测试集 | 用于最终评估泛化能力，不应参与模型选择。 |
| Dataset split | Dataset Split | 数据集划分 | 把数据分成训练、验证和测试集合。 |
| Subject-wise split | Subject-wise Split | 按受试者划分 | 不同受试者进入不同集合，避免相邻人体活动数据泄漏。 |
| Batch | Training Batch | 训练批次 | 一次参数更新所处理的一组训练样本。 |
| Batch size | Batch Size | 批次大小 | 每个 batch 包含的样本数量。 |
| Iteration / step | Training Iteration / Step | 训练迭代 | 处理一个 batch 并通常更新一次参数。 |
| Epoch | Training Epoch | 训练轮次 | 模型完整处理一遍训练集；一个 epoch 通常包含多个 iteration。 |
| Loss | Loss Function | 损失函数 | 衡量单次或一批预测与正确答案差距的函数。 |
| Objective | Objective Function | 目标函数 | 训练希望最小化或最大化的整体函数，通常包含 loss 和正则项。 |
| Optimizer | Optimization Algorithm | 优化器 | 根据 gradient 更新模型参数的算法，如 Adam 或 SGD。 |
| SGD | Stochastic Gradient Descent | 随机梯度下降 | 使用样本或小批次估计 gradient 并更新参数的优化方法。 |
| Adam | Adaptive Moment Estimation | 自适应矩估计 | 根据 gradient 的一阶与二阶统计量自适应调整更新幅度的优化器。 |
| Gradient | Gradient | 梯度 | loss 对参数变化的敏感方向和程度。 |
| Backpropagation | Backpropagation | 反向传播 | 从输出向输入计算各参数 gradient 的方法。 |
| Learning rate | Learning Rate | 学习率 | 控制每次参数更新步幅的超参数。 |
| Convergence | Convergence | 收敛 | 训练指标逐渐稳定、不再明显改善的状态。 |
| Random seed | Random Seed | 随机种子 | 固定伪随机序列起点，提高实验可复现性。 |
| Checkpoint | Model Checkpoint | 模型检查点 | 训练过程中保存的模型参数和状态。 |
| Baseline | Baseline Model | 基线模型 | 用于后续比较的第一个简单、可重复模型。 |
| Generalization | Generalization | 泛化 | 模型对未见数据仍能正确工作的能力。 |
| Overfitting | Overfitting | 过拟合 | 训练集表现很好，但未见数据表现明显较差。 |
| Underfitting | Underfitting | 欠拟合 | 模型连训练数据中的主要规律也没有学好。 |
| Regularization | Regularization | 正则化 | 限制模型复杂度、降低过拟合的方法。 |
| Early stopping | Early Stopping | 提前停止 | 验证指标不再改善时停止训练，避免继续过拟合。 |

## 5. Tensor、数据类型与模型层

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| Tensor | Tensor | 张量 | 多维数组；模型的输入、输出、权重和中间结果都可表示为 tensor。 |
| Scalar | Scalar | 标量 | 只有一个数值，可视为零维 tensor。 |
| Vector | Vector | 向量 | 一维数值数组。 |
| Matrix | Matrix | 矩阵 | 二维数值数组。 |
| Rank | Tensor Rank | 张量阶数 | tensor 具有多少个维度；不要与矩阵的线性代数秩混淆。 |
| Shape | Tensor Shape | 张量形状 | 每个维度的长度，如 `(batch, time, channel)`。 |
| Dimension | Tensor Dimension | 张量维度 | shape 中的一个轴，如时间轴或通道轴。 |
| dtype | Data Type | 数据类型 | tensor 单个元素的类型，如 Float32 或 INT8。 |
| Float32 / FP32 | 32-bit Floating Point | 32 位浮点数 | 常用于训练，每个值占用四个 byte。 |
| Float16 / FP16 | 16-bit Floating Point | 16 位浮点数 | 精度和范围低于 Float32，常用于降低存储或加速。 |
| INT8 | 8-bit Signed Integer | 8 位有符号整数 | 范围为 -128 到 127，是 MCU 量化模型的常见数据类型。 |
| UINT8 | 8-bit Unsigned Integer | 8 位无符号整数 | 范围为 0 到 255；部分旧量化模型使用。 |
| INT16 | 16-bit Signed Integer | 16 位有符号整数 | 精度高于 INT8，但通常占用更多内存和计算资源。 |
| Layer | Neural Network Layer | 神经网络层 | 模型中的一段确定计算。 |
| Neuron | Artificial Neuron | 人工神经元 | 对输入做加权求和并应用 activation 的计算单元。 |
| Activation | Activation Value | 激活值 | 某层计算产生的中间输出；也常泛指 activation function。 |
| Activation function | Activation Function | 激活函数 | 为模型引入非线性，如 ReLU、Sigmoid 和 Softmax。 |
| ReLU | Rectified Linear Unit | 修正线性单元 | 输出 `max(0, x)`，常用于隐藏层。 |
| Sigmoid | Sigmoid Function | S 形函数 | 把数值压缩到 0 与 1 之间，常用于二分类输出。 |
| Softmax | Softmax Function | Softmax 函数 | 把多个输出转换为总和为 1 的类别分布。 |
| Dense / FC | Dense Layer / Fully Connected Layer | 全连接层 | 每个输出神经元连接全部输入。 |
| MLP | Multilayer Perceptron | 多层感知机 | 由多个 Fully Connected layer 组成的前馈神经网络。 |
| CNN | Convolutional Neural Network | 卷积神经网络 | 使用 convolution 提取局部空间或时间模式的网络。 |
| Convolution | Convolution Operation | 卷积运算 | 使用小型 kernel 在输入上滑动并提取局部特征。 |
| Kernel / filter | Convolution Kernel / Filter | 卷积核 | 卷积层中可训练的小型权重数组；不要与软件 kernel 混淆。 |
| Conv1D | One-Dimensional Convolution | 一维卷积 | 沿时间等单一方向滑动，常用于音频和 IMU。 |
| Conv2D | Two-Dimensional Convolution | 二维卷积 | 沿高度和宽度滑动，常用于图像。 |
| Depthwise convolution | Depthwise Convolution | 深度卷积 | 各输入通道分别执行卷积，通常比普通卷积计算量更低。 |
| Pooling | Pooling Operation | 池化 | 汇总局部区域，缩小特征尺寸。 |
| Max pooling | Maximum Pooling | 最大池化 | 取局部区域最大值。 |
| Average pooling | Average Pooling | 平均池化 | 取局部区域平均值。 |
| Flatten | Flatten Layer | 展平层 | 把多维输入重新排列为一维向量，不改变元素数量。 |
| Reshape | Reshape Operation | 形状重排 | 改变 tensor 的 shape，不改变元素数据和总数。 |
| Dropout | Dropout Regularization | 随机失活 | 训练时随机屏蔽部分神经元，降低过拟合；推理时不随机屏蔽。 |
| Input tensor | Input Tensor | 输入张量 | Runtime 接收模型输入的内存区域。 |
| Output tensor | Output Tensor | 输出张量 | Runtime 存放模型结果的内存区域。 |
| Intermediate tensor | Intermediate Tensor | 中间张量 | 模型层之间传递 activation 的临时数据。 |

## 6. 模型评估

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| Metric | Evaluation Metric | 评估指标 | 衡量模型表现的统计量，不一定直接用于更新参数。 |
| Accuracy | Classification Accuracy | 分类准确率 | 正确预测数量除以总样本数量。 |
| Precision | Precision | 查准率 | 被预测为某类的样本中，真正属于该类的比例。 |
| Recall | Recall | 召回率 | 真正属于某类的样本中，被模型找出的比例。 |
| F1 score | F1 Score | F1 分数 | Precision 与 Recall 的调和平均。 |
| Confusion matrix | Confusion Matrix | 混淆矩阵 | 统计真实类别与预测类别组合，用于定位具体混淆。 |
| True positive | True Positive | 真阳性 | 实际为目标类，模型也预测为目标类。 |
| False positive | False Positive | 假阳性 | 实际不是目标类，模型却预测为目标类。 |
| True negative | True Negative | 真阴性 | 实际不是目标类，模型也预测为非目标类。 |
| False negative | False Negative | 假阴性 | 实际为目标类，模型却没有识别出来。 |
| Top-1 accuracy | Top-1 Accuracy | 首选准确率 | 模型最高分的类别与真实类别一致的比例。 |
| Top-k accuracy | Top-k Accuracy | 前 k 项准确率 | 真实类别出现在模型分数最高的 k 个类别中的比例。 |
| Probability | Probability | 概率 | 事件发生可能性的数值表示；模型输出分数不一定总是校准良好的真实概率。 |
| Confidence score | Confidence Score | 置信分数 | 模型对某个预测的相对确信程度，不等同于结果必然正确。 |
| Threshold | Decision Threshold | 决策阈值 | 把连续分数转换为类别或触发事件的边界。 |
| ROC | Receiver Operating Characteristic | 接收者操作特征曲线 | 展示不同 threshold 下 true positive rate 与 false positive rate 的关系。 |
| AUC | Area Under the Curve | 曲线下面积 | 对 ROC 等曲线的汇总指标。 |
| Cross-validation | Cross-Validation | 交叉验证 | 使用多组数据划分重复训练和验证，评估稳定性。 |

## 7. 量化、压缩与部署

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| Quantization | Model Quantization | 模型量化 | 用较低精度数值表示 weight 和 activation，以减少资源消耗。 |
| PTQ | Post-Training Quantization | 训练后量化 | Float 模型训练完成后再转换为低精度模型。 |
| QAT | Quantization-Aware Training | 量化感知训练 | 训练时模拟量化误差，使模型主动适应低精度计算。 |
| Dynamic range quantization | Dynamic Range Quantization | 动态范围量化 | 通常离线量化 weight，运行时动态处理 activation；不等于 Full INT8。 |
| Full integer quantization | Full Integer Quantization | 全整数量化 | weight、activation、输入和输出均可使用整数计算。 |
| Representative dataset | Representative Dataset | 代表性数据集 | 转换器用于估计 activation 范围的一小批典型训练数据。 |
| Calibration | Quantization Calibration | 量化校准 | 使用代表性数据估计 tensor 的数值范围和量化参数。 |
| Scale | Quantization Scale | 量化缩放系数 | 描述相邻整数值代表多少实数变化。 |
| Zero-point | Quantization Zero-Point | 量化零点 | 表示实数零对应的整数值。 |
| Symmetric quantization | Symmetric Quantization | 对称量化 | 正负范围围绕零近似对称，zero-point 通常为零。 |
| Asymmetric quantization | Asymmetric Quantization | 非对称量化 | 使用非零 zero-point 更灵活地覆盖偏移数值范围。 |
| Per-tensor quantization | Per-Tensor Quantization | 逐张量量化 | 整个 tensor 共用一组 scale 和 zero-point。 |
| Per-channel quantization | Per-Channel Quantization | 逐通道量化 | 不同通道使用不同 scale，常提高卷积 weight 的精度。 |
| Quantize | Quantize | 量化转换 | 按 scale 和 zero-point 把实数转换为整数表示。 |
| Dequantize | Dequantize | 反量化 | 把量化整数按参数还原为近似实数。 |
| Clipping | Clipping | 截断 | 把超出允许范围的数限制到边界值。 |
| Saturation | Saturating Arithmetic | 饱和运算 | 数值溢出时停在最大或最小值，而不是回绕。 |
| Pruning | Model Pruning | 模型剪枝 | 删除影响较小的 weight 或连接，降低模型复杂度。 |
| Sparsity | Sparsity | 稀疏性 | 参数中零值所占的程度；需要 Runtime 支持才能充分加速。 |
| Knowledge distillation | Knowledge Distillation | 知识蒸馏 | 使用较大 teacher model 指导较小 student model 训练。 |
| Teacher model | Teacher Model | 教师模型 | 蒸馏中提供软标签或中间知识的大模型。 |
| Student model | Student Model | 学生模型 | 蒸馏中学习教师行为、面向部署的小模型。 |
| Model conversion | Model Conversion | 模型转换 | 把训练框架模型转换为设备 Runtime 支持的格式。 |
| Converter | Model Converter | 模型转换器 | 执行模型格式转换和量化的工具。 |
| Compute graph | Computation Graph | 计算图 | 描述 tensor 如何经过 operator 连接并产生输出。 |
| Operator / Op | Model Operator | 模型算子 | 计算图中的操作类型，如 FullyConnected、Reshape 或 Softmax。 |
| Kernel | Operator Kernel | 算子内核 | 某个 operator 针对特定平台和数据类型的底层实现。 |
| Reference kernel | Reference Kernel | 参考算子内核 | 优先保证清晰和正确性的通用实现，性能通常不是最优。 |
| Optimized kernel | Optimized Kernel | 优化算子内核 | 针对 SIMD、DSP 或 MVE 等硬件能力优化的实现。 |
| Supported Ops | Supported Operators | 支持算子集合 | Runtime 能执行的 operator 及版本范围。 |
| Model size | Model Size | 模型大小 | 模型文件占用的存储空间，主要受 weight、metadata 和结构影响。 |
| Compression ratio | Compression Ratio | 压缩比 | 压缩前后模型大小的比例。 |
| Benchmark | Performance Benchmark | 性能基准测试 | 在固定条件下测量 latency、throughput 或资源占用。 |
| Profiling | Performance Profiling | 性能剖析 | 记录各 operator 或代码区域的时间和资源开销。 |
| Golden Test | Golden Reference Test | 黄金参考测试 | 使用固定输入和已知输出验证不同 Runtime 或平台结果。 |
| Bit-exact | Bit-Exact Result | 位精确一致 | 输出的每一位都完全相同，比只要求类别一致更严格。 |
| Deterministic | Deterministic Behavior | 确定性行为 | 相同条件和输入总产生相同结果。 |

## 8. LiteRT、TFLM 与 Arm 优化

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| TensorFlow | TensorFlow | TensorFlow 机器学习框架 | 用于定义、训练和转换模型的开源框架。 |
| Keras | Keras | Keras 高层 API | TensorFlow 中常用的高层模型构建与训练接口。 |
| TFLite | TensorFlow Lite | TensorFlow Lite | LiteRT 的旧品牌名称；`.tflite` 文件扩展名和大量 API 名称仍保留。 |
| LiteRT | Lite Runtime | LiteRT 设备端运行时 | Google 当前的 on-device ML Runtime 品牌，承接 TensorFlow Lite。 |
| TFLM | TensorFlow Lite for Microcontrollers | 微控制器版 TensorFlow Lite | 面向 MCU 的小型 C++ Runtime；当前官方代码仓库仍为 `tensorflow/tflite-micro`。 |
| LiteRT for Microcontrollers | Lite Runtime for Microcontrollers | 微控制器版 LiteRT | Google 当前文档使用的微控制器 Runtime 名称，与 TFLM 演进关系紧密。 |
| FlatBuffers | FlatBuffers Serialization Library | FlatBuffers 序列化库 | `.tflite` 模型使用的结构化二进制格式。 |
| `.tflite` | TensorFlow Lite Model File | LiteRT 模型文件 | 保存 computation graph、weight、tensor 和 quantization 参数的文件。 |
| Schema | Model File Schema | 模型文件模式 | 规定 `.tflite` 文件中各字段的结构和版本。 |
| Runtime | Machine Learning Runtime | 机器学习运行时 | 在目标平台加载模型并调度 operator 执行的软件。 |
| Interpreter | Model Interpreter | 模型解释器 | 读取 computation graph、准备 tensor 并依次执行 operator 的对象。 |
| MicroInterpreter | TensorFlow Lite Micro Interpreter | TFLM 微型解释器 | TFLM 中负责模型初始化、内存规划和 `Invoke()` 的核心类。 |
| OpResolver | Operator Resolver | 算子解析器 | 把模型中的 operator 映射到对应 kernel。 |
| MicroMutableOpResolver | Micro Mutable Operator Resolver | 微型可配置算子解析器 | 显式注册模型所需 operator，避免链接全部 kernel。 |
| Tensor Arena | Tensor Arena | 张量内存池 | TFLM 使用的一块预分配连续 RAM，存放 tensor 和 Runtime 数据。 |
| `AllocateTensors()` | Allocate Tensors | 分配张量 | 让 TFLM 在 Tensor Arena 内规划模型运行时内存。 |
| `Invoke()` | Invoke Model | 执行模型 | 让 Interpreter 对当前 input tensor 完成一次推理。 |
| CMSIS | Cortex Microcontroller Software Interface Standard | Cortex 微控制器软件接口标准 | Arm 为 Cortex-M 提供的核心、DSP、NN 和 RTOS 等标准接口集合。 |
| CMSIS-NN | Cortex Microcontroller Software Interface Standard Neural Network | Cortex-M 神经网络优化库 | 提供适配 Cortex-M 的高效 INT8/INT16 neural network kernel。 |
| CMSIS-DSP | Cortex Microcontroller Software Interface Standard Digital Signal Processing | Cortex-M 数字信号处理库 | 提供滤波、FFT、矩阵等优化信号处理函数。 |

## 9. 构建、固件与仿真工具

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| Firmware | Firmware | 固件 | 编译后运行在 MCU 上的软件。 |
| Toolchain | Software Toolchain | 工具链 | 从源代码到可执行固件所需的编译器、链接器和构建工具集合。 |
| GCC | GNU Compiler Collection | GNU 编译器集合 | 开源编译器套件。 |
| Arm GNU Toolchain | Arm GNU Toolchain | Arm GNU 工具链 | Arm 官方提供的 GCC、linker、GDB 等交叉开发工具。 |
| `arm-none-eabi` | Arm Embedded Application Binary Interface Target | Arm 裸机嵌入式目标 | 面向 Arm 裸机系统的工具链 target triplet。 |
| ABI | Application Binary Interface | 应用二进制接口 | 规定函数调用、寄存器、数据布局和目标文件兼容方式。 |
| EABI | Embedded Application Binary Interface | 嵌入式应用二进制接口 | 面向嵌入式系统的 ABI 规范。 |
| Cross-compilation | Cross Compilation | 交叉编译 | 在 PC 上生成给另一种架构运行的代码。 |
| CMake | CMake Build System | CMake 构建系统 | 根据 `CMakeLists.txt` 生成 Ninja 等具体构建规则；CMake 不是官方缩写。 |
| Ninja | Ninja Build System | Ninja 构建系统 | 执行 CMake 生成的编译任务，重点追求快速增量构建。 |
| Compiler | Compiler | 编译器 | 把 C/C++ 源代码转换为目标代码。 |
| Linker | Linker | 链接器 | 合并目标文件和库，分配地址并生成 ELF。 |
| Linker script | Linker Script | 链接脚本 | 描述 Flash、RAM 和各 section 放置位置。 |
| Section | Binary Section | 二进制段 | ELF 中按用途分组的代码或数据，如 `.text`、`.data` 和 `.bss`。 |
| ELF | Executable and Linkable Format | 可执行与可链接格式 | 保存固件代码、数据、地址和调试符号的文件格式。 |
| Map file | Linker Map File | 链接映射文件 | 记录 symbol 和 section 的地址、大小与来源，用于分析内存。 |
| Symbol | Binary Symbol | 二进制符号 | 函数或变量在目标文件中的名称与地址信息。 |
| Static library | Static Library | 静态库 | 链接时把需要的代码复制进最终 ELF 的库，常见扩展名为 `.a`。 |
| Debug build | Debug Build | 调试构建 | 保留调试信息并通常降低优化，方便单步和定位问题。 |
| Release build | Release Build | 发布构建 | 通常启用优化并减少调试开销，用于性能与体积评估。 |
| Optimization level | Compiler Optimization Level | 编译优化级别 | 控制编译器在速度、体积和可调试性之间的取舍。 |
| Renode | Renode Framework | Renode 仿真框架 | 对 CPU、内存和外设进行功能级仿真，可运行目标 ELF。 |
| Emulation | Hardware Emulation | 硬件仿真 | 用软件模拟目标硬件行为并运行目标指令。 |
| Simulation | System Simulation | 系统模拟 | 广义的软件模型运行；在 Renode 语境中常与 emulation 混用。 |
| `.repl` | Renode Platform Description File | Renode 平台描述文件 | 描述 CPU、内存映射、外设和连接关系；此处不是交互式解释器。 |
| RESC | Renode Script | Renode 脚本 | `.resc` 文件描述创建机器、加载 ELF 和启动仿真的命令。 |
| Robot Framework | Robot Framework | Robot 自动测试框架 | 可控制 Renode 并断言 UART、网络等外设行为。 |
| GDB | GNU Debugger | GNU 调试器 | 支持断点、单步、寄存器和内存检查。 |
| UART Analyzer | Universal Asynchronous Receiver-Transmitter Analyzer | UART 分析器 | Renode 中显示或交互 UART 数据的窗口。 |
| HardFault | Hard Fault Exception | 硬故障异常 | Cortex-M 无法由其他 fault 处理的严重异常。 |
| CI | Continuous Integration | 持续集成 | 每次代码变更后自动构建和测试。 |
| Reproducibility | Reproducibility | 可复现性 | 使用相同代码、数据和版本能够重新得到一致或近似结果。 |
| Version pinning | Version Pinning | 版本固定 | 明确锁定依赖版本或 commit，避免环境随时间变化。 |
| Commit SHA | Git Commit Hash | Git 提交哈希 | Git commit 的内容标识，常用于精确固定 TFLM 代码版本。 |

## 10. 当前应用相关术语

| 常用写法 | English full name | 中文名称 | 精简解释 |
|---|---|---|---|
| HAR | Human Activity Recognition | 人体活动识别 | 根据 IMU、视频等数据判断走路、坐下等人体活动。 |
| ADL | Activities of Daily Living | 日常生活活动 | 走路、站立、坐下等日常行为类别。 |
| UCI | University of California, Irvine | 加州大学欧文分校 | UCI Machine Learning Repository 的维护机构名称来源。 |
| UCI HAR | UCI Human Activity Recognition Dataset | UCI 人体活动识别数据集 | 使用智能手机 accelerometer 和 gyroscope 采集的人体活动数据集。 |
| Subject | Human Subject | 受试者 | 参与数据采集的人；按 subject 划分可评估对陌生人的泛化能力。 |
| Activity segment | Activity Segment | 活动片段 | 标签相同的一段连续传感器数据。 |
| Transition activity | Transition Activity | 姿态转换活动 | 从一种稳定状态切换到另一种状态的短暂动作。 |

## 11. 容易混淆的术语

| 容易混淆项 | 区别 |
|---|---|
| Sensor reading 与 ML sample | reading 是一个采样时刻的传感器值；sample 是一条完整模型输入，可能包含很多 reading。 |
| Batch 与 epoch | batch 是一次参数更新处理的一组样本；epoch 是完整处理一遍训练集。 |
| Training 与 inference | training 会更新 weight；inference 使用固定 weight 产生预测。 |
| Parameter 与 hyperparameter | parameter 由训练学习；hyperparameter 由工程师在训练前设置。 |
| Weight bias 与 sensor bias | 前者是模型参数；后者是传感器零偏误差。 |
| Convolution kernel 与 operator kernel | 前者是模型中的可训练权重；后者是执行 operator 的软件实现。 |
| Tensor rank 与 matrix rank | tensor rank 是维度数量；matrix rank 是线性独立性概念。 |
| Accuracy 与 confidence | accuracy 是一批数据的统计指标；confidence 是单条预测的输出分数。 |
| Latency 与 throughput | latency 是完成单次任务的时间；throughput 是单位时间完成多少任务。 |
| Power 与 energy | power 是能量消耗速率；energy 是一段任务实际消耗的能量。 |
| Model size 与 Tensor Arena | model size 主要占 Flash；Tensor Arena 是推理时的 RAM 工作区。 |
| Quantization 与 compression | quantization 是降低数值精度；compression 还包括 pruning、编码等更广方法。 |
| TFLite、LiteRT 与 TFLM | TFLite 是旧品牌和保留格式名；LiteRT 是当前设备端品牌；TFLM 是 MCU Runtime 的传统名称。 |
| `.repl` 与交互式 REPL | Renode 的 `.repl` 是平台描述文件，不是 Read-Eval-Print Loop。 |
| Emulation latency 与真机 latency | 仿真时间受宿主机和仿真模型影响，不能直接当作真实硬件性能。 |

## 12. 官方参考

- Google Machine Learning Glossary  
  <https://developers.google.com/machine-learning/glossary>
- Google LiteRT Post-training Quantization  
  <https://developers.google.com/edge/litert/conversion/tensorflow/quantization/post_training_quantization>
- Google LiteRT for Microcontrollers  
  <https://ai.google.dev/edge/litert/microcontrollers/overview>
- TensorFlow Lite Micro 官方仓库  
  <https://github.com/tensorflow/tflite-micro>
- Arm CMSIS-NN  
  <https://arm-software.github.io/CMSIS-NN/latest/>
- Renode Documentation  
  <https://renode.readthedocs.io/en/latest/>

自检问题：**一个 IMU 以 50 Hz 采样，模型窗口包含 128 个 sensor reading；这里的一个 ML sample 指单次 reading，还是完整窗口？**
