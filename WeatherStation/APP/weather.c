/*
 * weather.c
 *
 *  Created on: Mar 7, 2026
 *      Author: ynish
 */

#include "weather.h"

char* getWindDirection(uint16_t dir)
{
    switch (dir) {
        case NORTH:             return "N";
        case NORTHEAST:         return "NE";
        case EAST:              return "E";
        case SOUTHEAST:         return "SE";
        case SOUTH:             return "S";
        case SOUTHWEST:         return "SW";
        case WEST:              return "W";
        case NORTHWEST:         return "NW";
        case NORTH_2:           return "N";
        default:                return "NoWind";
    }
}


