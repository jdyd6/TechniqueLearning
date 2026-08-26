#include "pressure_monitor.h"

#include <stdint.h>

#include "sensor_hal.h"

/* 达到该压力阈值时，监测结果进入阈值触发状态。 */
#define PRESSURE_THRESHOLD_KPA (100U)

pressure_monitor_status_t pressure_monitor_check(void)
{
    uint16_t pressure_kpa = 0U;

    if (!sensor_hal_read_pressure(&pressure_kpa))
    {
        return PRESSURE_MONITOR_SENSOR_ERROR;
    }

    if (pressure_kpa >= PRESSURE_THRESHOLD_KPA)
    {
        return PRESSURE_MONITOR_THRESHOLD_REACHED;
    }

    return PRESSURE_MONITOR_NORMAL;
}
