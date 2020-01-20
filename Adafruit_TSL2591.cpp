/**************************************************************************/
/*!
    @file     Adafruit_TSL2591.cpp
    @author   KT0WN (adafruit.com)

    This is a library for the Adafruit TSL2591 breakout board
    This library works with the Adafruit TSL2591 breakout
    ----> https://www.adafruit.com/products/1980

    Check out the links above for our tutorials and wiring diagrams
    These chips use I2C to communicate

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2014 Adafruit Industries
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#if defined(ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#else
  #include <avr/pgmspace.h>
#endif
#if defined(__AVR__)
  #include <util/delay.h>
#endif
#include <stdlib.h>

#include "Adafruit_TSL2591.h"

/**************************************************************************/
/*!
    @brief  Instantiates a new Adafruit TSL2591 class
    @param  sensorID An optional ID # so you can track this sensor, it will tag sensorEvents you create.
*/
/**************************************************************************/
Adafruit_TSL2591::Adafruit_TSL2591(int32_t sensorID)
{
  _initialized = false;
  _enabled = false;
  // _integration = TSL2591_INTEGRATIONTIME_100MS;
  // _gain        = TSL2591_GAIN_MED;

  _config_local.gain = TSL2591_GAIN_MED;
  _config_local.integrationtime = TSL2591_INTEGRATIONTIME_100MS;
  _config_local.AINT_threshold_lower = 0;
  _config_local.AINT_threshold_upper = 0;
  _config_local.AINT_persistance = TSL2591_PERSIST_EVERY;
  _config_local.NPINTR_threshold_lower = 100;
  _config_local.NPINTR_threshold_upper = TSL2591_MAX_ADC_COUNT_100MS-100;

  _sensorID    = sensorID;

  // we can not do wire initialization now. we havent loaded Wire yet
}

/**************************************************************************/
/*!
    @brief  Setups the I2C interface and hardware, identifies if chip is found
    @param theWire a reference to TwoWire instance
    @returns True if a TSL2591 is found, false on any failure
*/
/**************************************************************************/
boolean Adafruit_TSL2591::begin(TwoWire *theWire)
{
_i2c=theWire;
  _i2c->begin();

  /*
  for (uint8_t i=0; i<0x20; i++)
  {
    uint8_t id = read8(0x12);
    Serial.print("$"); Serial.print(i, HEX);
    Serial.print(" = 0x"); Serial.println(read8(i), HEX);
  }
  */

  uint8_t id = read8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID);
  if (id != 0x50 ) {
    return false;
  }
  // Serial.println("Found Adafruit_TSL2591");

  _initialized = true;

  // Set default configuration
  setConfig(&_config_local);
  // integration time and gain
  // setTiming(_integration);
  // setGain(_gain);

  // Note: by default, the device is in power down mode on bootup
  disable();
  // you can enable it with
  // enable();

  return true;
}
/**************************************************************************/
/*!
    @brief  Setups the I2C interface and hardware, identifies if chip is found
    @returns True if a TSL2591 is found, false on any failure
*/
/**************************************************************************/
boolean Adafruit_TSL2591::begin()
{

	begin(&Wire);

  return true;
}

/**************************************************************************/
/*!
    @brief  Enables the chip, so it's ready to take readings
*/
/**************************************************************************/
void Adafruit_TSL2591::enable(void)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  // Enable the device by setting the control bit to 0x01
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE,
	 TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN | TSL2591_ENABLE_NPIEN);
  _enabled = true;
}


/**************************************************************************/
/*!
    @brief Disables the chip, so it's in power down mode
*/
/**************************************************************************/
void Adafruit_TSL2591::disable(void)
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  // Disable the device by setting the control bit to 0x00
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWEROFF);
  _enabled = false;
}

/************************************************************************/
/*!
    @brief  write gain & integrationtime to chip
*/
/**************************************************************************/
void Adafruit_TSL2591::_writeIntegrationtimeGain()
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  boolean was_enabled = _enabled;
  if (!_enabled) {
    enable();
  }
  write8(
      TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL,
      _config_local.integrationtime | _config_local.gain);
  if (!was_enabled) {
    disable();
  }
}

/************************************************************************/
/*!
    @brief  Setter for sensor light gain
    @param  gain {@link tsl2591Gain_t} gain value
*/
/**************************************************************************/
void Adafruit_TSL2591::setGain(tsl2591Gain_t gain)
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  _config_local.gain = gain;
  _writeIntegrationtimeGain();
}

/************************************************************************/
/*!
    @brief  Getter for sensor light gain
    @returns {@link tsl2591Gain_t} gain value
*/
/**************************************************************************/
tsl2591Gain_t Adafruit_TSL2591::getGain()
{
  return _config_local.gain;
}

/************************************************************************/
/*!
    @brief  get gain as integer (human readable)
    @param gain {@link tsl2591Gain_t} gain value
    @returns gain value as integer
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::gainAsInt(tsl2591Gain_t gain)
{
  uint16_t gain_as_int = 0;
  switch (gain) {
      case TSL2591_GAIN_LOW:
      gain_as_int = 1;
      break;
      case TSL2591_GAIN_MED:
      gain_as_int = 25;
      break;
      case TSL2591_GAIN_HIGH:
      gain_as_int = 428;
      break;
      case TSL2591_GAIN_MAX:
      gain_as_int = 9876;
      break;
  }
  return gain_as_int;
}

/************************************************************************/
/*!
    @brief  get current gain as integer (human readable)
    @returns gain value as integer
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::getGainAsInt()
{
  return gainAsInt(getGain());
}

/************************************************************************/
/*!
    @brief  Print gain value in Human readable form
    @param  out Print (=Serial) reference to use for printing
    @param  gain {@link tsl2591Gain_t} gain value
*/
/**************************************************************************/
void Adafruit_TSL2591::printGain(Print &out, tsl2591Gain_t gain)
{
    out.print(gainAsInt(gain));
    out.print(F("x ("));
    switch (gain) {
        case TSL2591_GAIN_LOW:
            out.print(F("Low"));
        break;
        case TSL2591_GAIN_MED:
            out.print(F("Medium"));
        break;
        case TSL2591_GAIN_HIGH:
            out.print(F("High"));
        break;
        case TSL2591_GAIN_MAX:
            out.print(F("Max"));
        break;
    }
    out.print(F(")"));
}

/************************************************************************/
/*!
    @brief  Print current gain value in Human readable form
    @param  out Print (=Serial) reference to use for printing
*/
/**************************************************************************/
void Adafruit_TSL2591::printGain(Print &out) {
    printGain(out, getGain());
}

/************************************************************************/
/*!
    @brief  Setter for sensor integration time setting
    @param integration {@link tsl2591IntegrationTime_t} integration time setting
*/
/**************************************************************************/
void Adafruit_TSL2591::setTiming(tsl2591IntegrationTime_t integration)
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  _config_local.integrationtime = integration;
  _writeIntegrationtimeGain();
}

/************************************************************************/
/*!
    @brief  Getter for sensor integration time setting
    @returns {@link tsl2591IntegrationTime_t} integration time
*/
/**************************************************************************/
tsl2591IntegrationTime_t Adafruit_TSL2591::getTiming()
{
  return _config_local.integrationtime;
}

/************************************************************************/
/*!
    @brief  get integration time in milliseconds
    @param {@link tsl2591IntegrationTime_t} integration time
    @returns integration time in milliseconds
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::getTimingInMS(tsl2591IntegrationTime_t integration)
{
  return ((integration + 1) * 100);
}

/************************************************************************/
/*!
    @brief  get sensor integration time in milliseconds
    @returns integration time in milliseconds
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::getTimingInMS()
{
  return getTimingInMS(getTiming());
}

/************************************************************************/
/*!
    @brief  get sensor max adc counts for given integration time
    @param {@link tsl2591IntegrationTime_t} integration time
    @returns max adc counts
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::getMaxADCCounts(tsl2591IntegrationTime_t integration)
{
  uint16_t result = 0;
  switch (integration) {
      case TSL2591_INTEGRATIONTIME_100MS: {
        result = TSL2591_MAX_ADC_COUNT_100MS;
      } break;
      case TSL2591_INTEGRATIONTIME_200MS:
      case TSL2591_INTEGRATIONTIME_300MS:
      case TSL2591_INTEGRATIONTIME_400MS:
      case TSL2591_INTEGRATIONTIME_500MS:
      case TSL2591_INTEGRATIONTIME_600MS: {
          result = TSL2591_MAX_ADC_COUNT_200MS_600MS;
      } break;
  }
  return result;
}

/************************************************************************/
/*!
    @brief  get sensor max adc counts for current integration time
    @returns max adc counts
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::getMaxADCCounts()
{
  return getMaxADCCounts(getTiming());
}

/************************************************************************/
/*!
    @brief  Calculates the visible Lux based on the two light sensors
    @param  ch0 Data from channel 0 (IR+Visible)
    @param  ch1 Data from channel 1 (IR)
    @returns Lux, based on AMS coefficients (or < 0 if overflow)
*/
/**************************************************************************/
float Adafruit_TSL2591::calculateLux(uint16_t ch0, uint16_t ch1)
{
  float    atime, again;
  float    cpl, lux1, lux2, lux;
  uint32_t chan0, chan1;

  // Check for overflow conditions first
  if ((ch0 == 0xFFFF) | (ch1 == 0xFFFF))
  {
    // Signal an overflow
    return -1;
  }

  // Note: This algorithm is based on preliminary coefficients
  // provided by AMS and may need to be updated in the future

  switch (_config_local.integrationtime)
  {
    case TSL2591_INTEGRATIONTIME_100MS :
      atime = 100.0F;
      break;
    case TSL2591_INTEGRATIONTIME_200MS :
      atime = 200.0F;
      break;
    case TSL2591_INTEGRATIONTIME_300MS :
      atime = 300.0F;
      break;
    case TSL2591_INTEGRATIONTIME_400MS :
      atime = 400.0F;
      break;
    case TSL2591_INTEGRATIONTIME_500MS :
      atime = 500.0F;
      break;
    case TSL2591_INTEGRATIONTIME_600MS :
      atime = 600.0F;
      break;
    default:  // 100ms
      atime = 100.0F;
      break;
  }

  switch (_config_local.gain)
  {
    case TSL2591_GAIN_LOW :
      again = 1.0F;
      break;
    case TSL2591_GAIN_MED :
      again = 25.0F;
      break;
    case TSL2591_GAIN_HIGH :
      again = 428.0F;
      break;
    case TSL2591_GAIN_MAX :
      again = 9876.0F;
      break;
    default:
      again = 1.0F;
      break;
  }

  // cpl = (ATIME * AGAIN) / DF
  cpl = (atime * again) / TSL2591_LUX_DF;

  // Original lux calculation (for reference sake)
  // lux1 = ( (float)ch0 - (TSL2591_LUX_COEFB * (float)ch1) ) / cpl;
  // lux2 = ( ( TSL2591_LUX_COEFC * (float)ch0 ) - ( TSL2591_LUX_COEFD * (float)ch1 ) ) / cpl;
  // lux = lux1 > lux2 ? lux1 : lux2;

  // Alternate lux calculation 1
  // See: https://github.com/adafruit/Adafruit_TSL2591_Library/issues/14
  lux = ( ((float)ch0 - (float)ch1 )) * (1.0F - ((float)ch1/(float)ch0) ) / cpl;

  // Alternate lux calculation 2
  // lux = ( (float)ch0 - ( 1.7F * (float)ch1 ) ) / cpl;

  // Signal I2C had no errors
  return lux;
}

/************************************************************************/
/*!
    @brief  Reads the raw data from both light channels
    @returns 32-bit raw count where high word is IR, low word is IR+Visible
*/
/**************************************************************************/
uint32_t Adafruit_TSL2591::getFullLuminosity (void)
{
  if (!_initialized) {
    if (!begin()) {
      return 0;
    }
  }

  boolean was_enabled = _enabled;
  if (!_enabled) {
    enable();
    // Wait x ms for ADC to complete
    for (uint8_t d=0; d<=_config_local.integrationtime; d++)
    {
      delay(120);
    }
  }

  // CHAN0 must be read before CHAN1
  // See: https://forums.adafruit.com/viewtopic.php?f=19&t=124176
  uint32_t x;
  uint16_t y;
  y = read16(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW);
  x = read16(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW);
  x <<= 16;
  x |= y;

  if (!was_enabled) {
    disable();
  }

  return x;
}

/************************************************************************/
/*!
    @brief  Reads the raw data from the channel
    @param  channel Can be 0 (IR+Visible, 1 (IR) or 2 (Visible only)
    @returns 16-bit raw count, or 0 if channel is invalid
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::getLuminosity (uint8_t channel)
{
  uint32_t x = getFullLuminosity();

  if (channel == TSL2591_FULLSPECTRUM)
  {
    // Reads two byte value from channel 0 (visible + infrared)
    return (x & 0xFFFF);
  }
  else if (channel == TSL2591_INFRARED)
  {
    // Reads two byte value from channel 1 (infrared)
    return (x >> 16);
  }
  else if (channel == TSL2591_VISIBLE)
  {
    // Reads all and subtracts out just the visible!
    return ( (x & 0xFFFF) - (x >> 16));
  }

  // unknown channel!
  return 0;
}

/************************************************************************/
/*!
    @brief  set up the ALS interrupt thresholds (interrupt generated when light level is outside the lower/upper range for long enough). (this is here for compatibily reasons only and DEPRECATED. please use the {@link setALSInterruptThresholds} instead!)
    @param  lowerThreshold Raw light data reading level that is the lower value threshold for interrupt
    @param  upperThreshold Raw light data reading level that is the higher value threshold for interrupt
    @param  persist How many counts we must be outside range for interrupt to fire, default is any single value
*/
/**************************************************************************/
void Adafruit_TSL2591::registerInterrupt(uint16_t lowerThreshold, uint16_t upperThreshold, tsl2591Persist_t persist = TSL2591_PERSIST_ANY)
{
  setALSInterruptThresholds(lowerThreshold, upperThreshold, persist);
}

/************************************************************************/
/*!
    @brief  set up the ALS interrupt thresholds (interrupt generated when light level is outside the lower/upper range for long enough).
*/
/**************************************************************************/
void Adafruit_TSL2591::_writeALSInterruptThresholds()
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  boolean was_enabled = _enabled;
  if (!_enabled) {
    enable();
  }
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_PERSIST_FILTER,
      _config_local.AINT_persistance);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AILTL,
      _config_local.AINT_threshold_lower);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AILTH,
      _config_local.AINT_threshold_lower >> 8);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AIHTL,
      _config_local.AINT_threshold_upper);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AIHTH,
      _config_local.AINT_threshold_upper >> 8);
  if (!was_enabled) {
    disable();
  }
}

/************************************************************************/
/*!
    @brief  set up the ALS interrupt thresholds (interrupt generated when light level is outside the lower/upper range for long enough).
    @param  lowerThreshold Raw light data reading level that is the lower value threshold for interrupt
    @param  upperThreshold Raw light data reading level that is the higher value threshold for interrupt
    @param  persist How many counts we must be outside range for interrupt to fire, default is any single value
*/
/**************************************************************************/
void Adafruit_TSL2591::setALSInterruptThresholds(
    uint16_t lowerThreshold,
    uint16_t upperThreshold,
    tsl2591Persist_t persist = TSL2591_PERSIST_ANY
) {
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  _config_local.AINT_threshold_lower = lowerThreshold;
  _config_local.AINT_threshold_upper = upperThreshold;
  _config_local.AINT_persistance = persist;
  _writeALSInterruptThresholds();
}

/************************************************************************/
/*!
    @brief  set up the NP interrupt thresholds (interrupt generated immediately after conversion when light level is outside the lower/upper range).
*/
/**************************************************************************/
void Adafruit_TSL2591::_writeNPInterruptThresholds()
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  boolean was_enabled = _enabled;
  if (!_enabled) {
    enable();
  }
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAILTL,
      _config_local.NPINTR_threshold_lower);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAILTH,
      _config_local.NPINTR_threshold_lower >> 8);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAIHTL,
      _config_local.NPINTR_threshold_upper);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAIHTH,
      _config_local.NPINTR_threshold_upper >> 8);

  if (!was_enabled) {
    disable();
  }
}

/************************************************************************/
/*!
    @brief  set up the NP interrupt thresholds (interrupt generated immediately after conversion when light level is outside the lower/upper range).
    @param  lowerThreshold Raw light data reading level that is the lower value threshold for interrupt
    @param  upperThreshold Raw light data reading level that is the higher value threshold for interrupt
*/
/**************************************************************************/
void Adafruit_TSL2591::setNPInterruptThresholds(uint16_t lowerThreshold, uint16_t upperThreshold)
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  _config_local.NPINTR_threshold_lower = lowerThreshold;
  _config_local.NPINTR_threshold_upper = upperThreshold;
  _writeNPInterruptThresholds();
}

/************************************************************************/
/*!
    @brief  Clear interrupt status
*/
/**************************************************************************/
void Adafruit_TSL2591::clearInterrupt()
{
  if (!_initialized) {
    if (!begin()) {
      return;
    }
  }

  boolean was_enabled = _enabled;
  if (!_enabled) {
    enable();
  }
  write8(TSL2591_CLEAR_INT);
  if (!was_enabled) {
    disable();
  }
}


/************************************************************************/
/*!
    @brief  Gets the most recent sensor event from the hardware status register.
    @return Sensor status as a byte. Bit 0 is ALS Valid. Bit 4 is ALS Interrupt. Bit 5 is No-persist Interrupt.
*/
/**************************************************************************/
uint8_t Adafruit_TSL2591::getStatus(void)
{
  if (!_initialized) {
    if (!begin()) {
      return 0;
    }
  }

  // Enable the device
  boolean was_enabled = _enabled;
  if (!_enabled) {
    enable();
  }
  uint8_t x;
  x = read8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_STATUS);
  if (!was_enabled) {
    disable();
  }
  return x;
}

/************************************************************************/
/*!
    @brief  convert persistance value to integer form
    @param  persistance {@link tsl2591Persist_t} persistance value
    @return persistance value as integer
*/
/**************************************************************************/
uint8_t Adafruit_TSL2591::convertPersistanceToInt(tsl2591Persist_t persistance)
{
    uint8_t result = 0;
    switch (persistance) {
        case TSL2591_PERSIST_EVERY: {
            result = 0;
        } break;
        case TSL2591_PERSIST_ANY: {
            result = 1;
        } break;
        case TSL2591_PERSIST_2: {
            result = 2;
        } break;
        case TSL2591_PERSIST_3: {
            result = 3;
        } break;
        case TSL2591_PERSIST_5: {
            result = 5;
        } break;
        case TSL2591_PERSIST_10: {
            result = 10;
        } break;
        case TSL2591_PERSIST_15: {
            result = 15;
        } break;
        case TSL2591_PERSIST_20: {
            result = 20;
        } break;
        case TSL2591_PERSIST_25: {
            result = 25;
        } break;
        case TSL2591_PERSIST_30: {
            result = 30;
        } break;
        case TSL2591_PERSIST_35: {
            result = 35;
        } break;
        case TSL2591_PERSIST_40: {
            result = 40;
        } break;
        case TSL2591_PERSIST_45: {
            result = 45;
        } break;
        case TSL2591_PERSIST_50: {
            result = 50;
        } break;
        case TSL2591_PERSIST_55: {
            result = 55;
        } break;
        case TSL2591_PERSIST_60: {
            result = 60;
        } break;
    }
    return result;
}

/************************************************************************/
/*!
    @brief  Print persistance value in Human readable form
    @param  out Print (=Serial) reference to use for printing
    @param  persistance {@link tsl2591Persist_t} persistance value
*/
/**************************************************************************/
void Adafruit_TSL2591::printPersistance(Print &out, tsl2591Persist_t persistance)
{
    switch (persistance) {
        case TSL2591_PERSIST_EVERY: {
            out.print(F("EVERY"));
        } break;
        case TSL2591_PERSIST_ANY: {
            out.print(F("ANY"));
        } break;
        case TSL2591_PERSIST_2:
        case TSL2591_PERSIST_3:
        case TSL2591_PERSIST_5:
        case TSL2591_PERSIST_10:
        case TSL2591_PERSIST_15:
        case TSL2591_PERSIST_20:
        case TSL2591_PERSIST_25:
        case TSL2591_PERSIST_30:
        case TSL2591_PERSIST_35:
        case TSL2591_PERSIST_40:
        case TSL2591_PERSIST_45:
        case TSL2591_PERSIST_50:
        case TSL2591_PERSIST_55:
        case TSL2591_PERSIST_60: {
            out.print(convertPersistanceToInt(persistance));
        } break;
    }
}

/************************************************************************/
/*!
    @brief  write current configuration to chip
*/
/**************************************************************************/
void Adafruit_TSL2591::_writeConfig() {
    if (!_initialized) {
      if (!begin()) {
        return;
      }
    }

    boolean was_enabled = _enabled;
    if (!_enabled) {
      enable();
    }

    // _writeIntegrationtimeGain
    write8(
        TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL,
        _config_current->integrationtime | _config_current->gain);
    // _writeALSInterruptThresholds
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_PERSIST_FILTER,
        _config_current->AINT_persistance);
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AILTL,
        _config_current->AINT_threshold_lower);
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AILTH,
        _config_current->AINT_threshold_lower >> 8);
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AIHTL,
        _config_current->AINT_threshold_upper);
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AIHTH,
        _config_current->AINT_threshold_upper >> 8);
    // _writeNPInterruptThresholds
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAILTL,
        _config_current->NPINTR_threshold_lower);
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAILTH,
        _config_current->NPINTR_threshold_lower >> 8);
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAIHTL,
        _config_current->NPINTR_threshold_upper);
    write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAIHTH,
        _config_current->NPINTR_threshold_upper >> 8);
    // clearInterrupt
    write8(TSL2591_CLEAR_INT);

    if (!was_enabled) {
      disable();
    }
}

/************************************************************************/
/*!
    @brief  copy parameters from config to internal local config
    @param  config {@link tsl2591Config_t} pointer to configuration
*/
/**************************************************************************/
void Adafruit_TSL2591::_copyConfigParamsToLocal(tsl2591Config_t *config) {
    _config_local.gain = config->gain;
    _config_local.integrationtime = config->integrationtime;
    _config_local.AINT_threshold_lower = config->AINT_threshold_lower;
    _config_local.AINT_threshold_upper = config->AINT_threshold_upper;
    _config_local.AINT_persistance = config->AINT_persistance;
    _config_local.NPINTR_threshold_lower = config->NPINTR_threshold_lower;
    _config_local.NPINTR_threshold_upper = config->NPINTR_threshold_upper;
}

/************************************************************************/
/*!
    @brief  set _config_current pointer to point to local configuration
*/
/**************************************************************************/
void Adafruit_TSL2591::_useLocalConfig() {
    _config_current = &_config_local;
}

/************************************************************************/
/*!
    @brief  set all configuration values
    @param  config {@link tsl2591Config_t} pointer to configuration
*/
/**************************************************************************/
void Adafruit_TSL2591::setConfig(tsl2591Config_t *config) {
    // _config_local = config;
    // the set functions are setting the _config_local values
    // and then write these to the chip.

    // TODO(s-light): think about options to directly
    // use the values from the pointed struct
    // to write to the chip in one go.

    _copyConfigParamsToLocal(config);
    _config_current = config;
    _writeConfig();

    // setGain(config->gain);
    // setTiming(config->integrationtime);
    //
    // setALSInterruptThresholds(
    //     config->AINT_threshold_lower,
    //     config->AINT_threshold_upper,
    //     config->AINT_persistance);
    // setNPInterruptThresholds(
    //     config->NPINTR_threshold_lower,
    //     config->NPINTR_threshold_upper);
    // clearInterrupt();
}

/************************************************************************/
/*!
    @brief  print all config parameters in human readable form
    @param  out {@link Print} reference to Serial to be used
    @param  config {@link tsl2591Config_t} pointer to configuration
*/
/**************************************************************************/
void Adafruit_TSL2591::printConfig(Print &out, tsl2591Config_t *config) {
    char buffer[] =
        "NPINTR Threshold Window: 88000 to 88000 with persistence 22   \0";
    snprintf(
        buffer, sizeof(buffer),
        "%-25s %5u",
        "Gain:",
        gainAsInt(config->gain));
    out.println(buffer);
    // printGain(out);
    // out.println();
    snprintf(
        buffer, sizeof(buffer),
        "%-25s %5u",
        "Integration Time:",
        getTimingInMS(config->integrationtime));
    out.println(buffer);
    // out.println(getTimingInMS());
    snprintf(
        buffer, sizeof(buffer),
        "%-25s %5u",
        "Max ADC Counts:",
        getMaxADCCounts(config->integrationtime));
    out.println(buffer);
    snprintf(
        buffer, sizeof(buffer),
        "%-25s %5u to %5u (%2ux)",
        "AINT Threshold Window:",
        config->AINT_threshold_lower,
        config->AINT_threshold_upper,
        config->AINT_persistance);
    out.println(buffer);
    snprintf(
        buffer, sizeof(buffer),
        "%-25s %5u to %5u ",
        "NPINTR Threshold Window:",
        config->NPINTR_threshold_lower,
        config->NPINTR_threshold_upper);
    out.println(buffer);
}

/************************************************************************/
/*!
    @brief  print all current config parameters in human readable form
    @param  out {@link Print} reference to Serial to be used
*/
/**************************************************************************/
void Adafruit_TSL2591::printConfig(Print &out) {
    printConfig(out, _config_current);
}

/************************************************************************/
/*!
    @brief  Gets the most recent sensor event
    @param  event Pointer to Adafruit_Sensor sensors_event_t object that will be filled with sensor data
    @return True on success, False on failure
*/
/**************************************************************************/
bool Adafruit_TSL2591::getEvent(sensors_event_t *event)
{
  uint16_t ir, full;
  uint32_t lum = getFullLuminosity();
  /* Early silicon seems to have issues when there is a sudden jump in */
  /* light levels. :( To work around this for now sample the sensor 2x */
  lum = getFullLuminosity();
  ir = lum >> 16;
  full = lum & 0xFFFF;

  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));

  event->version   = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type      = SENSOR_TYPE_LIGHT;
  event->timestamp = millis();

  /* Calculate the actual lux value */
  /* 0 = sensor overflow (too much light) */
  event->light = calculateLux(full, ir);

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the overall sensor_t data including the type, range and resulution
    @param  sensor Pointer to Adafruit_Sensor sensor_t object that will be filled with sensor type data
*/
/**************************************************************************/
void Adafruit_TSL2591::getSensor(sensor_t *sensor)
{
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy (sensor->name, "TSL2591", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name)- 1] = 0;
  sensor->version     = 1;
  sensor->sensor_id   = _sensorID;
  sensor->type        = SENSOR_TYPE_LIGHT;
  sensor->min_delay   = 0;
  sensor->max_value   = 88000.0;
  sensor->min_value   = 0.0;
  sensor->resolution  = 0.001;
}
/*******************************************************/

/**************************************************************************/
/*!
    @brief  read 8bit from sensor
    @param  reg register address
    @return register value
*/
/**************************************************************************/
uint8_t Adafruit_TSL2591::read8(uint8_t reg)
{
  uint8_t x;

  _i2c->beginTransmission(TSL2591_ADDR);
  _i2c->write(reg);
  _i2c->endTransmission();

  _i2c->requestFrom(TSL2591_ADDR, 1);
  x = _i2c->read();

  return x;
}

/**************************************************************************/
/*!
    @brief  read 16bit from sensor
    @param  reg register address
    @return register value
*/
/**************************************************************************/
uint16_t Adafruit_TSL2591::read16(uint8_t reg)
{
  uint16_t x;
  uint16_t t;

  _i2c->beginTransmission(TSL2591_ADDR);
  _i2c->write(reg);
  _i2c->endTransmission();

  _i2c->requestFrom(TSL2591_ADDR, 2);
  t = _i2c->read();
  x = _i2c->read();

  x <<= 8;
  x |= t;
  return x;
}

/**************************************************************************/
/*!
    @brief  write 8bit value to sensor register
    @param  reg register address
    @param value register value
*/
/**************************************************************************/
void Adafruit_TSL2591::write8 (uint8_t reg, uint8_t value)
{
  _i2c->beginTransmission(TSL2591_ADDR);
  _i2c->write(reg);
  _i2c->write(value);
  _i2c->endTransmission();
}

/**************************************************************************/
/*!
    @brief  write register address to sensor
    @param  reg register address
*/
/**************************************************************************/
void Adafruit_TSL2591::write8 (uint8_t reg)
{
  _i2c->beginTransmission(TSL2591_ADDR);
  _i2c->write(reg);
  _i2c->endTransmission();
}
