#include <cstdio>


class PIDController {
    public:
        PIDController(float kp, float ki, float kd, float dt):
        //构造函数，和类同名，没有返回值，用于初始化对象
        kp_(kp),
        ki_(ki),
        kd_(kd),
        dt_(dt),
        integral_(0.0),
        prev_error_(0.0)
        {
            
        }

        float compute(float setpoint, float measured_value) {
            float error = setpoint - measured_value;
            integral_ += error * dt_;
            float derivative = (error - prev_error_) / dt_;
            prev_error_ = error;
            return kp_ * error + ki_ * integral_ + kd_ * derivative;
        }

    ~PIDController() {
        
    } //析构函数，类名字前面加~，没有返回值，用于释放对象


    private:
        float kp_, ki_, kd_, dt_;
        float integral_, prev_error_;
};


int main() {

    PIDController pid(1.0, 0.1, 0.01, 0.01);
    float setpoint = 100.0;
    float measured_value = 0.0;
    float output = pid.compute(setpoint, measured_value);
    std::printf("output: %f\n", output);
    return 0;
}
