#include "unity.h"

#include "mock_sensor_hal.h"
#include "pressure_monitor.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/* 配置 CMock 生成的 mock，使硬件读取接口返回指定压力值。 */
static void expect_sensor_pressure(uint16_t pressure_kpa)
{
    sensor_hal_read_pressure_ExpectAndReturn(NULL, true);
    sensor_hal_read_pressure_IgnoreArg_pressure_kpa();
    sensor_hal_read_pressure_ReturnThruPtr_pressure_kpa(&pressure_kpa);
}

void test_pressure_below_threshold_is_normal(void)
{
    expect_sensor_pressure(80U);

    TEST_ASSERT_EQUAL(PRESSURE_MONITOR_NORMAL, pressure_monitor_check());
}

void test_pressure_at_threshold_triggers_status(void)
{
    expect_sensor_pressure(100U);

    TEST_ASSERT_EQUAL(PRESSURE_MONITOR_THRESHOLD_REACHED, pressure_monitor_check());
}

void test_pressure_above_threshold_triggers_status(void)
{
    expect_sensor_pressure(120U);

    TEST_ASSERT_EQUAL(PRESSURE_MONITOR_THRESHOLD_REACHED, pressure_monitor_check());
}

void test_sensor_read_failure_reports_error(void)
{
    sensor_hal_read_pressure_ExpectAndReturn(NULL, false);
    sensor_hal_read_pressure_IgnoreArg_pressure_kpa();

    TEST_ASSERT_EQUAL(PRESSURE_MONITOR_SENSOR_ERROR, pressure_monitor_check());
}
