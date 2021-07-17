//
// Created by nian on 2021/6/8.
//

#include "motor_task.h"
#include "MotorM2006.h"
//#include "MotorM3508.h"
//#include "MotorGM6020.h"
//#include "MotorSnail2305.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

#define MAX_CUR 0x150
#define AMPLITUDE 0x80

int expCurrent[4] = {MAX_CUR,0,0,0};
uint8_t dir = 0;

//extern M2006 m2006;
extern M2006 m2006;
//extern GM6020 gm6020;
//extern Snail2305 snail2305;

void spinCWandCCW()
{
    if(dir)
    {
        if(expCurrent[0]<MAX_CUR)
        {
            expCurrent[0] += AMPLITUDE;
        }
        else
        {
            expCurrent[0] = MAX_CUR;
            dir = 0;
        }
    }
    else
    {
        if(expCurrent[0]>-MAX_CUR)
        {
            expCurrent[0] -= AMPLITUDE;
        }
        else
        {
            expCurrent[0] = -MAX_CUR;
            dir = 1;
        }
    }
}

void motor_task()
{
    //m2006.updateExpCurrent(expCurrent);

//    int16_t speed[4] = { -700, 0 , 0, 0};
//    m2006.updateExpSpeed(speed);
//    gm6020.initMotor();
    //snail2305.initMotor();
    m2006.initMotor();
//    m2006.setExpSpeed((int *)speed);
    m2006.setPID( 1, 0,0.2);
    HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
    while (1)
    {
//        spinCWandCCW();
//        m2006.sendControlCmd();
        m2006.calculatePID();
        m2006.sendControlCmd();
//        gm6020.sendControlCmd();
//        snail2305.sendControlCmd();
        osDelay(2);
    }
}

