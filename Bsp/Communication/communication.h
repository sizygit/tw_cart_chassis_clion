//
// Created by nian on 2021/7/11.
//

#ifndef CHASSIS_COMMUNICATION_H
#define CHASSIS_COMMUNICATION_H

#include "main.h"

#define FRAME_LEN 5

class Communication
{
private:
    CAN_HandleTypeDef *hcan;

    struct communicateFrame
    {
        uint16_t gimbalYaw;
        uint16_t gimbalPitch;
        uint8_t shootCmd;
    };

    const int canSendID = 0x78A;
    const int canReceiveID = 0x78B;

public:
    struct communicateFrame sendFrame;
    struct communicateFrame receiveFrame;
    void initCommunication();
    void sendCommunicationFrame();
    void decodeCommunicationFrame(uint8_t * rxData);
};






#endif //CHASSIS_COMMUNICATION_H
