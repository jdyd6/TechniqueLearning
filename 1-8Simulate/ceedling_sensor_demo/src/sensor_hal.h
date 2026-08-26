#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 读取压力传感器当前值。
 *
 * @param pressure_kpa 用于接收压力值的有效指针。
 * @return 读取成功时返回 true，硬件通信失败时返回 false。
 */
bool sensor_hal_read_pressure(uint16_t *pressure_kpa);

#endif
