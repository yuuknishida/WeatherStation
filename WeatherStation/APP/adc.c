/*
 * adc.c
 *
 *  Created on: Feb 26, 2026
 *      Author: ynish
 *
 *  Reads analog input from wind speed sensor (PE3)
 */

/*******************************************************************
*                               INCLUDE DIRECTORIES
*********************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"

void ADC0_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    ADCSequenceDisable(ADC0_BASE, 3);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);
}

/*******************************************************************
*                               ADC_Read()
*
* Triggers ADC0, waits for conversion, and returns 12-bit value.
* Returns 0 if timeout occurs.
*********************************************************************/
uint32_t ADC_ReadRawWindSpeed(void) {
    uint32_t value;
    uint32_t timeout = 100000;

    ADCIntClear(ADC0_BASE, 3);
    ADCProcessorTrigger(ADC0_BASE, 3);

    while(!ADCIntStatus(ADC0_BASE, 3, false)) {
        if (--timeout == 0) {
            UARTprintf("ADC TIMEOUT\r\n");
            return 0;
        }
    }
    ADCIntClear(ADC0_BASE, 3);
    ADCSequenceDataGet(ADC0_BASE, 3, &value);
    return value & 0x0FFF;
}

/*******************************************************************
*                               ADC_ReadForWindSpeed()
*
* 16-sample software average for noise reduction.
* Returns averaged 12-bit ADC value.
*********************************************************************/
uint32_t ADC_ReadForWindSpeed(void)
{
    uint32_t sum = 0;
    for(uint8_t i = 0;i<16;i++) {
        sum += ADC_ReadRawWindSpeed();
    }
    return sum / 16;
}

/*******************************************************************
*                               ADC_CalibrateWindSpeedFloor()
*
* 64-sample average at rest to measure the sensor noise floor.
* Call ONCE at startup (no wind) - prints idle mV and suggested
* WIND_IDLE_MV value to UART. Returns idle voltage in mV.
*********************************************************************/
uint32_t ADC_CalibrateWindSpeedFloor(void)
{
    uint32_t sum = 0;
    for(uint8_t i = 0; i < 64; i++) {
        sum += ADC_ReadRawWindSpeed();
    }
    uint32_t avg        = sum / 64;
    uint32_t voltage_mv = (avg * 3300U) / 4095U;

    UARTprintf("=== Wind Sensor Calibration ===\r\n");
    UARTprintf("Idle ADC : %u\r\n",       avg);
    UARTprintf("Idle mV  : %u mV\r\n",    voltage_mv);
    UARTprintf("Set WIND_IDLE_MV to: %u mV (idle + 150 mV deadband)\r\n",
               voltage_mv + 150U);
    UARTprintf("===============================\r\n");

    return voltage_mv;
}

void mq135_adc_init_pe1_ain2(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) { }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) { }

    // PE1 = AIN2 in this case (prev using PE4, change all counts of PE4)
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);

    ADCSequenceDisable(ADC0_BASE, 2);
    ADCIntClear(ADC0_BASE, 2);

    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 0,
                             ADC_CTL_CH2 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 2);
    ADCIntClear(ADC0_BASE, 2);
}

uint16_t mq135_adc_read(void) //need to read the air quality sensor readings
{
    uint32_t sample = 0;
    uint32_t timeout = 2000000; // ~tens of ms at 80 MHz

    ADCIntClear(ADC0_BASE, 2);
    ADCProcessorTrigger(ADC0_BASE, 2);

    while(!ADCIntStatus(ADC0_BASE, 2, false))
    {
        if(--timeout == 0)
            return 0xFFFF; // timeout marker
    }

    ADCSequenceDataGet(ADC0_BASE, 2, &sample);
    return (uint16_t)(sample & 0x0FFF);
}
