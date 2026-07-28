/**
 * 配置开关“到处用”的体感演示（约百行）
 *
 * 玩法：只改下面「配置区」的 0/1，重新编译运行，看输出怎么变。
 *   gcc tern_demo.c -o tern_demo && ./tern_demo
 */
#include <stdio.h>
#include "macro.h"

/* ======================== 配置区（只改这里） ======================== */
#define HAS_SENSOR   1   /* 1=有传感器  0=无 */
#define HAS_FAN      0   /* 1=有风扇    0=无 */
#define HAS_HEATER   1   /* 1=有加热    0=无 */
#define DEBUG_LOG    1   /* 1=打详细日志 */
#define MACHINE_ID   1   /* 假想机型编号，给 TERN 当开关演示 */
/* ================================================================== */

/* 引脚：开关直接写进“常量表”，关了就变成 -1 */
enum {
    PIN_SENSOR = TERN(HAS_SENSOR, 42, -1),
    PIN_FAN    = TERN(HAS_FAN, 17, -1),
    PIN_HEAT   = TERN(HAS_HEATER, 9, -1),
};

/* 设备名字符串也靠配置拼 */
#define MACHINE_NAME TERN(MACHINE_ID, "Alpha-X1", "Beta-Lite")

typedef struct {
    const char *name;
    int sensor_pin;
    int fan_pin;
    int heat_pin;
    int base_speed;   /* TERN0：没风扇就不再加转速 */
} Printer;

static void log_dbg(const char *msg)
{
    /* 表达式位置用开关：关 DEBUG 时整句变成空操作依赖 */
    if (TERN(DEBUG_LOG, 1, 0))
        printf("[DEBUG] %s\n", msg);
}

static int calc_speed(int user_speed)
{
    /* 基础速 + 有风扇再加 10；加热器开着再抠 5（示意） */
    int speed = user_speed
              + TERN0(HAS_FAN, 10)
              - TERN0(HAS_HEATER, 5);
    return speed < 0 ? 0 : speed;
}

static void report_pins(const Printer *p)
{
    printf("---- 引脚表 ----\n");
    printf("  传感器 : %s (pin=%d)\n",
           TERN(HAS_SENSOR, "已连接", "未安装"), p->sensor_pin);
    printf("  风扇   : %s (pin=%d)\n",
           TERN(HAS_FAN, "已连接", "未安装"), p->fan_pin);
    printf("  加热器 : %s (pin=%d)\n",
           TERN(HAS_HEATER, "已连接", "未安装"), p->heat_pin);
}

static void boot_banner(const Printer *p)
{
    printf("========================================\n");
    printf(" 机器: %s\n", p->name);
    printf(" 配置指纹: SENSOR=%d FAN=%d HEAT=%d DEBUG=%d\n",
           HAS_SENSOR, HAS_FAN, HAS_HEATER, DEBUG_LOG);
    printf("========================================\n");
    log_dbg("boot_banner 完成");
}

static void read_sensor_or_stub(void)
{
    /* 函数体里也能用：开了就“读传感器”，关了走桩 */
    if (TERN(HAS_SENSOR, 1, 0)) {
        printf("[SENSOR] 读到温度约 %d C（模拟）\n", 25 + PIN_SENSOR % 7);
        log_dbg("真实传感器路径");
    } else {
        printf("[SENSOR] 无硬件，返回默认 25 C\n");
        log_dbg("桩路径");
    }
}

static void fan_control(int on)
{
    if (PIN_FAN < 0) {
        printf("[FAN] 本机无风扇，忽略开关请求\n");
        return;
    }
    printf("[FAN] pin %d -> %s\n", PIN_FAN, on ? "ON" : "OFF");
}

int main(void)
{
    Printer p = {
        .name       = MACHINE_NAME,
        .sensor_pin = PIN_SENSOR,
        .fan_pin    = PIN_FAN,
        .heat_pin   = PIN_HEAT,
        .base_speed = calc_speed(100),
    };

    boot_banner(&p);
    report_pins(&p);

    printf("\n---- 运行时行为 ----\n");
    read_sensor_or_stub();
    fan_control(1);

    printf("\n---- 速度计算 ----\n");
    printf("  user=100 → 最终速度 %d\n", p.base_speed);
    printf("  （公式: 100 + TERN0(FAN,10) - TERN0(HEAT,5)）\n");

    /* 数组初始化里夹配置：关了的轴填 -1 */
    int axes[] = {
        0, /* X 始终有 */
        TERN(HAS_FAN, 1, -1),      /* 借用风扇开关示意可选轴 */
        TERN(HAS_HEATER, 2, -1),
    };
    printf("\n---- 轴映射 (示意) ----\n");
    for (int i = 0; i < 3; i++)
        printf("  axes[%d] = %d %s\n", i, axes[i],
               axes[i] < 0 ? "(禁用)" : "(启用)");

    printf("\n提示: 改文件顶部 HAS_* / DEBUG_LOG 的 0/1，重新编译再跑。\n");
    return 0;
}
