/*
 * led.h
 *
 *  Created on: Mar 19, 2026
 *      Author: ynish
 */

#ifndef APP_LED_H_
#define APP_LED_H_

#include <stdint.h>
#include <stdbool.h>

extern uint16_t g_CurrentLED;

void LED_Init(void);
void LED_SetLED_Direction(uint16_t direction);


#endif /* APP_LED_H_ */
