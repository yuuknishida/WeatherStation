#ifndef APP_UART_H_
#define APP_UART_H_

#include "weather.h"


/*******************************************************************
*                               FUNCTION PROTOTYPES
********************************************************************/
void UART0_Init(void);
void UART5_Init(void);
//static void RS485_SendRequest(const uint8_t *buf, uint32_t len);
#endif /* APP_UART_H_ */
