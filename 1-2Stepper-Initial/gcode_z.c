#include "gcode_z.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* 仿真用：运动剩余时间 */
static float s_move_remain_s = 0.0f;
static float s_move_target_z = 0.0f;
static float s_move_start_z = 0.0f;
static float s_move_feed_mm_s = 0.0f;

void GcodeZ_Init(GcodeZAxis_t *axis)
{
    axis->position_mm = 0.0f;
    axis->feedrate_mm_min = 60.0f;
    axis->is_moving = false;
    axis->is_homed = false;
    s_move_remain_s = 0.0f;
}

bool GcodeZ_IsIdle(const GcodeZAxis_t *axis)
{
    (void)axis;
    return (s_move_remain_s <= 0.0f);
}

void GcodeZ_TickSimulation(GcodeZAxis_t *axis, float dt_s)
{
    if (!axis->is_moving || s_move_remain_s <= 0.0f) {
        axis->is_moving = false;
        /* 运动已结束：若目标为零点则标记已回零 */
        if (s_move_remain_s <= 0.0f && fabsf(s_move_target_z) < 1e-4f) {
            axis->position_mm = s_move_target_z;
            axis->is_homed = true;
        }
        return;
    }

    s_move_remain_s -= dt_s;
    if (s_move_remain_s <= 0.0f) {
        axis->position_mm = s_move_target_z;
        axis->is_moving = false;
        s_move_remain_s = 0.0f;
        /* 归零运动完成时置位 */
        if (fabsf(s_move_target_z) < 1e-4f) {
            axis->is_homed = true;
        }
    } else {
        float traveled = (s_move_start_z < s_move_target_z)
            ? (s_move_target_z - s_move_start_z) * (1.0f - s_move_remain_s /
               ((fabsf(s_move_target_z - s_move_start_z) / s_move_feed_mm_s) + 1e-6f))
            : (s_move_start_z - s_move_target_z) * (1.0f - s_move_remain_s /
               ((fabsf(s_move_target_z - s_move_start_z) / s_move_feed_mm_s) + 1e-6f));
        if (s_move_target_z >= s_move_start_z) {
            axis->position_mm = s_move_start_z + traveled;
        } else {
            axis->position_mm = s_move_start_z - traveled;
        }
    }
}

static void GcodeZ_StartMove(float target_z, float feed_mm_min, GcodeZAxis_t *axis)
{
    float distance = fabsf(target_z - axis->position_mm);
    s_move_start_z = axis->position_mm;
    s_move_target_z = target_z;
    s_move_feed_mm_s = feed_mm_min / 60.0f;

    /* 零距离：视为瞬时完成（已在目标位置） */
    if (distance < 1e-6f) {
        s_move_remain_s = 0.0f;
        axis->position_mm = target_z;
        axis->is_moving = false;
        if (fabsf(target_z) < 1e-4f) {
            axis->is_homed = true;
        }
        return;
    }

    s_move_remain_s = distance / s_move_feed_mm_s;
    axis->feedrate_mm_min = feed_mm_min;
    axis->is_moving = true;
}

bool GcodeZ_Home(GcodeZAxis_t *axis)
{
    printf("[Gcode] G28 Z ; Z轴回零\n");
    GcodeZ_StartMove(0.0f, 300.0f, axis);
    /* 已在零点且无需运动时，立即置位 is_homed */
    if (!axis->is_moving) {
        axis->is_homed = true;
    }
    return true;
}

bool GcodeZ_MoveAbs(float z_mm, float feed_mm_min, GcodeZAxis_t *axis)
{
    printf("[Gcode] G1 Z%.3f F%.0f ; 绝对移动\n", z_mm, feed_mm_min);
    GcodeZ_StartMove(z_mm, feed_mm_min, axis);
    return true;
}

bool GcodeZ_MoveRel(float dz_mm, float feed_mm_min, GcodeZAxis_t *axis)
{
    printf("[Gcode] G1 Z%.3f F%.0f ; 相对移动\n", dz_mm, feed_mm_min);
    GcodeZ_StartMove(axis->position_mm + dz_mm, feed_mm_min, axis);
    return true;
}

bool GcodeZ_Execute(const char *gcode_line, GcodeZAxis_t *axis)
{
    char buf[128];
    float z_val = 0.0f;
    float f_val = axis->feedrate_mm_min;

    strncpy(buf, gcode_line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    if (strstr(buf, "G28") != NULL) {
        return GcodeZ_Home(axis);
    }

    if (strstr(buf, "G0") != NULL || strstr(buf, "G1") != NULL) {
        char *pz = strstr(buf, "Z");
        char *pf = strstr(buf, "F");
        bool is_abs = (strstr(buf, "G90") != NULL);
        bool is_rel = (strstr(buf, "G91") != NULL);

        if (pz != NULL) {
            z_val = (float)atof(pz + 1);
        }
        if (pf != NULL) {
            f_val = (float)atof(pf + 1);
        }

        if (is_rel || (!is_abs && strstr(buf, "G91"))) {
            return GcodeZ_MoveRel(z_val, f_val, axis);
        }
        return GcodeZ_MoveAbs(z_val, f_val, axis);
    }

    return false;
}
