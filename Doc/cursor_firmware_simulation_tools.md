# Firmware simulation tools
_Exported on 8/26/2026 at 09:59:44 GMT+8 from Cursor (3.5.17)_

---

**User**

现在我这套固件如果我手边没有硬件（如人在外面），有什么办法能够仿真验证功能吗，我刚才了解到了woiki， renode等仿真工具，这些工具能达到目的吗？

---

**Cursor**

可以仿真，但不能指望一个工具把整机“无硬件验收”。你说的应该是 **Wokwi**。

你的固件是 **STM32F401RC + Arduino STM32 + Marlin**，还使用了 SPI（Serial Peripheral Interface，串行外设接口）、EXTI（External Interrupt，外部中断）和 DMA（Direct Memory Access，直接存储器访问）：

```741:753:e:\GiteeSpace\firmware-copy\firmware-dev\ini\stm32f4.ini
[env:JAKE401]
extends                     = stm32_variant
platform                    = ststm32@~14.1.0
board                       = marlin_STM32F401RC
...
upload_protocol             = jlink
```

### Wokwi：目前不适合直接跑这套固件

Wokwi 官方目前只支持少数 STM32，包括 STM32F103C8、STM32C031、STM32L031，**不支持 STM32F401RC/Cortex-M4**。

Custom Chips API 可以模拟 CS5552 这样的外设，却不能补出完整 STM32F401 MCU。因此若强行把工程迁移到 F103，验证的是“移植版”，而非当前固件，意义有限。

结论：Wokwi 适合简单 Arduino 外设演示，不适合作为本工程主仿真平台。

### Renode：更接近目标，但需要自己搭模型

Renode 支持 STM32F4 的 CPU、GPIO、UART、SPI、Timer 等基础模型，可以直接加载编译出来的 ELF（Executable and Linkable Format，可执行与可链接格式），适合验证：

- 固件启动和异常；
- UART/G-code 通信；
- GPIO、限位开关输入；
- SPI 命令帧；
- 部分中断及运动状态机；
- 自动化回归测试。

但当前 CS5552 路径并不简单：

```122:136:e:\GiteeSpace\firmware-copy\firmware-dev\Marlin\src\module\pressure\CS5552.cpp
extern "C" void DMA2_Stream0_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_spi1_rx); }
extern "C" void DMA2_Stream5_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_spi1_tx); }

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI1 && s_dma_busy) CS5552_DMA_SampleComplete();
}
```

它依赖：

- SPI1；
- PA6 同时作为 MISO 和数据就绪信号；
- EXTI 下降沿；
- DMA2 Stream0/Stream5；
- CS5552 连续采样协议。

Renode 没有现成的 CS5552 模型，而且 STM32F4 的高级 Timer/DMA/引脚复用仿真并不完整。要跑通这条链路，需要：

1. 编写 JAKE401 的 `.repl` 板级描述；
2. 编写 CS5552 外设模型；
3. 补齐或绕开 SPI1 DMA、EXTI 与 Alternate Function（复用功能）行为。

所以 Renode“能做到一部分”，但搭建成本不低，尤其无法证明 DMA 时序在真芯片上一模一样。

### 最推荐：分层验证

1. **PlatformIO 编译检查**  
   验证当前 JAKE401 配置、宏和链接是否正确。

2. **Native 单元测试——优先投入**  
   工程已经自带 `linux_native_test` 和 Windows Native Simulator：

```15:36:e:\GiteeSpace\firmware-copy\firmware-dev\ini\native.ini
[env:linux_native]
platform         = native
...
[env:linux_native_test]
extends          = env:linux_native
...
test_build_src   = true
```

把 ADC 输入抽象成接口，通过合成压力曲线验证：

- 接触 FEP（Fluorinated Ethylene Propylene，氟化乙烯丙烯）检测；
- 峰值搜索；
- 稳定窗口；
- stall、超时和急停；
- `pressureMove` 状态迁移；
- G-code 参数和边界条件。

这对你现在正在开发的压力运动逻辑最有效，测试速度也最快。

3. **Marlin Native Simulator**  
   验证 G-code、Planner、Stepper、LCD 和打印流程。  
   但 `CS5552.cpp` 直接调用 STM32 HAL（Hardware Abstraction Layer，硬件抽象层），需要先隔离硬件驱动，才能在 Native 环境注入虚拟压力数据。

4. **Renode 做少量目标级测试**  
   用于验证 ELF 启动、UART、SPI 命令顺序和异常处理，不建议一开始就追求完整机械闭环。

5. **最终仍需真机 HIL**  
   HIL（Hardware-in-the-Loop，硬件在环）负责验证以下仿真无法可信覆盖的项目：

- CS5552 电气时序和 DMA 丢样；
- ADC 噪声、漂移与校准；
- 电机惯性、膜粘附和机械回弹；
- EMI（Electromagnetic Interference，电磁干扰）；
- 急停和限位安全性。

一句话建议：**Native Mock 测算法，Renode 测固件与外设交互，真机测电气和机械。Wokwi 暂时跳过。**

小测试：如果 Native 测试中压力曲线完全正确，能否据此证明 PA6 的 EXTI+SPI DMA 在真机上不会丢样？答案是：不能。

---

**User**

介绍一下renote，我完全不了解，到我能上手使用，需要哪些知识和步骤

---

**Cursor**

你说的应该是 **Renode**。它不是“电子电路仿真器”，而是一台可编程的“虚拟开发板”：运行真实 MCU 固件，并用软件模型模拟 CPU、内存和外设。

## 1. Renode 到底模拟什么

Renode 主要由四层组成：

```text
你的 firmware.elf
       ↓
虚拟 Cortex-M4 CPU
       ↓
STM32F4 外设模型：GPIO / UART / SPI / Timer / DMA
       ↓
虚拟外设：传感器、ADC、显示屏、网络设备
```

它与 Proteus、SPICE 的区别是：

- **Renode**：寄存器级、行为级仿真，重点验证软件。
- **Proteus/SPICE**：更偏电路、电压、电流和波形。
- **Native 单元测试**：直接把业务代码编译成电脑程序，不模拟 MCU。

Renode 通常直接运行真实的 ELF（Executable and Linkable Format，可执行与可链接格式），不需要把固件改成 Windows 程序。这是它最大的价值。

但它不会自然产生 ADC 噪声、电机惯性、FEP 膜粘附力——除非你把这些行为写成模型。

---

## 2. Renode 的三个核心文件

### `.repl`：描述“开发板”

REPL（Renode Platform Description，Renode 平台描述）定义：

- CPU 类型；
- Flash、RAM 地址和大小；
- UART、SPI、Timer、DMA；
- 中断号；
- 外设之间的连接。

可以把它理解成“原理图 + STM32 Reference Manual 的地址表”。

你的芯片参数是：

```23:26:e:\GiteeSpace\firmware-copy\firmware-dev\buildroot\share\PlatformIO\boards\marlin_STM32F401RC.json
"name": "STM32F401RC (64k RAM. 256k Flash)",
"maximum_ram_size": 65536,
"maximum_size": 262144,
```

最终需要一份类似 `jake401.repl` 的板级描述。

### `.resc`：描述“怎么启动”

RESC（Renode Script，Renode 脚本）负责：

- 创建虚拟机器；
- 加载 `.repl`；
- 加载 `firmware.elf`；
- 打开 UART；
- 启动 GDB（GNU Debugger，GNU 调试器）服务；
- 开始运行。

概念上类似：

```resc
mach create "JAKE401"
machine LoadPlatformDescription @jake401.repl
sysbus LoadELF @../.pio/build/JAKE401/firmware.elf
showAnalyzer sysbus.usart1
machine StartGdbServer 3333
start
```

### `.robot`：自动化测试

Robot Framework 用来执行自动回归测试，例如：

1. 启动固件；
2. 等待 UART 输出；
3. 发送 `M115`；
4. 判断返回内容；
5. 模拟限位开关；
6. 检查运动或错误处理结果。

---

## 3. 上手需要哪些知识

不用全部学完再开始，容易把“学工具”学成第二职业。

### 第一阶段必须掌握

1. **基本嵌入式概念**
   - Flash、RAM；
   - GPIO；
   - 中断；
   - UART；
   - SPI；
   - Timer。

2. **STM32 内存映射**
   知道外设是通过固定地址寄存器访问的。例如固件读写 SPI1 寄存器时，Renode 会把访问转给虚拟 SPI1。

3. **ELF 与 BIN 的区别**
   - BIN：只有机器码；
   - ELF：包含机器码、段地址、函数符号和调试信息。

   Renode 优先使用 ELF，因为可以打断点、看函数名和变量。

4. **Renode Monitor**
   Monitor 是 Renode 的命令行控制台，最常用命令：

   ```text
   help
   mach create
   machine LoadPlatformDescription
   sysbus LoadELF
   showAnalyzer
   start
   pause
   reset
   quit
   ```

### 第二阶段需要掌握

5. **GDB 调试**
   Renode 可以像 J-Link 一样提供 GDB Server：

   ```text
   machine StartGdbServer 3333
   ```

   然后使用 ARM 工具链连接：

   ```powershell
   arm-none-eabi-gdb firmware.elf
   ```

   GDB 内执行：

   ```gdb
   target remote :3333
   break setup
   continue
   ```

6. **STM32 Reference Manual**
   遇到固件停死时，需要判断：

   - 是否在等待某个寄存器位；
   - Renode 是否实现了该寄存器；
   - 中断有没有连接；
   - 时钟是否真正启动。

### 后期才需要

7. **C#**
   Renode 外设模型主要使用 C#。模拟 CS5552 时才需要，不是入门门槛。

8. **Robot Framework**
   自动化回归阶段再学。

---

## 4. 推荐上手步骤

### 步骤一：安装并运行官方示例

Windows 可以下载安装包，也可以使用免安装压缩包：

- [Renode Releases](https://github.com/renode/renode/releases/latest)
- [官方安装文档](https://renode.readthedocs.io/en/latest/introduction/installing.html)

启动 Renode 后，在 Monitor 中运行官方 STM32F4 示例：

```text
start @scripts/single-node/stm32f4_discovery.resc
```

此时目标只是弄懂：

- 日志窗口；
- Monitor 窗口；
- UART Analyzer；
- `start`、`pause`、`reset`。

不要一上来就塞入完整 Marlin，否则第一天大概率会得到一台非常安静的虚拟砖头。

### 步骤二：运行最小 STM32F4 UART 程序

先准备一个仅完成以下工作的 STM32F4 程序：

1. 初始化时钟；
2. 初始化 USART1；
3. 循环输出文本。

确认 Renode 能够：

- 正确进入 `main()`；
- 输出 UART；
- 响应 GDB 断点；
- 正确运行 `millis()` 或 SysTick。

这一步是验证虚拟平台基础是否正确。

### 步骤三：建立 JAKE401 平台

基于 Renode 自带的 STM32F4 CPU 描述，补充：

- STM32F401RC 的 Flash/RAM；
- USART1；
- GPIO A～D；
- SPI1；
- Timer；
- EXTI；
- DMA2；
- NVIC（Nested Vectored Interrupt Controller，嵌套向量中断控制器）。

你的串口配置已经明确：

```84:97:e:\GiteeSpace\firmware-copy\firmware-dev\Marlin\src\module\pressure\Configuration.h
#define SERIAL_PORT 1
...
#define BAUDRATE 115200
```

因此第一个验收点是：固件启动后，USART1 能输出 Marlin 启动信息并响应 `M115`。

### 步骤四：加载当前固件

当前 PlatformIO 构建后，通常会生成：

```text
.pio/build/JAKE401/firmware.elf
```

将其加载到 JAKE401 虚拟平台，依次排查：

1. Reset_Handler 是否执行；
2. 是否进入 `main()`；
3. 时钟初始化是否成功；
4. SysTick 是否计时；
5. USART1 是否输出；
6. Marlin 是否进入主循环；
7. G-code 是否能够收发。

先不要启用 CS5552。把“Marlin 能活着说话”作为第一阶段成功标准。

### 步骤五：模拟简单输入输出

接着验证：

- 虚拟限位开关；
- Emergency Pause；
- STEP/DIR 输出；
- Heater/Fan GPIO；
- G-code 状态变化。

这些功能通常不需要编写复杂外设模型。

### 步骤六：处理 CS5552

你的 CS5552 链路包含：

```498:527:e:\GiteeSpace\firmware-copy\firmware-dev\Marlin\src\module\pressure\CS5552.cpp
hdma_spi1_rx.Instance = DMA2_Stream0;
...
hdma_spi1_tx.Instance = DMA2_Stream5;
...
__HAL_LINKDMA(hspi, hdmarx, hdma_spi1_rx);
__HAL_LINKDMA(hspi, hdmatx, hdma_spi1_tx);
...
HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
```

这是整个仿真的困难部分。建议分三次打通：

1. **阻塞 SPI**
   模拟 CS5552 寄存器读写和命令响应。

2. **数据就绪**
   模拟 PA6/SDO 下降沿和 EXTI。

3. **DMA 连续采样**
   验证 DMA 完成中断、环形缓冲区以及丢样恢复。

若 Renode 内置 STM32F4 DMA 模型无法覆盖当前连接，需要：

- 扩展 Renode 模型；或者
- 为仿真构建提供简化的阻塞采样后端。

后者更快，但不能证明真实 DMA 链路正确。

### 步骤七：加入自动化测试

最后用 Robot Framework 编写场景：

```text
启动固件
→ 等待 Marlin Ready
→ 发送 M553 初始化
→ 注入压力数据
→ 发送运动命令
→ 验证状态和 UART 输出
```

Renode 官方提供 `Write Line To Uart`、`Wait For Line On Uart` 等测试关键字，很适合 G-code 回归。

---

## 5. 适合你的学习路线

建议按下面顺序推进：

1. 官方 STM32F4 Demo；
2. 最小 UART 固件；
3. GDB 单步和断点；
4. JAKE401 `.repl`；
5. 启动完整 Marlin；
6. GPIO/限位/步进验证；
7. CS5552 阻塞 SPI 模型；
8. EXTI 与 DMA；
9. Robot 自动化。

第一阶段目标不要定成“完整模拟打印机”，而是：

> 在 Renode 中启动当前 ELF，通过虚拟 USART1 发送 `M115` 并得到正确响应。

做到这一点，就真正入门了。

小测试：在 Renode 中，描述“板上有哪些外设以及地址”的是 `.repl`，还是负责加载固件并启动机器的 `.resc`？答案：前者是 `.repl`，后者是 `.resc`。

---

**User**

GDB调试是什么

---

**Cursor**

GDB 调试就是：**让程序暂停在指定位置，然后查看它此刻正在做什么。**

GDB 全称是 **GNU Debugger（GNU 调试器）**，常用于调试 C/C++ 程序和嵌入式固件。

## 它能做什么

假设固件在 `pressureMoveUpdate()` 中行为异常，GDB 可以：

- 在函数入口设置断点；
- 程序运行到这里时自动暂停；
- 查看变量、数组和结构体；
- 查看函数调用链；
- 单步执行每行代码；
- 观察某个变量何时被修改；
- 检查 HardFault、死循环和非法内存访问。

常见操作：

| 操作 | 含义 |
|---|---|
| Breakpoint | 运行到指定位置暂停 |
| Step Into | 进入当前调用的函数 |
| Step Over | 执行当前行但不进入函数 |
| Continue | 继续运行 |
| Watch | 持续显示变量 |
| Watchpoint | 变量被修改时暂停 |
| Call Stack | 查看函数是如何一层层调用进来的 |
| Register | 查看 CPU 寄存器 |
| Memory | 查看指定内存地址 |

## 嵌入式 GDB 如何工作

电脑不能直接控制 STM32，因此需要三部分：

```text
Cursor / PlatformIO 调试界面
          ↓
arm-none-eabi-gdb
          ↓ GDB Remote Protocol
JLinkGDBServer 或 Renode
          ↓
真实 STM32 或虚拟 STM32
```

### 使用真实硬件

你的工程已经配置了 J-Link：

```755:768:e:\GiteeSpace\firmware-copy\firmware-dev\ini\stm32f4.ini
debug_tool                  = jlink
debug_port                  = :2331
debug_server                =
  E:/Program Files/SEGGER/JLink_V632/JLinkGDBServer.exe
...
  -device
  STM32F401RC
```

工作过程是：

1. J-Link 通过 SWD（Serial Wire Debug，串行线调试）连接 STM32；
2. `JLinkGDBServer` 控制芯片；
3. GDB 连接 `2331` 端口；
4. Cursor 将 GDB 信息显示为断点、变量和调用栈。

### 使用 Renode

没有硬件时，Renode 代替 J-Link 和 STM32：

```text
GDB → Renode GDB Server → 虚拟 Cortex-M4
```

在 Renode 中启动服务：

```text
machine StartGdbServer 3333
```

然后 GDB 连接：

```gdb
target remote :3333
```

因此上层调试体验基本相同，只是一个控制真芯片，一个控制虚拟芯片。

## 一个典型调试过程

假设压力打印流程卡住：

1. 在 `pressureMoveUpdate()` 设置断点；
2. 启动调试；
3. 发送对应 G-code；
4. 程序运行到断点后暂停；
5. 查看状态变量、压力值和 `planner` 状态；
6. 使用 Step Over 逐行执行；
7. 查看程序进入了哪个条件分支；
8. 根据 Call Stack 判断是谁调用了它。

这比连续添加串口打印更直接，因为它能在某个瞬间“冻结现场”。

## 为什么需要 ELF

GDB 通常加载 `firmware.elf`，因为 ELF 内包含：

- 机器指令；
- 函数名；
- 变量地址；
- 源文件和行号；
- DWARF 调试信息。

你的构建参数包含 `-g3`，表示生成较完整的调试信息。不过工程同时使用优化，某些变量可能显示为：

```text
<optimized out>
```

这表示编译器已删除、合并或重排变量，不是 GDB 闹脾气，而是优化后的代码已经和源码不再严格一一对应。

## GDB 不能证明什么

GDB 能告诉你：

> 程序为什么走到了这个分支。

但不能单独证明：

- ADC 电压是否准确；
- SPI 波形是否符合芯片要求；
- 电机是否真的移动；
- 机械压力是否真实；
- DMA 是否受到真实总线竞争影响。

简而言之：**GDB 是程序的“暂停、回放和现场勘查工具”，不是示波器。**

小测试：如果固件发生 HardFault，你应该优先查看串口最后一条日志，还是 GDB 的 Call Stack 和 CPU 寄存器？核心答案：后两者。

