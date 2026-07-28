#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <stdbool.h>

/* 压力传感器稳定判定参数 */
typedef struct {
    float stable_threshold;   /* 连续采样最大波动阈值 (Pa 或归一化单位) */
    float sample_interval_ms; /* 采样间隔 (ms) */
    int   stable_count_req;   /* 判定稳定所需连续达标次数 */
} PressureStableCfg_t;

/* 传感器读数快照 */
typedef struct {
    float value;        /* 当前压力值 */
    float baseline;     /* 基线（空载/静置参考） */
    float delta;        /* 相对基线的变化量 */
    bool  is_stable;    /* 是否已稳定 */
    int   stable_count; /* 当前连续稳定计数 */
} PressureReading_t;

/* 初始化传感器接口（桩函数，无真实硬件） */
void PressureSensor_Init(void);

/* 读取一次压力并更新稳定状态机 */
PressureReading_t PressureSensor_ReadAndUpdate(const PressureStableCfg_t *cfg);

/* 重置稳定计数（状态切换时调用） */
void PressureSensor_ResetStability(void);

/* 模拟注入压力值（仅用于桌面仿真/demo） */
void PressureSensor_SimulateInject(float value);

#endif
