/*
 * dht.c
 *
 *  Created on: Mar 11, 2026
 *      Author: ynish
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "driverlib/systick.h"

#define DHT_PORT GPIO_PORTB_BASE
#define DHT_PIN  GPIO_PIN_0
#define TIMEOUT  200000

void delay_us(uint32_t us)
{
    SysCtlDelay((SysCtlClockGet() / 3000000) * us);
}

void DHT_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinTypeGPIOInput(DHT_PORT, DHT_PIN);

    GPIOPadConfigSet(DHT_PORT,
                     DHT_PIN,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);  // enable weak pull-up
}

bool waitForState(uint8_t state)
{
    volatile uint32_t timeout = 0;

    while((GPIOPinRead(DHT_PORT, DHT_PIN) ? 1 : 0) == state)
    {
        if(timeout++ > TIMEOUT)
            return false;
    }
    return true;
}

bool DHT_Read(uint8_t *temp, uint8_t *hum)
{
    uint8_t data[5] = {0};
    uint8_t i, j;

    IntMasterDisable();

    // ----- Start signal -----
    GPIOPinTypeGPIOOutput(DHT_PORT, DHT_PIN);
    GPIOPinWrite(DHT_PORT, DHT_PIN, 0);
    SysCtlDelay(SysCtlClockGet() / 3 / 50);   // ~20 ms

    // ADDED 3/22/2026
    GPIOPinWrite(DHT_PORT, DHT_PIN, DHT_PIN);
    delay_us(40);
    GPIOPinTypeGPIOInput(DHT_PORT, DHT_PIN);
    GPIOPadConfigSet(DHT_PORT, DHT_PIN,
                         GPIO_STRENGTH_2MA,
                         GPIO_PIN_TYPE_STD_WPU);

    ///////////////////////////////

    // ----- Sensor response -----
    // ----- Sensor response -----
    if(!waitForState(1)) goto fail;  // wait for LOW
    if(!waitForState(0)) goto fail;  // wait for HIGH
    if(!waitForState(1)) goto fail;  // wait for LOW (data start)

    // ----- Read 40 bits -----
    for(i = 0; i < 5; i++)
    {
        for(j = 0; j < 8; j++)
        {
            if(!waitForState(0)) goto fail;

            delay_us(50);   // optimal sample point

            if(GPIOPinRead(DHT_PORT, DHT_PIN))
                data[i] |= (1 << (7 - j));

            if(!waitForState(1)) goto fail;
        }
    }

    IntMasterEnable();

    // ----- Checksum -----
    if((data[0] + data[1] + data[2] + data[3]) != data[4])
        return false;

    *hum  = data[0];
    *temp = data[2];
    return true;
fail:
    IntMasterEnable();
    return false;
}


