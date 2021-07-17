
#ifndef __BSP_UART_H__
#define __BSP_UART_H__



#include "main.h"
#include "usart.h"



#define UART_RX_DMA_SIZE (1024)
#define DBUS_MAX_LEN     (50)
#define DBUS_BUFLEN      (18)   //18 bytes of data frames
#define DBUS_HUART       huart3 /* for remote controler reciever */
#define TX_BUFF_LEN      (18)



/* define DBUS data structures  */
typedef struct DR16_rc_info
{
  /* rocker channel information */
  int16_t ch[4];                  //   =660  - +660

  /* left and right lever information */
  uint8_t sw1;
  uint8_t sw2;
  /* mouse movement and button information */
  struct
  {
    int16_t x;
    int16_t y;
    int16_t z;

    uint8_t l;
    uint8_t r;
  } mouse;
  /* keyboard key information */
   union {
    uint16_t key_code;
    struct
    {
      uint16_t W : 1;
      uint16_t S : 1;
      uint16_t A : 1;
      uint16_t D : 1;
      uint16_t SHIFT : 1;
      uint16_t CTRL : 1;
      uint16_t Q : 1;
      uint16_t E : 1;
      uint16_t R : 1;
      uint16_t F : 1;
      uint16_t G : 1;
      uint16_t Z : 1;
      uint16_t X : 1;
      uint16_t C : 1;
      uint16_t V : 1;
      uint16_t B : 1;
    } bit;
  } kb;
  int16_t wheel;
}DR16_rc_info;

extern DR16_rc_info rc;

int Uart_Receive_Dma_No_It(UART_HandleTypeDef* huart, uint8_t* pData1,uint8_t* pData2, uint32_t Size);
static void dbus_to_rc(DR16_rc_info *rc, uint8_t *dbus_buff);
void dr16_TO_M2006();

#ifdef __cplusplus
extern "C" {
#endif

void dr16_uart_rx_data_handle(void);
void dbus_uart_Init(void);

#ifdef __cplusplus
}
#endif


#endif

