/*
 * oled.c
 *
 *  Created on: Mar 11, 2026
 *      Author: ynish
 */

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"

#include "lettersAndColon.h"
#include "oled.h"


 void delay_ms(uint32_t ms)
{
    SysCtlDelay((SysCtlClockGet() / 3 / 1000) * ms);
}

//functions for making chip select on the OLED HI or LO, turning on or off
inline void cs_low(void)  { GPIOPinWrite(OLED_CS_PORT, OLED_CS_PIN, 0); } //make CS lo
inline void cs_high(void) { GPIOPinWrite(OLED_CS_PORT, OLED_CS_PIN, OLED_CS_PIN); } //CS is high

 void ssi0_write8(uint8_t b)
{
    uint32_t trash;
    SSIDataPut(OLED_SSI_BASE, b);
    while (SSIBusy(OLED_SSI_BASE)) { }
    while (SSIDataGetNonBlocking(OLED_SSI_BASE, &trash)) { }
}

 void u16_to_str(char *out, uint16_t v)
{
    char buf[6];
    int i = 0;

    if (v == 0) { out[0] = '0'; out[1] = 0; return; }

    while (v > 0 && i < 5) { buf[i++] = (char)('0' + (v % 10)); v /= 10; }
    for (int j = 0; j < i; j++) out[j] = buf[i - 1 - j];
    out[i] = 0;
}

 void oled_reset(void)
{
    GPIOPinWrite(OLED_RST_PORT, OLED_RST_PIN, 0); //clear RST pin first
    delay_ms(20);
    GPIOPinWrite(OLED_RST_PORT, OLED_RST_PIN, OLED_RST_PIN);
    delay_ms(20);
}

// ---- Command helpers (when we need CS to be low for a entire command section) ----
 void oled_cmd1(uint8_t c)
{
    IntMasterDisable();
    GPIOPinWrite(OLED_DC_PORT, OLED_DC_PIN, 0); // command
    cs_low();
    ssi0_write8(c); //input the argument c from the function input
    cs_high();
    IntMasterEnable();
}

 void oled_cmd2(uint8_t c, uint8_t a)
{
    IntMasterDisable();
    GPIOPinWrite(OLED_DC_PORT, OLED_DC_PIN, 0); // command
    cs_low();
    ssi0_write8(c); //input the arguments c, a
    ssi0_write8(a);
    cs_high();
    IntMasterEnable();
}

 void oled_cmd3(uint8_t c, uint8_t a, uint8_t b)
{
    IntMasterDisable();
    GPIOPinWrite(OLED_DC_PORT, OLED_DC_PIN, 0); // command
    cs_low();
    ssi0_write8(c); //input arguments c, a, b,
    ssi0_write8(a);
    ssi0_write8(b);
    cs_high();
    IntMasterEnable();
}



 void oled_data_fill(uint8_t val, uint32_t n)
{
    IntMasterDisable();
    GPIOPinWrite(OLED_DC_PORT, OLED_DC_PIN, OLED_DC_PIN); // data
    cs_low();
    for (uint32_t i = 0; i < n; i++) ssi0_write8(val);
    cs_high();
    IntMasterEnable();
}

 void oled_data_burst(const uint8_t *buf, uint32_t n)
{
    IntMasterDisable();
    GPIOPinWrite(OLED_DC_PORT, OLED_DC_PIN, OLED_DC_PIN); // data
    cs_low();
    for (uint32_t i = 0; i < n; i++) ssi0_write8(buf[i]);
    cs_high();
    IntMasterEnable();
}

 void oled_set_addr(uint8_t col, uint8_t page)
{
    // SSD1306 horizontal addressing
    oled_cmd3(0x21, col, 127);   // column start/end
    oled_cmd3(0x22, page, 7);    // page start/end
}
 void oled_clear_page(uint8_t page)
{
    oled_set_addr(0, page);
    oled_data_fill(0x00, 128); //fill with halfway btwn 0-255 (med intensity, grey)
}
 void oled_clear(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        oled_set_addr(0, page);
        oled_data_fill(0x00, 128);//fill with halfway btwn 0-255 (med intensity, grey)
    }
}

// Draw one character using the table in lettersAndColon.h
 void oled_put_char(uint8_t col, uint8_t page, char ch)
{
    uint8_t out[6] = {0,0,0,0,0,0};   // 5 columns + 1 spacer
    const uint8_t *g = glyph5_letters_colon_equal(ch);

    if (g != NULL)
    {
        out[0] = g[0];
        out[1] = g[1];
        out[2] = g[2];
        out[3] = g[3];
        out[4] = g[4];
        // out[5] stays 0 for spacing
    }

    oled_set_addr(col, page);
    oled_data_burst(out, 6);
}

 void oled_put_str(uint8_t col, uint8_t page, const char *s)
{
    while (*s && col <= 122)   // 128pixels wide, 6pixels per char
    {
        oled_put_char(col, page, *s++);
        col += 6;
    }
}

// ===== OLED initializatin + visibility test =====
 void oled_init(void)
{
    oled_reset();

    // SSD1306 initialion sequence
    oled_cmd1(0xAE);                 // display off
    oled_cmd2(0xD5, 0x80);           // clock divide
    oled_cmd2(0xA8, 0x3F);           // multiplex 1/64
    oled_cmd2(0xD3, 0x00);           // display offset
    oled_cmd1(0x40);                 // start line 0
    oled_cmd2(0x8D, 0x14);           // charge pump ON
    oled_cmd2(0x20, 0x00);           // memory mode = horizontal
    oled_cmd1(0xA1);                 // segment remap
    oled_cmd1(0xC8);                 // COM scan direction
    oled_cmd2(0xDA, 0x12);           // COM pins
    oled_cmd2(0x81, 0xCF);           // contrast
    oled_cmd2(0xD9, 0xF1);           // precharge
    oled_cmd2(0xDB, 0x40);           // VCOM detect
    oled_cmd1(0xA4);                 // resume RAM display
    oled_cmd1(0xA6);                 // normal display
    oled_cmd1(0xAF);                 // display on

    // Strong visibility test: fill screen white then clear
    for (uint8_t page = 0; page < 8; page++)
    {
        oled_set_addr(0, page);
        oled_data_fill(0xFF, 128);
    }
    delay_ms(500);
    oled_clear();

}
