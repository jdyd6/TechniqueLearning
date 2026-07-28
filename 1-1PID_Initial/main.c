#include <stdio.h>
#include "myPID.h"

/*
gcc main.c myPID.c -o pid
.\pid
*/


int main(void) {
    
    
    PID_t pid;
    float setpoint = SetPoint;
    float measured = 0.0f;
    float output = 0.0f;
    float dt = 0.01f;
    float tau = 0.4f;

    PID_init(&pid, 1.5f, 6.0f, 0.0f, dt);

    printf("---PID Speed Simulation---\n");

    for (int i = 0; i < 2000; i++) {
        output = PID_update(&pid, setpoint, measured);
        measured += (output - measured) * dt / tau;

        if (i % 10 == 0) {
            printf("t = %.2f, target = %.1f, speed = %.2f, pwm = %.2f, err = %.2f\n", i * dt, setpoint, measured, output, measured - setpoint);
        }


    }

    return 0;

}