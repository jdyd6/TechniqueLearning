#include "pressure_sensor.h"
#include <math.h>

/* 内部仿真状态：真实项目中替换为 ADC/I2C/SPI 读取 */
static float s_sim_pressure = 0.0f;
static float s_baseline = 0.0f;
static float s_last_value = 0.0f;
static int   s_stable_count = 0;

void PressureSensor_Init(void)
{
    s_sim_pressure = 10.0f;
    s_baseline = 10.0f;
    s_last_value = 0.0f;
    s_stable_count = 0;
}

void PressureSensor_ResetStability(void)
{
    s_stable_count = 0;
    s_last_value = 0.0f;
}

void PressureSensor_SimulateInject(float value)
{
    s_sim_pressure = value;
}

PressureReading_t PressureSensor_ReadAndUpdate(const PressureStableCfg_t *cfg)
{
    PressureReading_t reading;
    float diff;

    /* 桩接口：向基线自然衰减，模拟树脂/FEP 系统弛豫 */
    s_sim_pressure = s_baseline + (s_sim_pressure - s_baseline) * 0.85f;

    reading.value = s_sim_pressure;
    reading.baseline = s_baseline;
    reading.delta = reading.value - reading.baseline;

    if (s_last_value == 0.0f && s_stable_count == 0) {
        s_last_value = reading.value;
        reading.is_stable = false;
        reading.stable_count = 0;
        return reading;
    }

    diff = fabsf(reading.value - s_last_value);
    s_last_value = reading.value;

    if (diff <= cfg->stable_threshold) {
        s_stable_count++;
    } else {
        s_stable_count = 0;
    }

    reading.stable_count = s_stable_count;
    reading.is_stable = (s_stable_count >= cfg->stable_count_req);
    return reading;
}
