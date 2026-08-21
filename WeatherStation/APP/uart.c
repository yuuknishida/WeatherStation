/*
 * uart.c
 *
 *  Created on: Feb 26, 2026
 *      Author: ynish
 */

/*******************************************************************
*                               INCLUDE DIRECTORIES
*********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"

#include "weather.h"

/*******************************************************************
*                               UART0_Init()
*
*   Configures UART0 (PA0=RX, PA1=TX) for 9600 baud communication.
*********************************************************************/
void UART0_Init(void)
{
    /*
     * Enables GPIOA PA0 and PA1 for UART0 RX AND TX
     *
     * */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 9600, SysCtlClockGet());
}
void UART5_Init(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART5));

    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 9600,
            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTEnable(UART5_BASE);
}

//static void RS485_SendRequest(const uint8_t *buf, uint32_t len)
//{
//    for(uint32_t i =0;i<len;i++)
//    {
//        UARTCharPut(UART5_BASE, buf[i]);
//        while(UARTBusy(UART5_BASE));
//    }
//}
