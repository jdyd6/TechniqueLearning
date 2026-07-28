# LCD 光固化打印机 Z 轴运动流程参考

> 本文档供 `lcd_motion_fsm.c` 状态机设计参考，梳理真实 LCD（Masked Stereolithography / MSLA）打印机的典型 Z 轴运动阶段。

## 1. LCD 打印机运动本质

与 FDM（熔融沉积）不同，LCD 光固化打印机的 Z 轴运动围绕 **「曝光 → 剥离 → 回位」** 循环展开：

| 阶段 | Z 轴动作 | 目的 |
|------|----------|------|
| 回零 (Homing) | G28 Z | 建立机械零点 |
| 下降至液面 | G1 Z（慢速） | 构建平台接触树脂液面 |
| 曝光 (Exposure) | 静止 | UV/LCD 固化当前层 |
| 剥离 (Peel/Lift) | G1 Z+（抬升） | 将成型件从 FEP 膜上撕离 |
| 回退 (Retract) | G1 Z-（小幅） | 可选，减小冲击 |
| 回位 (Return) | G1 Z 至新层高 | 准备下一层曝光 |

## 2. 典型 G-code 指令（仅 Z 轴）

```gcode
G28 Z          ; Z 轴回零
G90            ; 绝对坐标模式
G1 Z5.000 F60  ; 以 60 mm/min 移动到 Z=5
G91            ; 相对坐标模式
G1 Z6.000 F60  ; 相对抬升 6 mm（剥离）
G1 Z-0.500 F120; 相对回退 0.5 mm
G90
G1 Z5.050 F120 ; 绝对移动到下一层高度
```

### 参数经验范围

- **剥离高度**：4–10 mm（视模型截面积和树脂粘度而定）
- **剥离速度**：30–120 mm/min（过快易拉坏薄壁）
- **回位速度**：60–300 mm/min
- **层高**：0.01–0.10 mm

## 3. 与状态机的映射

本项目的 `LcdMotionState_t` 与上述流程一一对应：

```
INIT → HOMING → MOVE_TO_PRINT_START → WAIT_CONTACT_STABLE
  → EXPOSURE → PEEL_LIFT → WAIT_PEEL_STABLE → RETRACT
  → WAIT_RETRACT_STABLE → RETURN_DOWN → WAIT_RETURN_STABLE
  → CHECK_COMPLETE → (下一层 EXPOSURE 或 FINISH)
```

每个 `WAIT_*_STABLE` 状态均要求压力传感器读数稳定后才允许迁移——这是本项目的核心约束。

## 4. 外部延伸阅读

- [RepRap G-code 维基](https://reprap.org/wiki/G-code) — G0/G1/G28 标准语义
- Chitubox / Lychee 切片软件导出的 `.gcode` 可观察真实剥离参数
- 工业 MSLA 设备维护手册中常有「Peel force / suction」相关描述

## 5. 项目内相关文件

| 文件 | 说明 |
|------|------|
| `lcd_motion_fsm.c` | 完整状态机实现 |
| `gcode_z.c` | G-code 解析与 Z 轴运动桩 |
| `pressure_sensor.c` | 压力稳定判定逻辑 |

---

*文档版本：2026-06，对应 1-2Stepper-Initial 初版*
