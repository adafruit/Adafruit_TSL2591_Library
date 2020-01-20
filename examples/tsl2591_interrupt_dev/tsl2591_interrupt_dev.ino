/* TSL2591 Digital Light Sensor, example with (simple) interrupt support  */
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */

/*  This example shows how the interrupt system on the TLS2591
*  can be used to detect a meaningful change in light levels.
*
*  Two thresholds can be set:
*
*  Lower Threshold - Any light sample on CHAN0 below this value
*                    will trigger an interrupt
*  Upper Threshold - Any light sample on CHAN0 above this value
*                    will trigger an interrupt
*
*  If CHAN0 (full light) crosses below the low threshold specified,
*  or above the higher threshold, an interrupt is asserted on the interrupt
*  pin. The use of the HW pin is optional, though, since the change can
*  also be detected in software by looking at the status byte via
*  tsl.getStatus().
*
*  An optional third parameter can be used in the .setALSInterruptThresholds
*  function to indicate the number of samples that must stay outside
*  the threshold window before the interrupt fires, providing some basic
*  debouncing of light level data.
*
*  For example, the following code will fire an interrupt on any and every
*  sample outside the window threshold (meaning a sample below 100 or above
*  2000 on CHAN0 or FULL light):
*
*    tsl.setALSInterruptThresholds(100, 2000, TSL2591_PERSIST_ANY);
*
*  This code would require five consecutive changes before the interrupt
*  fires though (see tls2591Persist_t in Adafruit_TLS2591.h for possible
*  values):
*
*    tsl.setALSInterruptThresholds(100, 2000, TSL2591_PERSIST_5);
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

// Example for demonstrating the TSL2591 library - public domain!

// connect SCL to I2C Clock
// connect SDA to I2C Data
// connect Vin to 3.3-5V DC
// connect GROUND to common ground

Adafruit_TSL2591 tsl = Adafruit_TSL2591();

/**************************************************************************/
/*
Displays some basic information on this sensor from the unified
sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void) {
    sensor_t sensor;
    tsl.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value);
        Serial.println(" lux");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value, 4);
        Serial.println(" lux");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution, 4);
        Serial.println(" lux");
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}

/**************************************************************************/
/*
Configures the gain and integration time for the TSL2591
*/
/**************************************************************************/
void configureSensor(void) {
    // You can change the gain on the fly, to adapt
    // to brighter/dimmer light situations
    tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
    // tsl.setGain(TSL2591_GAIN_MED);    // 25x gain
    // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
    // tsl.setGain(TSL2591_GAIN_MAX);    // 9876x gain

    // Changing the integration time gives you a longer time over which to sense light
    // longer timelines are slower, but are good in very low light situtations!
    tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
    // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

    // Display the gain and integration time for reference sake
    Serial.println("------------------------------------");
    Serial.print  ("Gain:          ");
    tsl.printGain(Serial);
    Serial.println();
    Serial.print  ("Timing:        ");
    Serial.print(tsl.getTimingInMS());
    Serial.println(" ms");
    Serial.print  ("Max ADC Counts: ");
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
    // const uint16_t NPINTR_threshold_upper = tsl.getMaxADCCounts() - 1;

    // this combination would be helpfull to:
    // AINT: a new value is available
    // NPINTR: nearly out of range → check gain and integrationtime
    const uint16_t AINT_threshold_lower = 0;
    const uint16_t AINT_threshold_upper = 0;
    const tsl2591Persist_t AINT_persistance = TSL2591_PERSIST_EVERY;
    const uint16_t NPINTR_threshold_lower = 200;
    const uint16_t NPINTR_threshold_upper = tsl.getMaxADCCounts() - 200;

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


/**************************************************************************/
/*
Program entry point for the Arduino sketch
*/
/**************************************************************************/
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

    Serial.println("tsl2591_interrupt_dev.ino");

    if (tsl.begin()) {
        Serial.println("Found a TSL2591 sensor");
        // enable sensor continuously
        // otherwise the interrupt will not work.
        tsl.enable();
    } else {
        Serial.println("No sensor found ... check your wiring?");
        while (1) {}
    }

    // Display some basic information on this sensor
    displaySensorDetails();

    // Configure the sensor (including the interrupt threshold)
    configureSensor();

    // Now we're ready to get readings ... move on to loop()!
}

/**************************************************************************/
/*
Show how to read IR and Full Spectrum at once and convert to lux
*/
/**************************************************************************/
uint32_t last_action = 0;
void printStatus(void) {
    uint32_t duration = millis() - last_action;
    last_action = millis();
    Serial.print(F("[ "));
    Serial.print(millis());
    Serial.print(F(" ms ]"));
    Serial.print(F(" ("));
    Serial.print(duration);
    Serial.print(F(" ms) "));

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

    Serial.print("IR: "); Serial.print(ir);  Serial.print("  ");
    Serial.print("Full: "); Serial.print(full); Serial.print("  ");
    Serial.print("Visible: "); Serial.print(full - ir); Serial.print("  ");
    Serial.print("Lux: "); Serial.print(tsl.calculateLux(full, ir), 4);
    // Serial.print("  ");
    Serial.println();

    tsl.clearInterrupt();
}


/**************************************************************************/
/*
Arduino loop function, called once 'setup' is complete (your own code
should go here)
*/
/**************************************************************************/
void loop(void) {
    printStatus();
    delay(90);
}
