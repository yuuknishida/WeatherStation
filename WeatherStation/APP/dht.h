/*
 * DHT.h
 *
 *  Created on: Mar 11, 2026
 *      Author: ynish
 */

#ifndef APP_DHT_H_
#define APP_DHT_H_

void delay_us(uint32_t us);
void DHT_Init(void);
bool waitForState(uint8_t state);
bool DHT_Read(uint8_t *temp, uint8_t *hum);

#endif /* APP_DHT_H_ */
