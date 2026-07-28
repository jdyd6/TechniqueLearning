#include "myPID.h"



void PID_init(PID_t *pid, float kp, float ki, float kd, float dt) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->dt = dt;
    pid->integral = 0;
    pid->prev_error = 0;
}

float PID_update(PID_t *pid, float setpoint, float measured_value) {
    float error = setpoint - measured_value;
    pid->integral += error * pid->dt;
    float derivative = (error - pid->prev_error) / pid->dt;
    pid->prev_error = error;
    return pid->kp * error + pid-> ki * pid->integral + pid->kd * derivative;
}



