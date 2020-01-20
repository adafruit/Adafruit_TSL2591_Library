/******************************************************************************

    auto-gain implementation for TSL2591 Digital Light Sensor

    written by stefan krueger (s-light),
        git@s-light.eu, http://s-light.eu, https://github.com/s-light/

******************************************************************************/
/******************************************************************************
The MIT License (MIT)

Copyright (c) 2020 Stefan Krüger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/



#ifndef slight_tsl2591_autosensitivity_H_
#define slight_tsl2591_autosensitivity_H_

// include Core Arduino functionality
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

class slight_TSL2591AutoSensitivity {
public:

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // constructor
    slight_TSL2591AutoSensitivity();
    ~slight_TSL2591AutoSensitivity();

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // basic library api
    bool begin(Stream &out);
    void update();
    void end();

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // objects
    Adafruit_TSL2591 tsl = Adafruit_TSL2591(42);

    // config
    void configure_sensor(Print &out);

    // handling
    void read_sensor(void);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // sensitivity config
    // struct sensitivity_config_t {
    //     tsl2591Gain_t gain;
    //     tsl2591IntegrationTime_t integrationtime;
    //     uint16_t AINT_threshold_lower;
    //     uint16_t AINT_threshold_upper;
    //     tsl2591Persist_t AINT_persistance;
    //     uint16_t NPINTR_threshold_lower;
    //     uint16_t NPINTR_threshold_upper;
    // };

    static const uint8_t sensitivity_configs_count = 12;
        // sizeof(sensitivity_configs) / sizeof(sensitivity_config_t);
    Adafruit_TSL2591::tsl2591Config_t sensitivity_configs[sensitivity_configs_count] = {
        // 0 bright sun
        {
            TSL2591_GAIN_LOW,
            TSL2591_INTEGRATIONTIME_100MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            1000,
            TSL2591_MAX_ADC_COUNT_100MS
        },
        // 1 dailight
        {
            TSL2591_GAIN_MED,
            TSL2591_INTEGRATIONTIME_100MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_100MS-1000
        },
        // 2 cloudy
        {
            TSL2591_GAIN_MED,
            TSL2591_INTEGRATIONTIME_200MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        },
        // 3
        {
            TSL2591_GAIN_HIGH,
            TSL2591_INTEGRATIONTIME_100MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_100MS-1000
        },
        // 4
        {
            TSL2591_GAIN_HIGH,
            TSL2591_INTEGRATIONTIME_200MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        },
        // 5
        {
            TSL2591_GAIN_HIGH,
            TSL2591_INTEGRATIONTIME_300MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        },
        // 6
        {
            TSL2591_GAIN_MAX,
            TSL2591_INTEGRATIONTIME_100MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_100MS-1000
        },
        // 7
        {
            TSL2591_GAIN_MAX,
            TSL2591_INTEGRATIONTIME_200MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        },
        // 8
        {
            TSL2591_GAIN_MAX,
            TSL2591_INTEGRATIONTIME_300MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        },
        // 9
        {
            TSL2591_GAIN_MAX,
            TSL2591_INTEGRATIONTIME_400MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        },
        // 10
        {
            TSL2591_GAIN_MAX,
            TSL2591_INTEGRATIONTIME_500MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            100,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        },
        // 11
        {
            TSL2591_GAIN_MAX,
            TSL2591_INTEGRATIONTIME_600MS,
            0,
            0,
            TSL2591_PERSIST_EVERY,
            0,
            TSL2591_MAX_ADC_COUNT_200MS_600MS-1000
        }
    };


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // helper
    void sensor_print_details(Print &out);
    void print_status(Print &out);
    // uint32_t last_action = 0;

    double value_lux = 0.0;
private:
    bool ready;

    static const uint8_t value_filter_count = 10;
    double value_filter[value_filter_count];
    uint8_t value_filter_index = 0;

};  // class slight_TSL2591AutoSensitivity

#endif  // slight_tsl2591_autosensitivity_H_
