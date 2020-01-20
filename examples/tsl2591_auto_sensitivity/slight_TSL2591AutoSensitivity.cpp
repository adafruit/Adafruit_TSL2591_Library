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

bool slight_TSL2591AutoSensitivity::begin(Print &out) {
    // clean up..
    end();
    // start up...
    if (ready == false) {
        // setup
        if (tsl.begin()) {
            // tsl.printConfig(out, & sens_conf[0]);
            // tsl.printConfig(out, & sens_conf[3]);
            set_sensitivity_config(sens_conf_current_id, out);
            tsl.enable();
            // enable
            ready = true;
        } else {
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
            print_status(Serial);
            Serial.println();
            tsl.clearInterrupt();
            read_sensor();
            if (x & TSL2591_STATUS_NPINTR) {
                uint32_t duration = millis() - sens_conf_changed_timestamp;
                if (duration > sens_conf_changed_extra_wait_duration) {
                    handle_out_of_range();
                }
            } else {
                update_filter();
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
    raw_ir = lum >> 16;
    raw_full = lum & 0xFFFF;
}

void slight_TSL2591AutoSensitivity::handle_out_of_range(void) {
    uint8_t config_id_new =  sens_conf_current_id;
    uint16_t config_lower =  sens_conf_current->NPINTR_threshold_lower;
    uint16_t config_upper =  sens_conf_current->NPINTR_threshold_upper;
    int8_t changed = 0;
    if (raw_full < config_lower) {
        // switch to higher id = more sensitiv
        if (config_id_new <  sens_conf_count-1) {
            config_id_new += 1;
            changed = 1;
        }
    } else {
        if (raw_full > config_upper) {
            // switch to lower id = less sensitiv
            if (config_id_new > 0) {
                config_id_new -= 1;
                changed = -1;
            }
        }
    }
    if (sens_conf_current_id != config_id_new) {
        // new configuration!!
        sens_conf_current_id = config_id_new;
        set_sensitivity_config(sens_conf_current_id);
        // set flag
        sens_conf_changed += changed;
        sens_conf_changed_timestamp = millis();
        sens_conf_changed_extra_wait_duration =
            tsl.getTimingInMS(sens_conf_current->integrationtime) * 5;
        // Serial.println("***");
        // Serial.print("  sens_conf_changed:");
        // Serial.print(sens_conf_changed);
        // Serial.print(";  sens_conf_current_id:");
        // Serial.print(sens_conf_current_id);
        // Serial.println();
        // Serial.println("***");
    }
}

void slight_TSL2591AutoSensitivity::update_filter(void) {
    double lux = tsl.calculateLux(raw_full, raw_ir);
    // meridian filter
    value_filter[value_filter_index] = lux;
    value_filter_index += 1;
    if (value_filter_index > value_filter_count) {
        value_filter_index = 0;
    }
    double temp_sum = 0;
    for (size_t i = 0; i < value_filter_count; i++) {
        temp_sum += value_filter[i];
    }
    value_lux = temp_sum / value_filter_count;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// config

void slight_TSL2591AutoSensitivity::set_sensitivity_config(uint8_t config_id) {
     sens_conf_current_id = config_id;
     sens_conf_current =
        & sens_conf[ sens_conf_current_id];
    tsl.setConfig(sens_conf_current);
}

void slight_TSL2591AutoSensitivity::set_sensitivity_config(
    uint8_t config_id, Print &out
) {
    // out.println("current config:");
    // tsl.printConfig(out);
    out.print("set config '");
    out.print(config_id);
    out.println("' ..");
    set_sensitivity_config(config_id);
    out.println("current config:");
    tsl.printConfig(out);
}

uint8_t slight_TSL2591AutoSensitivity::get_sensitivity_config_id() {
    return sens_conf_current_id;
}

uint8_t slight_TSL2591AutoSensitivity::get_sensitivity_config_changed() {
    return sens_conf_changed;
}

void slight_TSL2591AutoSensitivity::reset_sensitivity_config_changed() {
    sens_conf_changed = 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// custom tsl functions


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// helper

void slight_TSL2591AutoSensitivity::sensor_print_details(Print &out) {
    sensor_t sensor;
    tsl.getSensor(&sensor);
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
}

void slight_TSL2591AutoSensitivity::print_status(Print &out) {
    uint8_t x = tsl.getStatus();
    // Serial.print("status: '");
    // Serial.print(x, HEX);
    // Serial.print("  ");
    // Serial.print("'");
    // // for (size_t i = 0; i < 8; i++) {
    // //     if (bitRead(x, 7-i)) {
    // for (size_t i = 8; i > 0; i--) {
    //     if (bitRead(x, i-1)) {
    //         Serial.print("1");
    //     } else {
    //         Serial.print("0");
    //     }
    // }
    // Serial.print("' ");

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
    #if defined(ARDUINO_ARCH_AVR)
        // https://stackoverflow.com/a/27652012/574981
        int chars_written = snprintf(
            buffer, sizeof(buffer),
            "IR: %5u  Full: %5u  Visible: %5u  Lux: ",
            ir,
            full,
            (full-ir));
        if (chars_written > 0) {
            dtostrf(tsl.calculateLux(full, ir), 5, 4, buffer+chars_written);
        }
    #elif defined(ARDUINO_ARCH_SAMD)
        // enable float for printf
        // https://github.com/arduino/ArduinoCore-samd/issues/217
        asm(".global _printf_float");
        snprintf(
            buffer, sizeof(buffer),
            "IR: %5u  Full: %5u  "
            // "Visible: %5u  "
            "Lux: %5.4f",
            ir,
            full,
            // (full-ir),
            tsl.calculateLux(full, ir));
    #else
        #error “currently this lib supports only AVR or SAMD.”
    #endif
    out.print(buffer);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
