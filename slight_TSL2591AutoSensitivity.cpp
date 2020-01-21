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

/// @cond DEV

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
            // print_status(Serial);
            // Serial.println();
            tsl.clearInterrupt();
            read_sensor();
            if (x & TSL2591_STATUS_NPINTR) {
                uint32_t duration = millis() - sens_conf_changed_timestamp;
                if (duration > sens_conf_changed_extra_wait_duration) {
                    handle_out_of_range();
                }
            } else {
                update_filter();
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
    ir_raw = lum >> 16;
    full_raw = lum & 0xFFFF;
    lux_raw = tsl.calculateLux(full_raw, ir_raw);
}

void slight_TSL2591AutoSensitivity::handle_out_of_range(void) {
    uint8_t config_id_new =  sens_conf_current_id;
    uint16_t config_lower =  sens_conf_current->NPINTR_threshold_lower;
    uint16_t config_upper =  sens_conf_current->NPINTR_threshold_upper;
    int8_t changed = 0;
    if (full_raw < config_lower) {
        // switch to higher id = more sensitiv
        if (config_id_new <  sens_conf_count-1) {
            config_id_new += 1;
            changed = 1;
        }
    } else {
        if (full_raw > config_upper) {
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
        // if (changed > 0) {
        //     sens_conf_changed_extra_wait_duration =
        //         tsl.getTimingInMS(sens_conf_current->integrationtime) * 5;
        // } else {
        //     sens_conf_changed_extra_wait_duration = 0;
        // }
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
    // simple filter
    lux_filter[lux_filter_index] = lux_raw;
    lux_filter_index += 1;
    if (lux_filter_index > lux_filter_count) {
        lux_filter_index = 0;
    }
    double temp_sum = 0;
    for (size_t i = 0; i < lux_filter_count; i++) {
        temp_sum += lux_filter[i];
    }
    double lux_filtered_new = temp_sum / lux_filter_count;
    if (lux_filtered != lux_filtered_new) {
        lux_filtered = lux_filtered_new;
        // TODO(s-light): add precision filtering
        // TODO(s-light): add event generation
    }
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

uint8_t slight_TSL2591AutoSensitivity::sensitivity_config_changed() {
    return sens_conf_changed;
}

void slight_TSL2591AutoSensitivity::sensitivity_config_changed_clear() {
    sens_conf_changed = 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// value getter

uint16_t slight_TSL2591AutoSensitivity::get_ir_raw(void) {
    return ir_raw;
}

uint16_t slight_TSL2591AutoSensitivity::get_full_raw(void) {
    return full_raw;
}

double slight_TSL2591AutoSensitivity::get_lux_raw(void) {
    return lux_raw;
}

double slight_TSL2591AutoSensitivity::get_lux_filtered(void) {
    return lux_filtered;
}

bool slight_TSL2591AutoSensitivity::lux_filtered_changed(void) {
    return lux_filtered_changed;
}

void slight_TSL2591AutoSensitivity::lux_filtered_changed_clear(void) {
    lux_filtered_changed = false;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// helper

size_t slight_TSL2591AutoSensitivity::print_float(
    Print &out, float value, size_t leading, size_t precision
) {
    // const size_t buffer_length = leading + 1 + precision;
    char buffer[20];
    size_t chars_printed = 0;
    #if ( \
        defined(ARDUINO_ARCH_SAMD) \
        || defined(ARDUINO_ARCH_ESP32) \
        || defined(ARDUINO_ARCH_ESP8266) \
    )
        // enable float for printf
        // https://github.com/arduino/ArduinoCore-samd/issues/217
        asm(".global _printf_float");
        snprintf(
            buffer, sizeof(buffer),
            "%*.*f",
            leading,
            precision,
            value);
        chars_printed = out.print(buffer);
    #else
        #if defined(ARDUINO_ARCH_AVR)
            // https://stackoverflow.com/a/27652012/574981
            dtostrf(value, leading, precision, buffer);
            chars_printed = out.print(buffer);
        #else
            // fallback to non aligned Serial.print()
            chars_printed = out.print(value, precision);
        #endif
    #endif
    return chars_printed;
}

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
    snprintf(
        buffer, sizeof(buffer),
        "IR: %5u  "
        "Full: %5u  "
        "Lux: ",
        ir,
        full);
    out.print(buffer);
    print_float(out, tsl.calculateLux(full, ir), 5, 4);
}


/// @endcond

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
