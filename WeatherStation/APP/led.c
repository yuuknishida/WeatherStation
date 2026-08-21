/*
 * led.c
 *
 *  Created on: Mar 19, 2026
 *      Author: ynish
 */
#include "led.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "weather.h"
#include "driverlib/sysctl.h"

uint16_t g_CurrentLED = NO_WIND;

typedef enum {
    LED_NORTH     = 0,
    LED_NORTHEAST = 1,
    LED_EAST      = 2,
    LED_SOUTHEAST = 3,
    LED_SOUTH     = 4,
    LED_SOUTHWEST = 5,
    LED_WEST      = 6,
    LED_NORTHWEST = 7,
    LED_NO_WIND   = 8,
    LED_COUNT     = 9
} LED_Index_t;

typedef struct {
    uint32_t port;
    uint8_t pin;
} LEDPin_t;

static const LEDPin_t s_ledMap[] = {
    /* NORTH           (0)  */ { GPIO_PORTD_BASE, GPIO_PIN_0 },
    /* NORTHEAST       (1)  */ { GPIO_PORTD_BASE, GPIO_PIN_1 },
    /* EAST            (2)  */ { GPIO_PORTD_BASE, GPIO_PIN_2 },
    /* SOUTHEAST       (3)  */ { GPIO_PORTD_BASE, GPIO_PIN_3 },
    /* SOUTH           (4)  */ { GPIO_PORTB_BASE, GPIO_PIN_4 },
    /* SOUTHWEST       (5) */ { GPIO_PORTB_BASE, GPIO_PIN_5 },
    /* WEST            (6) */ { GPIO_PORTC_BASE, GPIO_PIN_6 },
    /* NORTHWEST       (7) */ { GPIO_PORTC_BASE, GPIO_PIN_7 },
    /* NO_WIND         (8) */ { GPIO_PORTF_BASE, GPIO_PIN_1 },
};

static LED_Index_t direction_to_led(uint16_t direction)
{
    switch ((WindDirection)direction)
    {
        case NORTH:             return LED_NORTH;
        case NORTH_NORTHEAST:   return LED_NORTHEAST;
        case NORTHEAST:         return LED_NORTHEAST;
        case EAST_NORTHEAST:    return LED_NORTHEAST;
        case EAST:              return LED_EAST;
        case EAST_SOUTHEAST:    return LED_SOUTHEAST;
        case SOUTHEAST:         return LED_SOUTHEAST;
        case SOUTH_SOUTHEAST:   return LED_SOUTHEAST;
        case SOUTH:             return LED_SOUTH;
        case SOUTH_SOUTHWEST:   return LED_SOUTHWEST;
        case SOUTHWEST:         return LED_SOUTHWEST;
        case WEST_SOUTHWEST:    return LED_SOUTHWEST;
        case WEST:              return LED_WEST;
        case WEST_NORTHWEST:    return LED_NORTHWEST;
        case NORTHWEST:         return LED_NORTHWEST;
        case NORTH_NORTHWEST:   return LED_NORTH;
        case NORTH_2:           return LED_NORTH;
        case NO_WIND:
        default:                return LED_NO_WIND;
    }
}

void LED_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    /* PORTD: PD0-PD3 — N, NNE/NE/ENE, E, ESE/SE/SSE */
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,
                          GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    /* PORTB: PB4-PB5 — S, SSW/SW/WSW
     * PB0 is the DHT data pin (configured by DHT_Init as input).
     * We only touch PB4 and PB5 here — do NOT include GPIO_PIN_0. */
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    /* PORTC: PC6-PC7 — W, WNW/NW/NNW */
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    /* PORTF: PF1 — No-wind indicator
     * PF2/PF3 are OLED DC/RST, already set as output in main — safe to repeat. */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    /* Clear all LED pins to known-OFF state */
    GPIOPinWrite(GPIO_PORTD_BASE,
                 GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5, 0);
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

    g_CurrentLED = NO_WIND;
}


void LED_SetLED_Direction(uint16_t direction)
{
    LED_Index_t newLED = direction_to_led(direction);
    LED_Index_t curLED = direction_to_led(g_CurrentLED);

    /* Skip if already showing the same LED */
    if (curLED == newLED) {
        g_CurrentLED = direction;   /* still update logical state */
        return;
    }

    /* Turn off the previous LED */
    const LEDPin_t *prev = &s_ledMap[curLED];
    GPIOPinWrite(prev->port, prev->pin, 0);

    /* Turn on the new LED */
    const LEDPin_t *next = &s_ledMap[newLED];
    GPIOPinWrite(next->port, next->pin, next->pin);

    g_CurrentLED = direction;
}
