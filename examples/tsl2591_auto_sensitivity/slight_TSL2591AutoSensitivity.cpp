// NOLINT(legal/copyright)
/******************************************************************************

    auto-gain implementation for TSL2591 Digital Light Sensor

    libraries used:
        ~ Adafruit_TSL2591
            written by KT0WN (adafruit.com),
            Copyright (c) 2014 Adafruit Industries
            license: BSD

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


// include own headerfile
// NOLINTNEXTLINE(build/include)
#include "./slight_TSL2591AutoSensitivity.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main api

slight_TSL2591AutoSensitivity::slight_TSL2591AutoSensitivity() {
    ready = false;
}

slight_TSL2591AutoSensitivity::~slight_TSL2591AutoSensitivity() {
    end();
}

bool slight_TSL2591AutoSensitivity::begin(Stream &out) {
    // clean up..
    end();
    // start up...
    if (ready == false) {
        // setup
        if (tsl.begin()) {
            out.println(F("found TSL2591 sensor"));

            configure_sensor(out);

            tsl_print_details(out);

            tsl.enable();

            // enable
            ready = true;
        } else {
            out.println(F("no sensor found ... check your wiring?"));
            ready = false;
        }
    }
    return ready;
}

void slight_TSL2591AutoSensitivity::end() {
    if (ready) {
        // nothing to do..
    }
}

void slight_TSL2591AutoSensitivity::update() {
    if (ready) {
        // do it :-)
        // check flags
        uint8_t x = tsl.getStatus();
        // bit 0: AVALID = ALS Valid
        // bit 4: AINT = ALS Interrupt occured
        // bit 5: NPINTR = No-persist Interrupt occurence
        if (
            (x & TSL2591_STATUS_AVALID)
            && (x & TSL2591_STATUS_AINT)
        ) {
            // valid reading
            if (x & TSL2591_STATUS_NPINTR) {
                // out of bounds
            } else {
                // read
                read_sensor();
                // if (value_lux > 300) {
                //     /* code */
                // }
            }
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// handling

void slight_TSL2591AutoSensitivity::read_sensor(void) {
    // More advanced data read example.
    // Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
    // That way you can do whatever math and comparisons you want!
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    value_filter[value_filter_index] = tsl.calculateLux(full, ir);
    value_filter_index += 1;
    if (value_filter_index > value_filter_count) {
        value_filter_index = 0;
    }
    double temp_sum = 0;
    for (size_t i = 0; i < value_filter_count; i++) {
        temp_sum += value_filter[i];
    }
    value_lux = temp_sum / value_filter_count;

    tsl.clearInterrupt();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// config

void slight_TSL2591AutoSensitivity::configure_sensor(Print &out) {
    // You can change the gain on the fly,
    // to adapt to brighter/dimmer light situations
    // tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
    tsl.setGain(TSL2591_GAIN_MED);    // 25x gain
    // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
    // tsl.setGain(TSL2591_GAIN_MAX);    // 9876x gain

    // Changing the integration time gives
    // you a longer time over which to sense light
    // longer timelines are slower, but are good in very low light situtations!
    // shortest integration time (bright light)
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
    // longest integration time (dim light)

    // Display the gain and integration time for reference sake
    Serial.println("------------------------------------");
    Serial.print("Gain:         ");
    tsl.printGain(Serial);
    Serial.println();
    Serial.print("Timing:       ");
    Serial.print(tsl.getTimingInMS());
    Serial.println(" ms");
    Serial.print("Max ADC Counts: ");
    Serial.print(tsl.getMaxADCCounts());
    Serial.println();
    Serial.println("------------------------------------");
    Serial.println("");


    // AINT persistance
    // TSL2591_PERSIST_EVERY → Every ALS cycle generates an interrupt
    // TSL2591_PERSIST_ANY → Fire on Any value outside of threshold range
    // TSL2591_PERSIST_2 → Require at least 2 samples outside of range to fire
    // TSL2591_PERSIST_3 → Require at least 3 samples outside of range to fire
    // TSL2591_PERSIST_5 → Require at least 5 samples outside of range to fire
    // TSL2591_PERSIST_10 → Require at least 10 samples outside of range to fire
    // TSL2591_PERSIST_15 → Require at least 15 samples outside of range to fire
    // TSL2591_PERSIST_nn → in steps of 5
    // TSL2591_PERSIST_60 → Require at least 60 samples outside of range to fire

    // this combination would be helpfull to:
    // AINT: set a custom range with some smoothing
    // NPINTR: out of range → check gain and integrationtime
    // const uint16_t AINT_threshold_lower = 50;
    // const uint16_t AINT_threshold_upper = 3000;
    // const tsl2591Persist_t AINT_persistance = TSL2591_PERSIST_20;
    // const uint16_t NPINTR_threshold_lower = 1;
    // const uint16_t NPINTR_threshold_upper = 0xFFFF-1;

    // this combination would be helpfull to:
    // AINT: a new value is available
    // NPINTR: nearly out of range → check gain and integrationtime
    const uint16_t AINT_threshold_lower = 0;
    const uint16_t AINT_threshold_upper = 0;
    const tsl2591Persist_t AINT_persistance = TSL2591_PERSIST_EVERY;
    const uint16_t NPINTR_threshold_lower = 100;
    const uint16_t NPINTR_threshold_upper =  tsl.getMaxADCCounts() - 200;;

    tsl.clearInterrupt();
    tsl.setALSInterruptThresholds(
        AINT_threshold_lower, AINT_threshold_upper, AINT_persistance);
    tsl.setNPInterruptThresholds(
        NPINTR_threshold_lower, NPINTR_threshold_upper);
    tsl.clearInterrupt();

    /* Display the interrupt threshold window */
    Serial.print("AINT Threshold Window: ");
    Serial.print(AINT_threshold_lower, DEC);
    Serial.print(" to ");
    Serial.print(AINT_threshold_upper, DEC);
    Serial.print(" with persist ");
    tsl.printPersistance(Serial, AINT_persistance);
    Serial.println();
    Serial.print("NPINTR Threshold Window: ");
    Serial.print(NPINTR_threshold_lower, DEC);
    Serial.print(" to ");
    Serial.print(NPINTR_threshold_upper, DEC);
    Serial.println();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// custom tsl functions


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// helper

void slight_TSL2591AutoSensitivity::tsl_print_details(Print &out) {
    sensor_t sensor;
    tsl.getSensor(&sensor);
    out.println(F("------------------------------------"));
    out.print(F("Sensor:     "));
    out.println(sensor.name);
    out.print(F("Driver Ver: "));
    out.println(sensor.version);
    out.print(F("Unique ID:  "));
    out.println(sensor.sensor_id);
    out.print(F("Max Value:  "));
    out.print(sensor.max_value);
    out.println(F(" lux"));
    out.print(F("Min Value:  "));
    out.print(sensor.min_value, 4);
    out.println(F(" lux"));
    out.print(F("Resolution: "));
    out.print(sensor.resolution, 4);
    out.println(F(" lux"));
    out.println(F("------------------------------------"));
    out.print(F("Gain:         "));
    tsl.printGain(out);
    out.println();
    out.print(F("Timing:       "));
    out.print(tsl.getTimingInMS());
    out.println(" ms");
    out.print("Max ADC Counts: ");
    out.print(tsl.getMaxADCCounts());
    out.println();
    out.println(F("------------------------------------"));
    out.println(F(""));
}

void slight_TSL2591AutoSensitivity::print_status(Print &out) {
    // uint32_t duration = millis() - last_action;
    // last_action = millis();
    // Serial.print(F("[ "));
    // Serial.print(millis());
    // Serial.print(F(" ms ]"));
    // Serial.print(F(" ("));
    // Serial.print(duration);
    // Serial.print(F(" ms) "));

    uint8_t x = tsl.getStatus();

    // Serial.print("status: '");
    // Serial.print(x, HEX);
    // Serial.print("  ");
    Serial.print("'");
    // for (size_t i = 0; i < 8; i++) {
    //     if (bitRead(x, 7-i)) {
    for (size_t i = 8; i > 0; i--) {
        if (bitRead(x, i-1)) {
            Serial.print("1");
        } else {
            Serial.print("0");
        }
    }
    Serial.print("' ");

    // print flags
    // bit 0: AVALID = ALS Valid
    // bit 4: AINT = ALS Interrupt occured
    // bit 5: NPINTR = No-persist Interrupt occurence
    if (x & TSL2591_STATUS_AVALID) {
        Serial.print("AVALID");
    } else {
        Serial.print(".     ");
    }
    Serial.print(" ");
    if (x & TSL2591_STATUS_AINT) {
        Serial.print("AINT");
    } else {
        Serial.print(".   ");
    }
    Serial.print(" ");
    if (x & TSL2591_STATUS_NPINTR) {
        Serial.print("NPINTR");
    } else {
        Serial.print(".     ");
    }
    Serial.print("  ");

    // More advanced data read example.
    // Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
    // That way you can do whatever math and comparisons you want!
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;

    char buffer[] =
        "IR: 65535  Full: 65535  Visible: 65535  Lux: 88000.0000     \0";

    // #if defined(ARDUINO_ARCH_AVR)
    //     int chars_written = snprintf(
    //         buffer, sizeof(buffer),
    //         "IR: %5u  Full: %5u  Visible: %5u  Lux: ",
    //         ir,
    //         full,
    //         (full-ir));
    //     if (chars_written > 0) {
    //         dtostrf(tsl.calculateLux(full, ir), 5, 4, buffer+chars_written);
    //     }
    // #elif defined(ARDUINO_ARCH_SAMD)
    //     // enable float for printf
    //     // https://github.com/arduino/ArduinoCore-samd/issues/217
    //     asm(".global _printf_float");
    //     snprintf(
    //         buffer, sizeof(buffer),
    //         "IR: %5u  Full: %5u  Visible: %5u  Lux: %5.4f",
    //         ir,
    //         full,
    //         (full-ir),
    //         tsl.calculateLux(full, ir));
    // #else
    //     #error “currently this lib supports only AVR or SAMD.”
    // #endif

    // enable float for printf
    // https://github.com/arduino/ArduinoCore-samd/issues/217
    asm(".global _printf_float");
    snprintf(
        buffer, sizeof(buffer),
        "IR: %5u  Full: %5u  Visible: %5u  Lux: %5.4f",
        ir,
        full,
        (full-ir),
        tsl.calculateLux(full, ir));
    out.print(buffer);

    tsl.clearInterrupt();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
