//
// Created by nian on 2021/6/8.
//

#include "MotorM2006.h"

#ifdef ENABLE_M2006

#include "can.h"
#include "cmsis_os.h"

M2006 m2006{};

void M2006::initMotor() {
    hcan = &hcan1;
    CAN_Filter_Init(hcan);
    HAL_CAN_Start(hcan);
    HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    hcan = &hcan2;
    CAN_Filter_Init(hcan);
    HAL_CAN_Start(hcan);
    HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void M2006::setExpAngle(const int * angle)
{
    for (int i = 0; i < motorNum; ++i) {
        expAngle[i] = angle[i];
    }
}

void M2006::setExpSpeed(const int * speed)
{
    for (int i = 0; i < motorNum; ++i) {
            expSpeed[i] = speed[i];
    }
}

void M2006::calculatePID() {
    for (int i = 0; i < motorNum; ++i) {
        expCurrent[i] = PIDSpeedRing.PID_Output(this->feedBackInfo.speed[i], expSpeed[i]);
    }
}

void M2006::sendControlCmd() {
    uint8_t data[8] = {0};
/*    for (uint8_t i = 0, j = 0; i < motorNum; i++) {
        data[j++] = expCurrent[i] >> 8;
        data[j++] = expCurrent[i];
    }
    HAL_CAN_Transmit(hcan, protocolID, data, CAN_DLC);*/     //when only use a CAN

/*******  when use two CAN and CAN1 ID is 0 ~ motor_CAN2ID-1,CAN2 ID is motor_CAN2ID ~ motorNum ***/
    for (uint8_t i = 0, j = 0; i < m2006.motor_CAN2ID - 1; i++) {
        data[j++] = expCurrent[i] >> 8;
        data[j++] = expCurrent[i];
    }
    HAL_CAN_Transmit(&hcan1, protocolID, data, CAN_DLC);
    uint8_t data2 [8] = {0};
    for (uint8_t i = (m2006.motor_CAN2ID - 1), j = ((m2006.motor_CAN2ID - 1) * 2); i < m2006.motorNum; i++) {
        data2[j++] = expCurrent[i] >> 8;
        data2[j++] = expCurrent[i];
    }
    HAL_CAN_Transmit(&hcan2, protocolID, data2, CAN_DLC);
}

void M2006::updateFeedBackInfo(uint8_t id, const uint8_t* rxData) {
    this->feedBackInfo.angle[id - 1] = ((uint16_t) rxData[0] << 8) | rxData[1];
    feedBackInfo.speed[id - 1] = ((uint16_t) rxData[2] << 8) | rxData[3];
    feedBackInfo.current[id - 1] = ((uint16_t) rxData[4] << 8) | rxData[5];
    feedBackInfo.temperature[id - 1] = rxData[6];
}



void M2006::setPID(float p, float i, float d)
{
    speedRingPara.kp = p;
    speedRingPara.ki = i;
    speedRingPara.kd = d;
}

#endif
