//
// Created by nian on 2021/6/19.
//

#include "cmsis_os.h"
#include "can.h"
#include "main.h"
#include "MotorM2006.h"
#include "communication.h"

extern M2006 m2006;
//extern M3508 m3508;
//extern GM6020 gm6020;
int16_t  rx_spend[m2006.motorNum]; //for test

extern Communication chassisComInfo;

#ifdef __cplusplus
extern "C" {
#endif

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rxHeader;

    uint8_t id;
    static uint8_t rxData[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData);

    if (hcan == &hcan1)
    {
        //For m2006
        id = rxHeader.StdId - m2006.protocolID;
        m2006.updateFeedBackInfo(id, rxData);
        rx_spend[id - 1] = m2006.feedBackInfo.speed[id - 1];

        //for m3508
//        id = rxHeader.StdId - m3508.protocolID;
//        m2006.updateFeedBackInfo(id, rxData);

//      for GM6020
//        id = rxHeader.StdId - 0x204;
//        gm6020.updateFeedBackInfo(id, rxData);

        //debug
        //HAL_GPIO_WritePin(GPIOH, GPIO_PIN_10, GPIO_PIN_SET);  //Blue
    }

    if (hcan == &hcan2)
    {
        //chassisComInfo.decodeCommunicationFrame(rxData);
        //debug

        //For m2006
        id = rxHeader.StdId - m2006.protocolID;
        m2006.updateFeedBackInfo(id, rxData);
        rx_spend[id - 1] = m2006.feedBackInfo.speed[id - 1];

        //HAL_GPIO_WritePin(GPIOH, GPIO_PIN_12, GPIO_PIN_RESET);  //Red
    }
}

#ifdef __cplusplus
}
#endif