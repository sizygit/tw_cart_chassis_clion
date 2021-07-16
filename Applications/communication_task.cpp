//
// Created by nian on 2021/7/11.
//

#include "communication_task.h"
#include "communication.h"
#include "cmsis_os.h"

Communication chassisComInfo;

void communication_task()
{
    chassisComInfo.initCommunication();
    while (1)
    {
        //chassisComInfo.sendCommunicationFrame();
        osDelay(2);
    }
}