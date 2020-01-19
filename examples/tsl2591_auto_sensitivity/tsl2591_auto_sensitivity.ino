// NOLINT(legal/copyright)
/******************************************************************************

    example for usage of slight_TSL2591AutoSensitivity
    auto-gain & auto Intergrationtime
    implementation for TSL2591 Digital Light Sensor


    libraries used:
        ~ Adafruit_TSL2591
            written by KT0WN (adafruit.com),
            Copyright (c) 2014 Adafruit Industries
            license: BSD
        ~ slight_TSL2591AutoSensitivity
            written by stefan krueger (s-light),
            git@s-light.eu, http://s-light.eu, https://github.com/s-light/
            Copyright (c) 2020 Stefan Krüger
            license: MIT

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

#include "slight_TSL2591AutoSensitivity.h"

// Ambient Light Sensor
// TSL2591
// hw connection
//      SCL → I2C Clock
//      SDA → I2C Data
//      Vin → 3.3-5V DC
//      GROUND → common ground
slight_TSL2591AutoSensitivity als = slight_TSL2591AutoSensitivity();

// enable float for printf
// https://github.com/arduino/ArduinoCore-samd/issues/217
// asm(".global _printf_float");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// debug out
uint32_t timeStamp_debugout = millis();

void debugout(Print &out) {
    while((millis() - timeStamp_debugout) > 270) {
        char buffer[] = "[1234567890ms]   \0";
        snprintf(
            buffer, sizeof(buffer),
            "[%8lums] ", millis());
        out.print(buffer);

        als.print_status(out);
        out.println();

        timeStamp_debugout = millis();
    }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// setup
void setup(void) {
    // Waits for the serial port to connect before sending data out
    // wait for arduino IDE to release all serial ports after upload.
    delay(1000);
    // initialise serial
    Serial.begin(115200);
    // Wait for Serial Connection to be Opend from Host
    // or timeout after 2seconds
    uint32_t timeStamp_Start = millis();
    while( (! Serial) && ( (millis() - timeStamp_Start) < 2000 ) ) {
        // nothing to do
    }

    Serial.println();
    Serial.println();
    Serial.println("******************************************");
    Serial.println("tsl2591_auto_sensitivity.ino");
    Serial.println("******************************************");
    Serial.println();

    if (als.begin(Serial)) {
        Serial.println("Ambient Light Sensor started.");
    } else {
        Serial.println("No sensor found ... check your wiring?");
    }
}

void loop(void) {
    als.update();
    debugout(Serial);
}
