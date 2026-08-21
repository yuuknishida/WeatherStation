/*
 * main.c
 *
 *  Created on: Feb 23, 2026
 *      Author: ynish
 *
 *
 * Naming Conventions:
 * Task Functions - <Task>_<TaskName> (e.g. Task_WindSpeed)
 * Helper Functions - <Module>_<Action> (e.g. ADC_Read)
 *
 *
 */

/*******************************************************************
*                               INCLUDE DIRECTORIES
*********************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

#include "weather.h"
#include "uart.h"
#include "adc.h"
#include "OS.h"
#include "dht.h"
#include "oled.h"
#include "mailbox.h"
#include "led.h"
/*******************************************************************
*                               DEFINITION DECLARATIONS
*********************************************************************/
#define CPU_FREQ    80000000UL
#define DEMCR       (*((volatile uint32_t *)0xE000EDFC))              // Debug Exception and Monitor Control Register
#define DWT_CTRL    (*((volatile uint32_t *)0xE0001000))           // Control Register
#define DWT_CYCCNT  (*((volatile uint32_t *)0xE0001004))         // Cycle Count Register

#define ADC_MAX         4095U         // 12-bit ADC max value
#define VREF            3300U          // Reference voltage in V

#define WIND_IDLE_MV    450U           // Minimum voltage to detect wind
#define WIND_MAX_MV     2000U          // Max voltage span for sensor
#define SPEED_MAX_MMS   32400U         // Maximum speed in mm/s
#define MMS_TO_MPH      447U           // Conversion factor from mm/s to mph

#define WIND_MIN_MPH    1000U

#define WIND_CONFIRM_COUNT 3

#define OLED_PAGE_AIRQ 0
#define OLED_PAGE_WIND 1    /* wind speed */
#define OLED_PAGE_DIR  2    /* wind direction */
#define OLED_PAGE_TEMP 3
#define OLED_PAGE_HUM  4

//static inline uint32_t cycles_to_us(uint32_t cycles)
//{
//    return cycles / (CPU_FREQ / 1000000);
//}
//
//static inline uint32_t cycles_to_ms(uint32_t cycles)
//{
//    return cycles / (CPU_FREQ / 1000);
//}

/*******************************************************************
*                               GLOBAL VARIABLES
*********************************************************************/
static WeatherData_t    g_WeatherData;          // Shared weather data structure
static volatile bool g_windSpeedReady = false;

static volatile bool g_windActive = false;
static volatile uint8_t g_windConsecutive = 0;
/*******************************************************************
*                               FUNCTION PROTOTYPES
*********************************************************************/
static void System_Init();                 // Initialize system clock and peripherals
static void CycleCounter_Init();
static void Task_WindSpeed();       // Task to read wind speed from ADC
static void Task_WindDirection();
static void Task_TemperatureAndHumidity();
static void Task_AirQuality();
static void Task_DisplayUpdate();
/*******************************************************************
*                               systemInit()
*
*   Configures system clock for TM4C123G 80MHz
*********************************************************************/
static void System_Init()
{
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
}
/*******************************************************************
*                               CycleCounter_Init()
*
*   Configures for counting ticks
*********************************************************************/
static void CycleCounter_Init()
{
    DEMCR |= 0x01000000;
    DWT_CYCCNT = 0;
    DWT_CTRL |= 1;
}

/*******************************************************************
*                               WindSpeedTask()
*
*   Task to read wind Speed Sensor and convert to mph
*********************************************************************/
static void Task_WindSpeed() {
    while(1) {
//        START TIME HERE ---------------
//        uint32_t start = DWT_CYCCNT;

        uint16_t adcValue = ADC_ReadForWindSpeed();
        uint16_t voltage_mv = (adcValue * VREF) / ADC_MAX;
//        UARTprintf("ADC: %u", adcValue);
//        UARTprintf("Voltage MV: %u\n", voltage_mv);
        uint16_t windSpeed_mph = 0;

        if (voltage_mv > WIND_IDLE_MV)
        {
            uint16_t span_mv = voltage_mv - WIND_IDLE_MV;
            uint16_t max_span = WIND_MAX_MV - WIND_IDLE_MV;
            if (span_mv > max_span) {       // measure voltage above threshold minimum voltage
                span_mv = max_span;
            }
            uint16_t speed_mms = (uint16_t)((SPEED_MAX_MMS * (uint32_t)span_mv) / max_span);
            windSpeed_mph = (uint16_t)((speed_mms * 1000U) / MMS_TO_MPH);

            if (windSpeed_mph < WIND_MIN_MPH) windSpeed_mph = 0;
        }
        bool noWindy = (windSpeed_mph >= WIND_MIN_MPH);

        if(noWindy == g_windActive)
        {
            g_windConsecutive = 0;
        }
        else
        {
            g_windConsecutive++;
            if(g_windConsecutive >= WIND_CONFIRM_COUNT)
            {
                g_windActive = noWindy;
                g_windConsecutive = 0;
            }
        }

        g_WeatherData.windSpeed_mph = windSpeed_mph;
        g_windSpeedReady = true;

//        STOP HERE ----------------------
//        uint32_t end = DWT_CYCCNT;
//
//        uint32_t cycles = end - start;
//        uint32_t time_us = cycles_to_us(cycles);
//        UARTprintf("%u\n", time_us);
        OS_Sleep();
    }
}

/*******************************************************************
*                               Task_WindDirection()
*********************************************************************/
static void Task_WindDirection()
{
    while(1)
    {
        // START HERE ---------------------------
//        uint32_t start = DWT_CYCCNT;
        if (!g_windSpeedReady)
        {
//            uint32_t end = DWT_CYCCNT;
//            uint32_t cycles = end - start;
//            uint32_t time_ms = cycles_to_ms(cycles);
//            UARTprintf("%u\n", time_ms);

            OS_Sleep();
            continue;
        }

        // consume flag
        g_windSpeedReady = false;

        uint16_t speed = g_WeatherData.windSpeed_mph / 1000;

        // No-wind path
        if(speed == 0)
        {
            g_WeatherData.windDirection = NO_WIND;

//            uint32_t end = DWT_CYCCNT;
//            uint32_t cycles = end - start;
//            uint32_t time_ms = cycles_to_ms(cycles);
//            UARTprintf("%u\n", time_ms);
            LED_SetLED_Direction(NO_WIND);
            OS_Sleep();
            continue;
        }

        // Wind Detected

        // Flush any stale RX bytes
        while(UARTCharsAvail(UART5_BASE))
        {
            UARTCharGetNonBlocking(UART5_BASE);
        }

        uint8_t request[8] = {0x02, 0x03, 0x00, 0x01,
                              0x00, 0x01, 0xD5, 0xF9};

        for (int i = 0; i < 8; i++) {
            UARTCharPut(UART5_BASE, request[i]);
        }
        while(UARTBusy(UART5_BASE));

        // FLUSH TX ECHO
        SysCtlDelay(SysCtlClockGet() / 3 / 1000);
        while(UARTCharsAvail(UART5_BASE)){
            UARTCharGetNonBlocking(UART5_BASE);
        }

        // Read 7-byte response
        uint8_t response[7];
        bool timeout = false;
        for (int i = 0; i < 7; i++) {
            uint32_t t = 200000;
            while(!UARTCharsAvail(UART5_BASE))
            {
                if (--t == 0) { timeout = true; break; }
            }
            if (timeout) break;
            response[i] = (uint8_t)UARTCharGetNonBlocking(UART5_BASE);
        }

        if (timeout)
        {
//            UARTprintf("Wind Speed: %u mph\t Wind Direction: TIMEOUT\t", mph_whole);
        }
        else {
            uint16_t raw_dir = ((uint16_t)response[3] << 8) | response[4];
            g_WeatherData.windDirection = (raw_dir <= NORTH_2) ? (WindDirection)raw_dir : NO_WIND;
            LED_SetLED_Direction(g_WeatherData.windDirection);
        }
//        uint32_t end = DWT_CYCCNT;
//        uint32_t cycles = end - start;
//        uint32_t time_ms = cycles_to_ms(cycles);
//        UARTprintf("%u\n", time_ms);

        OS_Sleep();
    }
}

/*******************************************************************
*                               Task_TemperatureAndHumidity()
*********************************************************************/
static void Task_TemperatureAndHumidity()
{
    uint8_t temp = 0, hum = 0;
    while(1)
    {
//        uint32_t start = DWT_CYCCNT;
        bool ok = DHT_Read(&temp, &hum);

        if(ok)
        {
            g_WeatherData.temperature = temp;
            g_WeatherData.humidity = hum;
        }

//        uint32_t end = DWT_CYCCNT;
//        uint32_t cycles = end - start;
//        uint32_t time_ms = cycles_to_ms(cycles);
//        UARTprintf("%u\n", time_ms);
        OS_Sleep();
    }
}

/*******************************************************************
*                               Task_AirQuality()
*********************************************************************/
static void Task_AirQuality()
{

    while(1) {
//        uint32_t start = DWT_CYCCNT;
        uint16_t adc = mq135_adc_read();

        if (adc == 0xFFFF) //if I am incorrectly reading the wrong thing
        {
            UARTprintf("AirQ: ADC TIMEOUT\n");
        }
        else
        {
            g_WeatherData.airQuality = adc;
        }
//        uint32_t end = DWT_CYCCNT;
//        uint32_t cycles = end - start;
//        uint32_t time_ms = cycles_to_ms(cycles);
//        UARTprintf("%u\n", time_ms);

        OS_Sleep();
    }
}

static void Task_DisplayUpdate()
{
    char bufAQ[10], bufWS[10], bufTmp[10], bufHum[10];

    while(1)
    {
        uint32_t start = DWT_CYCCNT;
        /* --- Atomic snapshot ----------------------------------------
         * Disable interrupts just long enough to copy the 7-byte
         * struct so no sensor task can update it mid-copy.          */
        IntMasterDisable();
        WeatherData_t snap = g_WeatherData;
        IntMasterEnable();

        /* --- OLED update ------------------------------------------ */
        u16_to_str(bufAQ,  snap.airQuality);
        u16_to_str(bufWS,  snap.windSpeed_mph / 1000);
        u16_to_str(bufTmp, snap.temperature);
        u16_to_str(bufHum, snap.humidity);

        /* Air quality */
        oled_clear_page(OLED_PAGE_AIRQ);
        oled_put_str(0,  OLED_PAGE_AIRQ, "AQ:");
        oled_put_str(50, OLED_PAGE_AIRQ, bufAQ);

        /* Wind speed & direction */
        oled_clear_page(OLED_PAGE_WIND);
        oled_clear_page(OLED_PAGE_DIR);
        if (snap.windSpeed_mph == 0)
        {
            oled_put_str(0, OLED_PAGE_WIND, "WS: 0 mph");
            oled_put_str(0, OLED_PAGE_DIR,  "WD: NoWind");
        }
        else
        {
            oled_put_str(0,  OLED_PAGE_WIND, "WS:");
            oled_put_str(50, OLED_PAGE_WIND, bufWS);
            oled_put_str(0,  OLED_PAGE_DIR,  "WD:");
            oled_put_str(50, OLED_PAGE_DIR,  getWindDirection(snap.windDirection));
        }

        /* Temperature & humidity */
        oled_clear_page(OLED_PAGE_TEMP);
        oled_clear_page(OLED_PAGE_HUM);
        if (snap.temperature == 0 || snap.humidity == 0)
        {
            oled_put_str(0, OLED_PAGE_TEMP, "DHT Err");
        }
        else
        {
            oled_put_str(0,  OLED_PAGE_TEMP, "Tmp:");
            oled_put_str(50, OLED_PAGE_TEMP, bufTmp);
            oled_put_str(0,  OLED_PAGE_HUM,  "Hum:");
            oled_put_str(50, OLED_PAGE_HUM,  bufHum);
        }

        /* --- Single UART line (never split by a context switch) --- */
        if (snap.temperature == 0 || snap.humidity == 0)
        {
            UARTprintf("AirQ: %u  WS: %u mph  WD: %s  DHT Err\n",
                       snap.airQuality,
                       snap.windSpeed_mph / 1000,
                       getWindDirection(snap.windDirection));
        }
        else
        {
            UARTprintf("AirQ: %u  WS: %u mph  WD: %s  Tmp: %d C  Hum: %d %%\n",
                       snap.airQuality,
                       snap.windSpeed_mph / 1000,
                       getWindDirection(snap.windDirection),
                       snap.temperature,
                       snap.humidity);
        }
        uint32_t end = DWT_CYCCNT;
        uint32_t cycles = end - start;
        uint32_t time_ms = cycles_to_ms(cycles);
        UARTprintf("%u\n", time_ms);
        OS_Sleep();
    }
//    char buf[10];
//    while(1)
//    {
//        // -- AIR QUALITY
//        oled_clear_page(OLED_PAGE_AIRQ);
//        u16_to_str(buf, g_WeatherData.airQuality);
//        oled_put_str(0, OLED_PAGE_AIRQ, "AQ:");
//        oled_put_str(50, OLED_PAGE_AIRQ, buf);
//        UARTprintf("AirQ: %u\t", g_WeatherData.airQuality);
//
//
//        // -- WIND SPEED AND DIRECTION
//        if (g_WeatherData.windSpeed_mph == 0)
//        {
//            oled_clear_page(OLED_PAGE_WIND);
//            oled_clear_page(OLED_PAGE_DIR);
//            oled_put_str(0, OLED_PAGE_WIND, "WS: 0 mph");
//            oled_put_str(0, OLED_PAGE_DIR, "WD: NoWind");
//            UARTprintf("Wind Speed: 0 mph\t Wind Direction: NoWind\t");
//        }
//        else
//        {
//            oled_clear_page(OLED_PAGE_WIND);
//            u16_to_str(buf, g_WeatherData.windSpeed_mph / 1000);
//            oled_put_str(0, OLED_PAGE_WIND, "WS:");
//            oled_put_str(50, OLED_PAGE_WIND, buf);
//
//            oled_clear_page(OLED_PAGE_DIR);
//            oled_put_str(0, OLED_PAGE_DIR, "WD:");
//            oled_put_str(50, OLED_PAGE_DIR, getWindDirection(g_WeatherData.windDirection));
//            UARTprintf("Wind Speed: %u mph\t Wind Direction: %s\t",
//                        g_WeatherData.windSpeed_mph / 1000,
//                        getWindDirection(g_WeatherData.windDirection));
//        }
//        // -- TEMPERATURE
//        if (g_WeatherData.temperature == 0 || g_WeatherData.humidity == 0) {
//            oled_clear_page(OLED_PAGE_TEMP);
//            oled_put_str(0, OLED_PAGE_TEMP, "DHT Err");
//            UARTprintf("DHT Read Error\r\t");
//        }
//        else {
//            oled_clear_page(OLED_PAGE_TEMP);
//            u16_to_str(buf, g_WeatherData.temperature);
//            oled_put_str(0, OLED_PAGE_TEMP, "Tmp:");
//            oled_put_str(50, OLED_PAGE_TEMP, buf);
//
//            // -- HUMIDITY
//            oled_clear_page(OLED_PAGE_HUM);
//            u16_to_str(buf, g_WeatherData.humidity);
//            oled_put_str(0, OLED_PAGE_HUM, "Hum:");
//            oled_put_str(50, OLED_PAGE_HUM, buf);
//
//            UARTprintf("Temp: %d C\t Hum: %d %%\n", g_WeatherData.temperature, g_WeatherData.humidity);
//        }
//
//        OS_Sleep();
//    }
}

/*******************************************************************
*                               main()
*********************************************************************/
int main(void)
{
    System_Init();
    CycleCounter_Init();
    UART0_Init();
    UART5_Init();
    ADC0_Init();
    DHT_Init();
    SysCtlDelay(SysCtlClockGet());

    UARTprintf("Weather Station Started\r\n");

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);            // Enable GPIOF to test LED Task
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
    cs_high(); //chip select for oled high

    // SSI0 pins: PA2 CLK, PA5 TX(MOSI)
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_5);

    // Bring-up settings: slow + mode 0
    SSIConfigSetExpClk(OLED_SSI_BASE, SysCtlClockGet(),
                       SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER,
                       400000, 8);      // 100000 -> 400000
    SSIEnable(OLED_SSI_BASE);

    oled_init();
    mq135_adc_init_pe1_ain2(); //initialize the input pin for the sensor

    LED_Init();
    ADC_CalibrateWindSpeedFloor();
    OS_Init();

//    LED_SetLED_Direction(0);
//    LED_SetLED_Direction(1);
//    LED_SetLED_Direction(2);
//    LED_SetLED_Direction(3);
//    LED_SetLED_Direction(4);
//    LED_SetLED_Direction(5);
//    LED_SetLED_Direction(6);
//    LED_SetLED_Direction(7);
//    LED_SetLED_Direction(8);
//    LED_SetLED_Direction(9);

    OS_AddTask(Task_DisplayUpdate, 900);                 // 1, 900
    OS_AddTask(Task_TemperatureAndHumidity, 2000);       // 2, 2000
    OS_AddTask(Task_WindSpeed, 150);                    // 3, 1000
    OS_AddTask(Task_WindDirection, 110);                // 4, 1100
    OS_AddTask(Task_AirQuality, 100);                   // 5, 1200

    SysTick_Init();             // Configure for 1ms tick
    IntMasterEnable();
    OS_Scheduler();
    OS_Launch();
    while(1) {}
}


