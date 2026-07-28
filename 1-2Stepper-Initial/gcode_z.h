#ifndef GCODE_Z_H
#define GCODE_Z_H

#include <stdbool.h>

/* Z 轴运动参数 */
typedef struct {
    float position_mm;    /* 当前 Z 位置 (mm) */
    float feedrate_mm_min; /* 进给速度 (mm/min) */
    bool  is_moving;      /* 是否运动中 */
    bool  is_homed;       /* 是否已回零 */
} GcodeZAxis_t;

void GcodeZ_Init(GcodeZAxis_t *axis);
bool GcodeZ_Execute(const char *gcode_line, GcodeZAxis_t *axis);

/* 常用 Z 轴指令封装 */
bool GcodeZ_Home(GcodeZAxis_t *axis);
bool GcodeZ_MoveAbs(float z_mm, float feed_mm_min, GcodeZAxis_t *axis);
bool GcodeZ_MoveRel(float dz_mm, float feed_mm_min, GcodeZAxis_t *axis);

/* 运动完成轮询（真实固件中对接步进中断/Planner idle） */
bool GcodeZ_IsIdle(const GcodeZAxis_t *axis);
void GcodeZ_TickSimulation(GcodeZAxis_t *axis, float dt_s);

#endif
