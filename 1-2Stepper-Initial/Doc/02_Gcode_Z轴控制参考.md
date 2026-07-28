# G-code Z 轴控制参考

> 面向步进电机 Z 轴单轴控制，说明本项目 `gcode_z.c` 的实现范围与扩展方向。

## 1. 支持的指令子集

本项目为研究用途，仅实现 Z 轴相关子集：

| 指令 | 功能 | 实现函数 |
|------|------|----------|
| `G28 Z` | Z 轴回零 | `GcodeZ_Home()` |
| `G0/G1 Z...` | 绝对/相对移动 | `GcodeZ_MoveAbs/Rel()` |
| `F...` | 进给速度 (mm/min) | 写入 `feedrate_mm_min` |

未实现：G2/G3 圆弧、多轴联动、温度、风扇 M 指令等——对 LCD Z 轴流程无影响。

## 2. 真实固件中的 G-code 执行链

```
切片软件 → .gcode 文件
    ↓
G-code 解析器 (Parser)
    ↓
运动规划器 (Planner / Motion Buffer)
    ↓
步进脉冲生成 (Stepper ISR / TMC 驱动)
    ↓
电机物理运动
```

本项目的 `GcodeZ_Execute()` 相当于简化版 Parser + Planner，用 `GcodeZ_TickSimulation()` 模拟运动完成。

### 对接 Marlin / Klipper 的思路

- **Marlin**：可直接向串口发送 `G1 Z...`，用 `planner.has_blocks_queued()` 判断运动是否结束
- **Klipper**：通过 `toolhead.get_position()` 或 G-code 脚本等待 `G4` 延时
- **自研固件**：在步进中断中递减剩余步数，`GcodeZ_IsIdle()` 返回 true

## 3. 坐标模式

| 模式 | G-code | 含义 |
|------|--------|------|
| 绝对 | G90 | `G1 Z5` = 移动到 Z=5 mm |
| 相对 | G91 | `G1 Z6` = 从当前位置上移 6 mm |

状态机中：
- 接触、回位用 **绝对坐标**（精确定位层高）
- 剥离、回退用 **相对坐标**（增量更直观）

## 4. 进给速度 F 的单位

标准 G-code 中 `F` 单位为 **mm/min**（毫米/分钟）。

换算：
```
速度 (mm/s) = F / 60
运动时间 (s) = 距离 (mm) / 速度 (mm/s)
```

## 5. 代码示例

```c
GcodeZAxis_t axis;
GcodeZ_Init(&axis);
GcodeZ_Home(&axis);

/* 等待回零完成 */
while (!GcodeZ_IsIdle(&axis)) {
    GcodeZ_TickSimulation(&axis, 0.01f);
}

GcodeZ_MoveRel(6.0f, 60.0f, &axis);  /* 剥离抬升 6 mm */
```

## 6. 推荐进一步阅读

- [LinuxCNC G-code 概述](https://linuxcnc.org/docs/html/gcode.html)
- Marlin Firmware `G0_G1.cpp` 源码 — 工业级实现范例
- Smoothieware / Grbl 的 planner 模块 — 理解运动缓冲

---

*文档版本：2026-06*
