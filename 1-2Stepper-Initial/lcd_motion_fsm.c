#include "lcd_motion_fsm.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* 状态名称 —— 调试输出用 */
static const char *STATE_NAMES[] = {
    "INIT", "HOMING", "MOVE_TO_PRINT_START", "WAIT_CONTACT_STABLE",
    "EXPOSURE", "PEEL_LIFT", "WAIT_PEEL_STABLE", "RETRACT",
    "WAIT_RETRACT_STABLE", "RETURN_DOWN", "WAIT_RETURN_STABLE",
    "LAYER_ADVANCE", "WAIT_LAYER_STABLE", "CHECK_COMPLETE",
    "FINISH", "ERROR", "IDLE"
};

const char *LcdMotionFsm_StateName(LcdMotionState_t state)
{
    if (state >= 0 && state <= LCD_STATE_IDLE) {
        return STATE_NAMES[state];
    }
    return "UNKNOWN";
}

static void fsm_transition(LcdMotionFsm_t *fsm, LcdMotionState_t next)
{
    if (fsm->state != next) {
        fsm->state = next;
        fsm->state_timer_s = 0.0f;
        fsm->pressure_wait_s = 0.0f;
        fsm->motion_cmd_issued = false;
        PressureSensor_ResetStability();
        printf("[FSM] --> %s\n", LcdMotionFsm_StateName(next));
    }
}

void LcdMotionFsm_Init(LcdMotionFsm_t *fsm, const LcdPrintProfile_t *profile)
{
    memset(fsm, 0, sizeof(*fsm));
    fsm->state = LCD_STATE_IDLE;
    fsm->profile = *profile;
    fsm->pressure_cfg.stable_threshold = 0.02f;
    fsm->pressure_cfg.sample_interval_ms = 10.0f;
    fsm->pressure_cfg.stable_count_req = 10;
    fsm->current_layer = 0;
    fsm->print_start_z_mm = profile->layer_thickness_mm;
    fsm->layer_base_z_mm = 0.0f;
    GcodeZ_Init(&fsm->z_axis);
    PressureSensor_Init();
}

void LcdMotionFsm_Start(LcdMotionFsm_t *fsm)
{
    if (fsm->state != LCD_STATE_IDLE && fsm->state != LCD_STATE_FINISH) {
        return;
    }
    fsm->current_layer = 0;
    fsm->layer_base_z_mm = 0.0f;
    fsm->is_running = true;
    fsm->request_abort = false;
    fsm_transition(fsm, LCD_STATE_INIT);
}

void LcdMotionFsm_Abort(LcdMotionFsm_t *fsm)
{
    fsm->request_abort = true;
}

bool LcdMotionFsm_IsDone(const LcdMotionFsm_t *fsm)
{
    /* ERROR 立即结束；正常完成须 FINISH 收工后回到 IDLE 且 is_running 已清 */
    if (fsm->state == LCD_STATE_ERROR) {
        return true;
    }
    return (!fsm->is_running && fsm->state == LCD_STATE_IDLE);
}

/* 模拟运动引起的压力扰动：Z 移动时注入瞬态，随后自然衰减回基线 */
static void simulate_motion_pressure(LcdMotionFsm_t *fsm, float magnitude)
{
    (void)fsm;
    PressureSensor_SimulateInject(magnitude);
}

static bool wait_pressure_stable(LcdMotionFsm_t *fsm, float dt_s)
{
    (void)dt_s;
    fsm->last_pressure = PressureSensor_ReadAndUpdate(&fsm->pressure_cfg);
    return fsm->last_pressure.is_stable;
}

void LcdMotionFsm_Tick(LcdMotionFsm_t *fsm, float dt_s)
{
    if (!fsm->is_running && fsm->state == LCD_STATE_IDLE) {
        return;
    }

    fsm->state_timer_s += dt_s;
    GcodeZ_TickSimulation(&fsm->z_axis, dt_s);

    if (fsm->request_abort) {
        fsm_transition(fsm, LCD_STATE_ERROR);
    }

    switch (fsm->state) {
    case LCD_STATE_INIT:
        /* 上电初始化：设置绝对坐标模式，准备归零 */
        printf("[FSM] G90 ; 绝对坐标模式\n");
        fsm_transition(fsm, LCD_STATE_HOMING);
        break;

    case LCD_STATE_HOMING:
        if (!fsm->z_axis.is_moving) {
            if (!fsm->motion_cmd_issued) {
                GcodeZ_Home(&fsm->z_axis);
                simulate_motion_pressure(fsm, 0.8f);
                fsm->motion_cmd_issued = true;
            } 
            else if (fsm->z_axis.is_homed) {
                fsm->layer_base_z_mm = 0.0f;
                fsm_transition(fsm, LCD_STATE_MOVE_TO_PRINT_START);
            }
        }
        break;

    case LCD_STATE_MOVE_TO_PRINT_START:
        if (!fsm->z_axis.is_moving) {
            if (!fsm->motion_cmd_issued) {
                float target = fsm->print_start_z_mm;
                GcodeZ_MoveAbs(target, fsm->profile.contact_speed_mm_min, &fsm->z_axis);
                simulate_motion_pressure(fsm, 0.5f);
                fsm->motion_cmd_issued = true;
            } else {
                fsm_transition(fsm, LCD_STATE_WAIT_CONTACT_STABLE);
            }
        }
        break;

    case LCD_STATE_WAIT_CONTACT_STABLE:
        /* 等待 build plate 与 FEP 接触压力稳定后再曝光 */
        if (wait_pressure_stable(fsm, dt_s)) {
            printf("[Pressure] 接触稳定 @ %.3f (delta=%.4f)\n",
                   fsm->last_pressure.value, fsm->last_pressure.delta);
            fsm_transition(fsm, LCD_STATE_EXPOSURE);
        }
        break;

    case LCD_STATE_EXPOSURE:
        /* UV/LCD 曝光 —— Z 轴静止 */
        if (fsm->state_timer_s >= fsm->profile.exposure_time_s) {
            printf("[Exposure] 第 %d 层曝光完成 (%.1fs)\n",
                   fsm->current_layer + 1, fsm->profile.exposure_time_s);
            fsm_transition(fsm, LCD_STATE_PEEL_LIFT);
        }
        break;

    case LCD_STATE_PEEL_LIFT:
        if (!fsm->z_axis.is_moving) {
            if (!fsm->motion_cmd_issued) {
                GcodeZ_MoveRel(fsm->profile.peel_lift_mm,
                               fsm->profile.peel_speed_mm_min, &fsm->z_axis);
                simulate_motion_pressure(fsm, 1.5f);
                fsm->motion_cmd_issued = true;
            } else {
                fsm_transition(fsm, LCD_STATE_WAIT_PEEL_STABLE);
            }
        }
        break;

    case LCD_STATE_WAIT_PEEL_STABLE:
        if (wait_pressure_stable(fsm, dt_s)) {
            printf("[Pressure] Peel 后压力稳定\n");
            fsm_transition(fsm, LCD_STATE_RETRACT);
        }
        break;

    case LCD_STATE_RETRACT:
        if (!fsm->z_axis.is_moving) {
            if (!fsm->motion_cmd_issued) {
                float retract = -1.0f;
                GcodeZ_MoveRel(retract, fsm->profile.return_speed_mm_min, &fsm->z_axis);
                simulate_motion_pressure(fsm, 0.6f);
                fsm->motion_cmd_issued = true;
            } else {
                fsm_transition(fsm, LCD_STATE_WAIT_RETRACT_STABLE);
            }
        }
        break;

    case LCD_STATE_WAIT_RETRACT_STABLE:
        if (wait_pressure_stable(fsm, dt_s)) {
            fsm_transition(fsm, LCD_STATE_RETURN_DOWN);
        }
        break;

    case LCD_STATE_RETURN_DOWN:
        /* 回位：用绝对坐标落到下一层高度，避免相对位移累加误差 */
        if (!fsm->z_axis.is_moving) {
            if (!fsm->motion_cmd_issued) {
                float next_z = fsm->print_start_z_mm
                    + (float)(fsm->current_layer + 1) * fsm->profile.layer_thickness_mm;
                GcodeZ_MoveAbs(next_z, fsm->profile.return_speed_mm_min, &fsm->z_axis);
                simulate_motion_pressure(fsm, 0.4f);
                fsm->motion_cmd_issued = true;
            } else {
                fsm->layer_base_z_mm = fsm->z_axis.position_mm;
                fsm_transition(fsm, LCD_STATE_WAIT_RETURN_STABLE);
            }
        }
        break;

    case LCD_STATE_WAIT_RETURN_STABLE:
        if (wait_pressure_stable(fsm, dt_s)) {
            fsm_transition(fsm, LCD_STATE_LAYER_ADVANCE);
        }
        break;

    case LCD_STATE_LAYER_ADVANCE:
        fsm->current_layer++;
        printf("[Layer] 完成第 %d / %d 层\n",
               fsm->current_layer, fsm->profile.total_layers);
        fsm_transition(fsm, LCD_STATE_CHECK_COMPLETE);
        break;

    case LCD_STATE_WAIT_LAYER_STABLE:
        /* 后续层曝光前再次确认压力稳定 */
        if (wait_pressure_stable(fsm, dt_s)) {
            fsm_transition(fsm, LCD_STATE_EXPOSURE);
        }
        break;

    case LCD_STATE_CHECK_COMPLETE:
        if (fsm->current_layer >= fsm->profile.total_layers) {
            fsm_transition(fsm, LCD_STATE_FINISH);
        } else {
            fsm_transition(fsm, LCD_STATE_WAIT_LAYER_STABLE);
        }
        break;

    case LCD_STATE_FINISH:
        /* 打印结束：Z 轴抬升至零点停放 */
        if (!fsm->z_axis.is_moving) {
            if (!fsm->motion_cmd_issued) {
                GcodeZ_MoveAbs(0.0f, 300.0f, &fsm->z_axis);
                fsm->motion_cmd_issued = true;
            } else if (fsm->z_axis.is_homed || fabsf(fsm->z_axis.position_mm) < 1e-3f) {
                fsm->is_running = false;
                printf("[FSM] 打印完成。总层数: %d, 最终 Z=%.3f mm\n",
                       fsm->profile.total_layers, fsm->z_axis.position_mm);
                fsm_transition(fsm, LCD_STATE_IDLE);
            }
        }
        break;

    case LCD_STATE_ERROR:
        fsm->is_running = false;
        printf("[FSM] 异常中止\n");
        break;

    case LCD_STATE_IDLE:
    default:
        break;
    }
}
