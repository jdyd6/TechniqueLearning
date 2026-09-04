# 从零搭建 Renode 仿真项目

> 面向：已经会写 STM32 固件、但还没碰过指令级仿真的人。  
>
> 目标芯片：STM32F401RCT6。  
>
> 本机 Renode 安装位置：`E:\ProgramFiles\Renode`。  
>
> 配套最小工程：同目录 `demo-stm32f401rc\`。  
>
> 选型对比见：`嵌入式代码快速仿真工具调研.md`。

读完并跑通 demo 之后，你应当能独立完成四件事：装仿真器、描述一块板、加载一段固件、用串口输出判断「它在跑」。

---

## 0. 先建立三个文件的心智模型

一个 Renode 工程不是 Keil 那种「点一下 Debug」。它由三样东西拼起来：

| 文件 | 扩展名 | 干什么 |
| --- | --- | --- |
| 平台描述 | `.repl`（Renode Platform，平台描述） | 告诉仿真器：有哪颗 CPU、Flash/SRAM 多大、USART 在哪个地址 |
| 启动脚本 | `.resc`（Renode Script，Renode 脚本） | 创建机器、加载固件、把 UART 打到文件、启动/暂停 |
| 固件 | `.bin` / `.elf` | 真正在虚拟 CPU 上执行的指令 |

对应关系可以记成：

```text
.repl  = 原理图上的那颗 MCU（内存映射 + 外设模型）
.resc  = 你插上仿真器后按的那些按钮
.bin   = 下载到 Flash 里的程序
```

没有 `.repl`，CPU 不知道外设在哪。没有固件，机器是空壳。没有 `.resc`，你每次都要手工敲 Monitor 命令。

本目录最小工程对应：

```text
1-8Simulate/
├── 从零搭建Renode仿真项目.md          ← 本文
├── 嵌入式代码快速仿真工具调研.md      ← 工具怎么选
└── demo-stm32f401rc/
    ├── stm32f401rc.repl               ← 板级内存尺寸（阅读用副本）
    ├── stm32f401rc.resc               ← 启动脚本
    ├── build_hello.py                 ← 生成 hello.bin
    ├── hello.bin                      ← 运行 run.cmd 时生成
    └── run.cmd                        ← 一键无界面跑通
```

Renode 真正加载的板级文件在安装目录：

```text
E:\ProgramFiles\Renode\renode_1.16.1+20260825giteedd64f2e-portable\platforms\boards\stm32f401rc.repl
```

它会 `using` 官方的 `platforms/cpus/stm32f4.repl`，只改 Flash/SRAM 大小，使其符合 F401RCT6（256KB Flash、64KB SRAM）。

---

## 1. 安装仿真器（本机已完成，重装时按这节）

### 1.1 为什么用 portable 而不是安装包

Windows 上官方提供：

- `*.setup.exe`：安装程序，目录不一定好控；
- `*.windows-portable.zip`：解压即用，适合指定盘符。

指定目录时选 portable，解压到 `E:\ProgramFiles\Renode`。当前实际路径：

```text
E:\ProgramFiles\Renode\renode_1.16.1+20260825giteedd64f2e-portable\renode.exe
```

仓库根下的 `renode.cmd` 会转到上述 `renode.exe`。用户 PATH 已加入 `E:\ProgramFiles\Renode`，**新开** PowerShell 后可直接打 `renode`。

下载地址（版本会变，以 [Renode Releases](https://github.com/renode/renode/releases) 或 [builds.renode.io](https://builds.renode.io/) 为准）：

```text
https://builds.renode.io/renode-latest.windows-portable.zip
```

解压示例：

```powershell
New-Item -ItemType Directory -Force -Path E:\ProgramFiles\Renode | Out-Null
curl.exe -L -o E:\ProgramFiles\renode-win-portable.zip https://builds.renode.io/renode-latest.windows-portable.zip
Expand-Archive E:\ProgramFiles\renode-win-portable.zip -DestinationPath E:\ProgramFiles\Renode -Force
```

解压后会多一层带版本号的目录，以里面的 `renode.exe` 为准。

### 1.2 本机还用到的辅助工具

| 工具 | 用途 |
| --- | --- |
| Python 3 | 跑 `build_hello.py` |
| `keystone-engine` | 把 Thumb 汇编变成机器码（`pip install keystone-engine`） |

正式项目里应改用 `arm-none-eabi-gcc` 编 ELF。最小 demo 故意不用完整工具链，降低「从 0」的门槛。

### 1.3 第一次启动会干什么

`stm32f4.repl` 会下载 SVD（System View Description，系统视图描述）压缩包，用来给寄存器起名字。需要能访问：

```text
https://dl.antmicro.com/projects/renode/svd/STM32F40x.svd.gz
```

失败时仿真仍可能起来，只是未实现寄存器的日志不好读。

---

## 2. 从 0 新建一个仿真工程：按顺序做

下面按「空文件夹 → 第一次看到串口字」拆步骤。配套文件已经放在 `demo-stm32f401rc\`，你可以对照着抄，而不是凭空发明。

### 步骤 A — 定芯片资源，不要抄错容量

查 STM32F401RCT6 手册（或选芯片手册的 Memory mapping）：

| 资源 | 地址 | 本芯片容量 |
| --- | --- | --- |
| Flash | `0x08000000` | 256KB = `0x40000` |
| SRAM | `0x20000000` | 64KB = `0x10000` |
| USART2 | `0x40004400` | 与 F4 系列相同 |
| Cortex-M4 | — | 与 `stm32f4.repl` 中 `cpuType: "cortex-m4"` 一致 |

官方 `stm32f4.repl` 默认按更大的 F407 来（Flash 2MB、SRAM 256KB）。接拿直来跑 F401 **多数时候 CPU 仍能跑**，但内存边界与真机不符。板级 `.repl` 必须覆盖：

```text
using "platforms/cpus/stm32f4.repl"

flash:
    size: 0x40000

sram:
    size: 0x10000
```

把该文件放到 Renode 安装树的 `platforms/boards/stm32f401rc.repl`。脚本里用：

```text
machine LoadPlatformDescription @platforms/boards/stm32f401rc.repl
```

`@platforms/...` 相对的是 **Renode 安装根目录**，不是当前工程目录。

### 步骤 B — 写启动脚本 `.resc`

最小脚本做五件事：

1. `using sysbus`：后续命令默认打在系统总线；
2. `mach create "名字"`：新建一台虚拟机；
3. `LoadPlatformDescription`：装上 .repl；
4. 加载固件并设置 PC / SP / VTOR；
5. 把 UART 接到文件，便于无界面验收。

本工程脚本要点：

```text
sysbus.usart2 CreateFileBackend @E:/Codes/CAndC++/1-8Simulate/demo-stm32f401rc/uart.log true
sysbus LoadBinary @.../hello.bin 0x08000000
cpu VectorTableOffset 0x08000000
cpu SP 0x20010000
cpu PC 0x08000009
```

说明：

- 路径用正斜杠。Windows 下 Renode 认 `E:/...`。
- Cortex-M 复位时从 **向量表** 取 MSP 和复位地址。`stm32f4.repl` 没有把 Flash 镜像到 `0x00000000`，所以要显式 `VectorTableOffset 0x08000000`，并手动设 `SP`/`PC`，避免复位读到空总线。
- `PC = 0x08000009`：代码从 `0x08000008` 开始，最低位 1 表示 Thumb 状态。这是 ARM 的硬约定，不是笔误。
- `CreateFileBackend` 的第二个参数为 true 时，每写一字节就刷新文件，适合短 demo。

无界面跑完就退出时，不要把 `start` 和 `quit` 写进脚本再开 Telnet Monitor：Monitor 占着端口，进程可能不退出。正确做法是命令行：

```text
renode.exe --disable-gui -P -1 --plain --hide-analyzers -e "i @脚本.resc; emulation RunFor ""00:00:00.05""; q"
```

| 参数 | 作用 |
| --- | --- |
| `--disable-gui` | 不要窗口（也可用 `--disable-xwt`） |
| `-P -1` | 关掉 Telnet Monitor，脚本才能自己退出 |
| `emulation RunFor "00:00:00.05"` | 虚拟时间跑 50ms，然后暂停 |
| `q` | 退出 Renode |

### 步骤 C — 准备最小固件

目标：CPU 取指、写 USART2 数据寄存器，文件里出现 `F401RCT6 OK`。

向量表（小端，Flash 开头 8 字节）：

| 偏移 | 值 | 含义 |
| --- | --- | --- |
| `+0` | `0x20010000` | 初始 MSP，64KB SRAM 顶端 |
| `+4` | `0x08000009` | 复位处理函数（Thumb） |

随后是 Thumb 代码：打开 USART2 的 UE（USART Enable）和 TE（Transmitter Enable），把字符串逐字节写入 `USART2_DR`（`0x40004404`），然后死循环。

生成：

```powershell
cd E:\Codes\CAndC++\1-8Simulate\demo-stm32f401rc
python build_hello.py
```

需要已安装 `keystone-engine`。这不是生产固件流程，只是为了不引入 `arm-none-eabi-gcc`。

换成自己的工程时，删除 `LoadBinary`，改为：

```text
sysbus LoadELF @E:/path/to/firmware.elf
```

ELF（Executable and Linkable Format，可执行与可链接格式）带符号，GDB 才能按函数名下断点。`hello.bin` 没有符号。

### 步骤 D — 跑一次，用输出当验收

```powershell
E:\Codes\CAndC++\1-8Simulate\demo-stm32f401rc\run.cmd
```

成功时终端末尾类似：

```text
---- uart.log ----
F401RCT6 OK
```

同时日志里应出现：

```text
Loading block of 56 bytes length at 0x8000000.
STM32F401RCT6: Machine started.
STM32F401RCT6: Machine paused.
```

这三句分别表示：固件进了 Flash、虚拟机跑起来了、到时暂停。缺 `F401RCT6 OK` 就还没证明 UART 通路。

---

## 3. 交互式用法（你「还不知道怎么用」时从这里下手）

`run.cmd` 是验收脚本。要自己敲命令，新开 PowerShell：

```powershell
renode --console
```

若 `renode` 找不到，用全路径：

```powershell
E:\ProgramFiles\Renode\renode_1.16.1+20260825giteedd64f2e-portable\renode.exe --console
```

Monitor 里：

```text
i @E:/Codes/CAndC++/1-8Simulate/demo-stm32f401rc/stm32f401rc.resc
start
```

常用命令：

| 命令 | 作用 |
| --- | --- |
| `pause` | 停住虚拟时间 |
| `start` | 继续 |
| `cpu PC` | 看程序计数器 |
| `cpu SP` | 看主栈指针 |
| `sysbus usart2` | 看 UART 外设是否挂上 |
| `peripherals` | 列出已创建外设 |
| `q` | 退出 |

`--console` 会开 Monitor。不要和 `-P -1` 一起用在「只想跑完就退出」的脚本上。

---

## 4. 接 GDB（GNU Debugger，GNU 调试器）

仿真器扮演 J-Link 的角色：提供 GDB Server，GDB 连上来看寄存器和栈。

在 Monitor（此时不要用 `-P -1`）：

```text
machine StartGdbServer 3333
start
```

另开终端：

```text
arm-none-eabi-gdb firmware.elf
(gdb) target remote :3333
(gdb) break main
(gdb) continue
```

当前 `hello.bin` 没有符号，`break main` 无意义。对真实固件用 ELF。无硬件时链路是：

```text
GDB  →  Renode GDB Server  →  虚拟 Cortex-M4
```

有硬件时是：

```text
GDB  →  J-Link GDB Server  →  真芯片
```

上层操作（断点、单步、看变量）相同，底层一个是模型、一个是硅片。

---

## 5. 从最小 demo 长成「自己的固件工程」

建议仍保持三个文件，只替换固件来源：

```text
my-fw.repl     继续用 F401 的 Flash/SRAM 覆盖
my-fw.resc     LoadELF 指向 PlatformIO/Keil 的 .elf
firmware.elf   工程编译产物
```

`.resc` 里典型替换：

```text
$bin?=@E:/GiteeSpace/firmware-copy/firmware-dev/.pio/build/xxx/firmware.elf
sysbus LoadELF $bin
```

然后按层加码，不要一上来仿真整机：

1. **能启动、能打 UART**：证明时钟/控制台假设和模型匹配；
2. **能跑状态机、G-code 解析**：这些不依赖未建模的 ADC 芯片；
3. **SPI 外设**：Renode 有 STM32 SPI 模型，但 **没有 CS5552 模型**，要自己写外设模型或在固件侧注入假数据；
4. **DMA / 定时器耦合的压力环**：模型覆盖不完整，结论不能当真机时序。

分层策略与调研文档一致：Native Mock 测算法，Renode 测「这颗 F4 核 + 已建模外设」，真机测电气和机械。

---

## 6. 常见翻车

| 现象 | 原因 | 处理 |
| --- | --- | --- |
| `renode` 不是命令 | PATH 未刷新 | 新开终端，或用 `renode.exe` 全路径 |
| 进程一直不退出 | 开了 Telnet Monitor（默认端口 1234） | 无界面验收加 `-P -1`，脚本末尾 `q` |
| 加载了 bin 但 UART 文件是空的 | PC/VTOR 不对，或写错 UART 基址 | 确认 `0x08000009`、USART2=`0x40004400` |
| `CreateFileBackend` 路径无效 | 用了反斜杠或 `$ORIGIN` 在该版本无效 | 使用 `E:/...` 正斜杠绝对路径 |
| 想仿真 CS5552 压力链路 | 没有该芯片模型 | 先 Mock 寄存器，或写 C# 外设模型；不要指望 demo 覆盖 |
| Wokwi 跑这套固件 | Wokwi 当前不覆盖 STM32F401RC | 用 Renode，不要用 Wokwi 当主仿真 |

---

## 7. 本机验收记录（2026-08-26）

在 `E:\ProgramFiles\Renode\demo-stm32f401rc` 上首次跑通：USART2 文件内容为 `F401RCT6 OK`。  

同内容已迁到本目录 `demo-stm32f401rc\`，用 `run.cmd` 复现。

这只证明：**Cortex-M4 取指 + USART2 模型可出字**。不证明 SPI、DMA、CS5552、电机与机械。

---

## 8. 你下次打开电脑时的最短路径

```powershell
E:\Codes\CAndC++\1-8Simulate\demo-stm32f401rc\run.cmd
```

看到 `F401RCT6 OK` 就说明工具链仍在。要改自己的板，只动 `.repl` 的内存大小和 `.resc` 的 ELF 路径。

---

**小测试：** 为什么最小 demo 里要把 `cpu PC` 设成 `0x08000009`，而不是 `0x08000008`？最低那一位代表什么？
