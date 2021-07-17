//
// Created by nian on 2021/6/8.
//

#ifndef DRIVER_M3508_M3508_M2006_H
#define DRIVER_M3508_M3508_M2006_H

#include "MotorDriver.h"

#ifdef ENABLE_M2006
#define M2006_MAX_SPEED_RPM 10000
class M2006 : public Motor
{
public:
    static const uint8_t motorNum = 1;
    uint32_t protocolID = 0x200;

private:
    CAN_HandleTypeDef *hcan;

    int expCurrent[motorNum] = {0};
    int expSpeed[motorNum] = {0};
    int expAngle[motorNum] = {0};

    PID_Para_Typedef speedRingPara = {
            1, 0, 0.2,
            6000,
            13000,
            500
    };
    PID PIDSpeedRing = PID(speedRingPara.kp, speedRingPara.ki, speedRingPara.kd, speedRingPara.errorMax, speedRingPara.outputMax, speedRingPara.integralLimit);

public:
    void initMotor() override;
    void setExpAngle(const int * angle);
    void setExpSpeed(const int * speed);
    void calculatePID() override;
    void sendControlCmd() override;
    void updateFeedBackInfo(uint8_t id, const uint8_t* rxData);
    void setPID(float p, float i, float d);
};

#endif

#endif
