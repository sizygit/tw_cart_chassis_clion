//
// Created by nian on 2021/7/11.
//

#include "communication.h"
#include "can.h"
#include "cstring"

void Communication::initCommunication()
{
    hcan = &hcan1;
    CAN_Filter_Init(hcan);
    HAL_CAN_Start(hcan);
    HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void Communication::sendCommunicationFrame()
{
    static uint8_t data[FRAME_LEN];
    memcpy(data, (uint8_t*)&sendFrame, FRAME_LEN);
    HAL_CAN_Transmit(&hcan1, canSendID, data, FRAME_LEN);
}

void Communication::decodeCommunicationFrame(uint8_t * rxData)
{
    memcpy((uint8_t*)&receiveFrame, rxData, FRAME_LEN);
}