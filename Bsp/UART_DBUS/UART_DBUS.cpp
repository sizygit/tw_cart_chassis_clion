#include "string.h"
#include "stdlib.h"
#include "UART_DBUS.h"
#include "stdarg.h"

#include "MotorM2006.h"
#include "math.h"
double ch_spend[3] = { 0 , 0 , 0};
uint8_t   dbus_buf0[DBUS_BUFLEN];
uint8_t   dbus_buf1[DBUS_BUFLEN];

DR16_rc_info rc;
extern M2006 m2006;
/**
* @brief 			initialize the DBUS(UART3)
* @param[in]   none
*/
void dbus_uart_Init(void)
{
    /* open uart idle it */
    __HAL_UART_CLEAR_IDLEFLAG(&DBUS_HUART);
    /* enalbe idle interrupt */
    __HAL_UART_ENABLE_IT(&DBUS_HUART, UART_IT_IDLE);

    Uart_Receive_Dma_No_It(&DBUS_HUART, dbus_buf0,dbus_buf1, DBUS_MAX_LEN);
}


/**
* @brief 			enable global uart IT and do not use DMA transfer done it in use of Double buffer mode
* @param[in]   the UART_HandleTypeDef* param
*							the pData1 is DMA stream x memory 0 address register
*							the pData1 is DMA stream x memory 1 address register
*              UART Rx Transfer size
*/
int Uart_Receive_Dma_No_It(UART_HandleTypeDef* huart, uint8_t* pData1,uint8_t* pData2, uint32_t Size)
{
    uint32_t tmp1 = HAL_UART_STATE_RESET;

    tmp1 = huart->RxState;

    if (tmp1 == HAL_UART_STATE_READY)
    {
        if ((pData1 == NULL) || (Size == 0))
        {
            return HAL_ERROR;
        }

        huart->pRxBuffPtr = pData1;
        huart->RxXferSize = Size;
        huart->ErrorCode  = HAL_UART_ERROR_NONE;

//		/* Enable the DMA Stream */
//		HAL_DMA_Start(huart->hdmarx, (uint32_t)&huart->Instance->DR, (uint32_t)pData, Size);

        /*
         * Enable the DMA transfer for the uart receiver request by setting the DMAR bit
         * in the UART CR3 register
         */
        SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);
        //disable DMA
        __HAL_DMA_DISABLE(&hdma_usart3_rx);
        while(hdma_usart3_rx.Instance->CR & DMA_SxCR_EN)
        {
            __HAL_DMA_DISABLE(&hdma_usart3_rx);
        }
        hdma_usart3_rx.Instance->PAR = (uint32_t) & (huart->Instance->DR);
        //memory buffer 1
        hdma_usart3_rx.Instance->M0AR = (uint32_t)(pData1);
        //memory buffer 2
        hdma_usart3_rx.Instance->M1AR = (uint32_t)(pData2);
        //data length
        hdma_usart3_rx.Instance->NDTR = DBUS_BUFLEN;
        //enable double memory buffer
        SET_BIT(hdma_usart3_rx.Instance->CR, DMA_SxCR_DBM);
        //enable DMA

        __HAL_DMA_ENABLE(&hdma_usart3_rx);

        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/* UART1 -- DMA transfer*/
void uart1_tx_Init(void)
{
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);        // clear idle flag
    /* DMA2 Channe7 (triggered by USART1 Tx event) Config */

}

/**
  * @brief      returns the number of remaining data units in the current DMAy Streamx transfer.
  * @param[in]  dma_stream: where y can be 1 or 2 to select the DMA and x can be 0
  *             to 7 to select the DMA Stream.
  */
uint16_t dma_DR16_RX_BUFFER_SIZE(DMA_Stream_TypeDef *dma_stream)
{
    /* Return the number of remaining data units for DMAy Streamx */
    return ((uint16_t)(dma_stream->NDTR));
}


/**
 * @brief 			interrupt function of UART3(DBUS)
 * @param[in]   none
 */
void dr16_uart_rx_data_handle(void)
{
    if(huart3.Instance->SR & UART_FLAG_RXNE)//???????????????
    {
        __HAL_UART_CLEAR_PEFLAG(&huart3);
    }
    else if(USART3->SR & UART_FLAG_IDLE)
    {
        static uint16_t this_time_rx_len = 0;
        __HAL_UART_CLEAR_PEFLAG(&huart3);
        /* whether memory1 or memory 2*/
        if ((hdma_usart3_rx.Instance->CR & DMA_SxCR_CT) == RESET)
        {
            /* Current memory buffer used is Memory 0 */
            /* CT 0 -- 0    CT 1 -- 1 */
            //disable DMA
            __HAL_DMA_DISABLE(&hdma_usart3_rx);
            //get receive data length, length = set_data_length - remain_length
            this_time_rx_len = DBUS_MAX_LEN  - hdma_usart3_rx.Instance->NDTR;
            //reset set_data_lenght
            hdma_usart3_rx.Instance->NDTR = DBUS_MAX_LEN;

            //set memory buffer 1
            hdma_usart3_rx.Instance->CR |= DMA_SxCR_CT;
            //enable DMA
            __HAL_DMA_ENABLE(&hdma_usart3_rx);
            if(this_time_rx_len == DBUS_BUFLEN)
            {
                dbus_to_rc(&rc, dbus_buf0);
            }
        }
        else
        {
            /* Current memory buffer used is Memory 1 */
            //disable DMA
            __HAL_DMA_DISABLE(&hdma_usart3_rx);
            //get receive data length, length = set_data_length - remain_length
            this_time_rx_len = DBUS_MAX_LEN - hdma_usart3_rx.Instance->NDTR;
            //reset set_data_lenght
            hdma_usart3_rx.Instance->NDTR = DBUS_MAX_LEN;
            //set memory buffer 0
            DMA1_Stream1->CR &= ~(DMA_SxCR_CT);
            //enable DMA
            __HAL_DMA_ENABLE(&hdma_usart3_rx);
            if(this_time_rx_len == DBUS_BUFLEN)
            {
                //get dbus data to real data
                dbus_to_rc(&rc, dbus_buf1);
            }
        }
    }
}



/**
 * @brief 			analysis the DR16 dbus data frame to the real data
 * @param[in]   the struct that store the data from DR16 in DBUS
 *							 DMA stream x memory 0/1 address register
 */
static void dbus_to_rc(DR16_rc_info *rc, uint8_t *dbus_buff)
{

    //satori??????????????????????????????????????????????????????1024????????????????????????????????????0
    rc->ch[0] = (dbus_buff[0] | dbus_buff[1] << 8) & 0x07FF;
    rc->ch[0] -= 1024;
    rc->ch[1] = (dbus_buff[1] >> 3 | dbus_buff[2] << 5) & 0x07FF;
    rc->ch[1] -= 1024;
    rc->ch[2] = (dbus_buff[2] >> 6 | dbus_buff[3] << 2 | dbus_buff[4] << 10) & 0x07FF;
    rc->ch[2] -= 1024;
    rc->ch[3] = (dbus_buff[4] >> 1 | dbus_buff[5] << 7) & 0x07FF;
    rc->ch[3] -= 1024;

    //satori:?????????????????????????????????5?????????
    /* prevent remote control zero deviation */
    if(rc->ch[0] <= 5 && rc->ch[0] >= -5)
        rc->ch[0] = 0;
    if(rc->ch[1] <= 5 && rc->ch[1] >= -5)
        rc->ch[1] = 0;
    if(rc->ch[2] <= 5 && rc->ch[2] >= -5)
        rc->ch[2] = 0;
    if(rc->ch[3] <= 5 && rc->ch[3] >= -5)
        rc->ch[3] = 0;

    rc->sw1 = ((dbus_buff[5] >> 4) & 0x000C) >> 2;
    rc->sw2 = (dbus_buff[5] >> 4) & 0x0003;

    //satori:??????????????????
    if ((abs(rc->ch[0]) > 660) || \
      (abs(rc->ch[1]) > 660) || \
      (abs(rc->ch[2]) > 660) || \
      (abs(rc->ch[3]) > 660))
    {
        memset(rc, 0, sizeof(struct DR16_rc_info));
        return ;
    }

    rc->mouse.x = dbus_buff[6] | (dbus_buff[7] << 8); // x axis
    rc->mouse.y = dbus_buff[8] | (dbus_buff[9] << 8);
    rc->mouse.z = dbus_buff[10] | (dbus_buff[11] << 8);

    rc->mouse.l = dbus_buff[12];
    rc->mouse.r = dbus_buff[13];

    rc->kb.key_code = dbus_buff[14] | dbus_buff[15] << 8; // key borad code
    rc->wheel = (dbus_buff[16] | dbus_buff[17] << 8) - 1024;
    dr16_TO_M2006();
}
/******** DR16 's data to M2006 's data ************************/
/********  three wheel cart's control   ***********************/
void dr16_TO_M2006()
{
    /*int ch_spend_int[M2006::motorNum];
    for(int i = 0;i < M2006::motorNum; i++) {
        ch_spend[i] = M2006_MAX_SPEED_RPM / 660.0 * rc.ch[i];
        if(ch_spend[i] >0 && ch_spend[i] < 400)
            ch_spend[i] = 400;
        if(ch_spend[i] <0 && ch_spend[i] > -400)
            ch_spend[i] = -400;
        ch_spend_int[i] = ch_spend[i];
    }

    m2006.setExpSpeed(ch_spend_int); */   //for test

    int ch_spend_int[3] = {0};
    if(rc.ch[1] != 0){             // RIGHT  forward or back
        ch_spend[2] = 0;
        ch_spend[1] = - M2006_MAX_SPEED_RPM / 660.0 * rc.ch[1];
        ch_spend[0] =   M2006_MAX_SPEED_RPM / 660.0 * rc.ch[1];
        for(int i = 0;i < 3; i++) {
            if (ch_spend[i] > 0 && ch_spend[i] < 400)
                ch_spend[i] = 400;
            if (ch_spend[i] < 0 && ch_spend[i] > -400)
                ch_spend[i] = -400;
            ch_spend_int[i] = ch_spend[i];
        }
    }
    if(rc.ch[0] != 0){             // RIGHT  turn left or right
        ch_spend[0] +=   M2006_MAX_SPEED_RPM / 660.0 * rc.ch[0] * 0.5;
        ch_spend[1] +=   M2006_MAX_SPEED_RPM / 660.0 * rc.ch[0] * 0.5;
        ch_spend[2] += - M2006_MAX_SPEED_RPM / 660.0 * rc.ch[0] ;
        for(int i = 0;i < 3; i++) {
            if (ch_spend[i] > 0 && ch_spend[i] < 400)
                ch_spend[i] = 400;
            if (ch_spend[i] < 0 && ch_spend[i] > -400)
                ch_spend[i] = -400;
            ch_spend_int[i] = ch_spend[i];
        }
    }
    if(rc.ch[2] != 0){             // RIGHT  turn left or right
        ch_spend[0] +=   M2006_MAX_SPEED_RPM / 660.0 * rc.ch[2];
        ch_spend[1] +=   M2006_MAX_SPEED_RPM / 660.0 * rc.ch[2];
        ch_spend[2] +=   M2006_MAX_SPEED_RPM / 660.0 * rc.ch[2];
        for(int i = 0;i < 3; i++) {
            if (ch_spend[i] > 0 && ch_spend[i] < 400)
                ch_spend[i] = 400;
            if (ch_spend[i] < 0 && ch_spend[i] > -400)
                ch_spend[i] = -400;
            ch_spend_int[i] = ch_spend[i];
        }
    }
    m2006.setExpSpeed(ch_spend_int);
    for(int i =0;i<3;i++)
        ch_spend[i] = 0;

}