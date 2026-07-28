#ifndef MYPID_H
#define MYPID_H


#define SetPoint 100

typedef struct {
    float kp, ki, kd;
    float integral, prev_error;
    float dt;
}PID_t;


void PID_init(PID_t *pid, float kp, float ki, float kd, float dt);
float PID_update(PID_t *pid, float setpoint, float measured_value);

#endif