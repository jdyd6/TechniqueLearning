#ifndef LCD_MOTION_FSM_H
#define LCD_MOTION_FSM_H

#include "gcode_z.h"
#include "pressure_sensor.h"
#include <stdbool.h>

typedef enum {
    LCD_STATE_INIT = 0,
    LCD_STATE_HOMING,
    LCD_STATE_MOVE_TO_PRINT_START,
    LCD_STATE_WAIT_CONTACT_STABLE,
    LCD_STATE_EXPOSURE,
    LCD_STATE_PEEL_LIFT,
    LCD_STATE_WAIT_PEEL_STABLE,
    LCD_STATE_RETRACT,
    LCD_STATE_WAIT_RETRACT_STABLE,
    LCD_STATE_RETURN_DOWN,
    LCD_STATE_WAIT_RETURN_STABLE,
    LCD_STATE_LAYER_ADVANCE,
    LCD_STATE_WAIT_LAYER_STABLE,
    LCD_STATE_CHECK_COMPLETE,
    LCD_STATE_FINISH,
    LCD_STATE_ERROR,
    LCD_STATE_IDLE
} LcdMotionState_t;

/* 单层工艺参数 */
typedef struct {
    float layer_thickness_mm;  /* 层高 (mm) */
    float peel_lift_mm;        /* 剥离抬升量 (mm) */
    float peel_speed_mm_min;   /* 剥离速度 */
    float return_speed_mm_min; /* 回位速度 */
    float contact_speed_mm_min;/* 接触速度 */
    float exposure_time_s;     /* 曝光时间 (s) */
    int   total_layers;        /* 总层数 */
} LcdPrintProfile_t;

/* 状态机运行时上下文 */
typedef struct {
    LcdMotionState_t state;
    GcodeZAxis_t     z_axis;
    PressureStableCfg_t pressure_cfg;
    LcdPrintProfile_t profile;

    int   current_layer;
    float print_start_z_mm;
    float layer_base_z_mm;

    float state_timer_s;
    float pressure_wait_s;
    PressureReading_t last_pressure;

    bool  request_abort;
    bool  is_running;
    bool  motion_cmd_issued;  /* 当前状态是否已下发 G-code */
} LcdMotionFsm_t;

void LcdMotionFsm_Init(LcdMotionFsm_t *fsm, const LcdPrintProfile_t *profile);
void LcdMotionFsm_Start(LcdMotionFsm_t *fsm);
void LcdMotionFsm_Abort(LcdMotionFsm_t *fsm);
void LcdMotionFsm_Tick(LcdMotionFsm_t *fsm, float dt_s);

const char *LcdMotionFsm_StateName(LcdMotionState_t state);
bool LcdMotionFsm_IsDone(const LcdMotionFsm_t *fsm);

#endif
