#ifndef APP_ADC_H_
#define APP_ADC_H_

#include <stdint.h>

/*******************************************************************
*                               FUNCTION PROTOTYPES
*********************************************************************/
void ADC0_Init(void);
void mq135_adc_init_pe1_ain2(void); //changed up the pin to PE1 instead

uint32_t ADC_ReadRawWindSpeed(void);
uint32_t ADC_ReadForWindSpeed(void);
uint32_t ADC_CalibrateWindSpeedFloor(void);
uint16_t mq135_adc_read(void); //need to read the air quality sensor readings



#endif /* APP_ADC_H_ */
