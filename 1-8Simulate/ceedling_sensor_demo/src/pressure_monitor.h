#ifndef PRESSURE_MONITOR_H
#define PRESSURE_MONITOR_H

/**
 * @brief 压力监测结果。
 */
typedef enum
{
    PRESSURE_MONITOR_NORMAL = 0,
    PRESSURE_MONITOR_THRESHOLD_REACHED,
    PRESSURE_MONITOR_SENSOR_ERROR
} pressure_monitor_status_t;

/**
 * @brief 读取压力传感器并判断当前状态。
 *
 * @return 压力正常、达到阈值或传感器错误状态。
 */
pressure_monitor_status_t pressure_monitor_check(void);

#endif
