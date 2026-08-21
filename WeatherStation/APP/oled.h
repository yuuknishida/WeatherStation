/*
 * oled.h
 *
 *  Created on: Mar 11, 2026
 *      Author: ynish
 */

#ifndef APP_OLED_H_
#define APP_OLED_H_

#define OLED_SSI_BASE   SSI0_BASE

#define OLED_DC_PORT    GPIO_PORTF_BASE
#define OLED_DC_PIN     GPIO_PIN_2 //changed to PF2

#define OLED_RST_PORT   GPIO_PORTF_BASE
#define OLED_RST_PIN    GPIO_PIN_3 //changed to PF3

#define OLED_CS_PORT    GPIO_PORTA_BASE
#define OLED_CS_PIN     GPIO_PIN_3 //defining chip select CS on the OLED to PB3

inline void cs_low(void);//make CS lo
inline void cs_high(void); //CS is high

void delay_ms(uint32_t ms);
void ssi0_write8(uint8_t b);
void u16_to_str(char *out, uint16_t v);
void oled_reset(void);
void oled_cmd1(uint8_t c);
void oled_cmd2(uint8_t c, uint8_t a);
  void oled_cmd3(uint8_t c, uint8_t a, uint8_t b);
  void oled_data_fill(uint8_t val, uint32_t n);
  void oled_data_burst(const uint8_t *buf, uint32_t n);
  void oled_set_addr(uint8_t col, uint8_t page);
  void oled_clear_page(uint8_t page);
  void oled_clear(void);
  void oled_put_char(uint8_t col, uint8_t page, char ch);
  void oled_put_str(uint8_t col, uint8_t page, const char *s);
  void oled_init(void);
  void mq135_adc_init_pe1_ain2(void); //changed up the pin to PE1 instead
  uint16_t mq135_adc_read(void); //need to read the air quality sensor readings





#endif /* APP_OLED_H_ */
