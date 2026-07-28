#include <stdio.h>
#include <locale.h>
#include "lcd_motion_fsm.h"

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * LCD 光固化打印机 Z 轴运动状态机 Demo
 *
 * 编译:
 *   gcc main.c pressure_sensor.c gcode_z.c lcd_motion_fsm.c -o lcd_fsm -lm
 * 运行:
 *   .\lcd_fsm
 *
 * 源码为 UTF-8；Windows 终端默认 GBK，程序启动时会自动切换为 UTF-8。
 * 若仍乱码，可先执行: chcp 65001
 *
 * 本程序用 PressureSensor_SimulateInject() 模拟压力变化，
 * 真实固件中删除仿真注入，改为硬件读取即可。
 */

/* Windows 控制台切换为 UTF-8，避免中文乱码 */
// static void console_init_utf8(void)
// {
// #ifdef _WIN32
//     SetConsoleOutputCP(65001);
//     SetConsoleCP(65001);
// #endif
//     setlocale(LC_ALL, ".UTF-8");
// }

/* 根据状态机阶段注入合理的压力仿真曲线 */
static void SimPressureForState(LcdMotionFsm_t *fsm)
{
    float t = fsm->state_timer_s;
    float base = 10.0f;

    switch (fsm->state) {
    case LCD_STATE_INIT:
    case LCD_STATE_HOMING:
        PressureSensor_SimulateInject(10.0f + 0.1f * (t < 0.3f));
        break;

    case LCD_STATE_MOVE_TO_PRINT_START:
        /* 接触树脂时压力爬升 */
        PressureSensor_SimulateInject(10.0f + 30.0f * (t < 0.5f ? t / 0.5f : 1.0f));
        break;

    case LCD_STATE_WAIT_CONTACT_STABLE:
        PressureSensor_SimulateInject(40.0f + 0.05f * (t < 0.2f));
        break;

    case LCD_STATE_EXPOSURE:
        PressureSensor_SimulateInject(40.0f);
        break;

    case LCD_STATE_PEEL_LIFT:
        PressureSensor_SimulateInject(40.0f - 20.0f * (t < 0.4f ? t / 0.4f : 1.0f));
        break;

    case LCD_STATE_WAIT_PEEL_STABLE:
        PressureSensor_SimulateInject(18.0f + 0.03f * (t < 0.15f));
        break;

    case LCD_STATE_RETRACT:
    case LCD_STATE_WAIT_RETRACT_STABLE:
        PressureSensor_SimulateInject(16.0f);
        break;

    case LCD_STATE_RETURN_DOWN:
        PressureSensor_SimulateInject(16.0f + 22.0f * (t < 0.5f ? t / 0.5f : 1.0f));
        break;

    case LCD_STATE_WAIT_RETURN_STABLE:
        PressureSensor_SimulateInject(base + fsm->current_layer * 2.0f + 0.04f * (t < 0.2f));
        break;

    case LCD_STATE_FINISH:
        PressureSensor_SimulateInject(10.0f);
        break;

    default:
        PressureSensor_SimulateInject(base);
        break;
    }

    (void)base;
}

int main(void)
{
    LcdMotionFsm_t fsm;
    LcdPrintProfile_t profile = {
        .layer_thickness_mm = 0.05f,
        .peel_lift_mm = 6.0f,
        .peel_speed_mm_min = 60.0f,
        .return_speed_mm_min = 120.0f,
        .contact_speed_mm_min = 60.0f,
        .exposure_time_s = 0.3f,  /* 仿真缩短曝光时间 */
        .total_layers = 3
    };

    const float dt = 0.02f;
    int tick = 0;
    LcdMotionState_t last_state = LCD_STATE_INIT;

    /* 必须在任何 printf 之前调用 */
    // console_init_utf8();

    LcdMotionFsm_Init(&fsm, &profile);
    LcdMotionFsm_Start(&fsm);

    printf("=== LCD Z轴运动状态机仿真 ===\n");

    while (!LcdMotionFsm_IsDone(&fsm) && tick < 50000) {
        SimPressureForState(&fsm);
        LcdMotionFsm_Tick(&fsm, dt);

        if (fsm.state != last_state) {
            printf(">> 状态切换: %s | Z=%.3f mm | layer=%d\n",
                   LcdMotionFsm_StateName(fsm.state),
                   fsm.z_axis.position_mm,
                   fsm.current_layer);
            last_state = fsm.state;
        }

        tick++;
    }

    printf("=== 仿真结束: %s ===\n", LcdMotionFsm_StateName(fsm.state));
    return (fsm.state == LCD_STATE_IDLE && fsm.current_layer >= profile.total_layers) ? 0 : 1;
}
