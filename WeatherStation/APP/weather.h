#ifndef APP_WEATHER_H_
#define APP_WEATHER_H_

#include <stdint.h>
#include <string.h>
typedef enum {
    NORTH,              // 0    0° - 11.2°
    NORTH_NORTHEAST,    // 1    11.3° - 33.7°
    NORTHEAST,          // 2    33.8° - 56.2°
    EAST_NORTHEAST,     // 3    56.3° - 78.7°
    EAST,               // 4    78.8° - 101.2°
    EAST_SOUTHEAST,     // 5    101.3° - 123.7°
    SOUTHEAST,          // 6    123.8° - 146.2°
    SOUTH_SOUTHEAST,    // 7    146.3° - 168.7°
    SOUTH,              // 8    168.8° - 191.2°
    SOUTH_SOUTHWEST,    // 9    191.3° - 213.7°
    SOUTHWEST,          // 10   213.8° - 236.2°
    WEST_SOUTHWEST,     // 11   236.3° - 258.7°
    WEST,               // 12   258.8° - 281.2°
    WEST_NORTHWEST,     // 13   281.3° - 303.7°
    NORTHWEST,          // 14   303.8° - 326.2°
    NORTH_NORTHWEST,    // 15   326.3° - 348.7°
    NORTH_2,               // 16   348.8° - 360°
    NO_WIND
} WindDirection;

typedef struct {
    uint16_t windSpeed_mph;
    WindDirection windDirection;
    uint8_t temperature;
    uint8_t humidity;
    uint8_t airQuality;
} WeatherData_t;

char* getWindDirection(uint16_t direction);

#endif /* APP_WEATHER_H_ */
