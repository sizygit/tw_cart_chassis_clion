//
// Created by nian on 2021/6/19.
//

#ifndef MOTOR_DRIVER_MOTORDRIVER_H
#define MOTOR_DRIVER_MOTORDRIVER_H

#include "main.h"
#include "pid.h"

#define ENABLE_M3508
#define ENABLE_M2006
#define ENABLE_GM6020
#define ENABLE_SNAIL2305

class Motor
{
protected:
    const static uint8_t MOTOR_NUMBER_MAX = 4;
    const uint8_t CAN_DLC = 8;

    typedef struct
    {
        float kp, ki, kd;
        float errorMax;
        float outputMax;
        float integralLimit;
    }PID_Para_Typedef;

public:
    struct
    {
        int16_t angle[MOTOR_NUMBER_MAX];
        int16_t speed[MOTOR_NUMBER_MAX];
        int16_t current[MOTOR_NUMBER_MAX];
        uint8_t temperature[MOTOR_NUMBER_MAX];
    }feedBackInfo;

    virtual void initMotor() = 0;
    virtual void sendControlCmd() = 0;
    virtual void calculatePID() = 0;

};


#endif //MOTOR_DRIVER_MOTORDRIVER_H
