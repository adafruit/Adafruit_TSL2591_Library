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
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#if defined(__AVR__)
#include <util/delay.h>
#endif
#include <stdlib.h>

#include "Adafruit_TSL2591.h"

Sensor_TSL2591::Sensor_TSL2591()
{
  _integration = TSL2591_INTEGRATIONTIME_100MS;
  _gain        = TSL2591_GAIN_MED;

  // we cant do wire initialization till later, because we havent loaded Wire yet
}

boolean Sensor_TSL2591::begin(void)
{
  Wire.begin();

  uint8_t id = read8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID);
  if (id == 0x50 )
  {
     // Serial.println("Found Sensor_TSL2591");
  }
  else
  {
    return false;
  }

  // Set default integration time and gain
  setTiming(_integration);
  setGain(_gain);

  return true;
}

void Sensor_TSL2591::enable(void)
{
  // Enable the device by setting the control bit to 0x01
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN | TSL2591_ENABLE_NPIEN);
}

void Sensor_TSL2591::disable(void)
{
  // Disable the device by setting the control bit to 0x00
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWEROFF);
}

void Sensor_TSL2591::setGain(tsl2591Gain_t gain)
{
  enable();
  _gain = gain;
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL, _integration | _gain);
}

tsl2591Gain_t Sensor_TSL2591::getGain()
{
  return _gain;
}

void Sensor_TSL2591::setTiming(tsl2591IntegrationTime_t integration)
{
  enable();
  _integration = integration;
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL, _integration | _gain);
}

tsl2591IntegrationTime_t Sensor_TSL2591::getTiming()
{
  return _integration;
}

uint16_t Sensor_TSL2591::getTime()
{
  switch (_integration)
  {
    case TSL2591_INTEGRATIONTIME_100MS :
      return 100;
    case TSL2591_INTEGRATIONTIME_200MS :
      return 200;
    case TSL2591_INTEGRATIONTIME_300MS :
      return 300;
    case TSL2591_INTEGRATIONTIME_400MS :
      return 400;
    case TSL2591_INTEGRATIONTIME_500MS :
      return 500;
    case TSL2591_INTEGRATIONTIME_600MS :
      return 600;
  }

  return 100;
}

uint32_t Sensor_TSL2591::getClip()
{
  switch (_integration)
  {
    case TSL2591_INTEGRATIONTIME_100MS :
      return TSL2591_CLIP_100MS;

    case TSL2591_INTEGRATIONTIME_200MS :
    case TSL2591_INTEGRATIONTIME_300MS :
    case TSL2591_INTEGRATIONTIME_400MS :
    case TSL2591_INTEGRATIONTIME_500MS :
    case TSL2591_INTEGRATIONTIME_600MS :
      return TSL2591_CLIP_OTHER;

  }

  return TSL2591_CLIP_OTHER;
}

uint16_t Sensor_TSL2591::getGainFactor()
{
  switch (_gain)
  {
    case TSL2591_GAIN_LOW :
      return 1;
    case TSL2591_GAIN_MED :
      return 25;
    case TSL2591_GAIN_HIGH :
      return 428;
    case TSL2591_GAIN_MAX :
      return 9876;
  }

  return 1;
}

uint32_t Sensor_TSL2591::calculateLux(uint16_t ch0, uint16_t ch1)
{
  uint16_t clip = getClip();

  // Note: This algorithm is based on preliminary coefficients
  // provided by AMS and may need to be updated in the future

  // Check for overflow conditions first
  if ((ch0 >= clip) | (ch1 >= clip))
  {
    // Signal an overflow
    return TSL2591_LUX_CLIPPED;
  }

  uint16_t atime = getTime();
  uint16_t again = getGainFactor();

  // cpl = (ATIME * AGAIN) / DF
  float cpl = (float) (atime * again) / TSL2591_LUX_DF;

  float lux1 = ( (float)ch0 - (TSL2591_LUX_COEFB * (float)ch1) ) / cpl;
  float lux2 = ( ( TSL2591_LUX_COEFC * (float)ch0 ) - ( TSL2591_LUX_COEFD * (float)ch1 ) ) / cpl;
  float lux = lux1 > lux2 ? lux1 : lux2;

  // Alternate lux calculation
  //lux = ( (float)ch0 - ( 1.7F * (float)ch1 ) ) / cpl;

  if( lux < 0 ) lux = 0;

  // Signal I2C had no errors
  return (uint32_t)lux;
}

uint32_t Sensor_TSL2591::getFullLuminosity (void)
{
  // confirm, ADC result is valid
  uint8_t status = read8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_STATUS);
  if( ! status & TSL2591_STATUS_AVALID )
    return UINT32_MAX;

  Wire.beginTransmission(TSL2591_ADDR);
#if ARDUINO >= 100
  if( 1 != Wire.write(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW) )
    return UINT32_MAX;
#else
  if( 1 != Wire.send(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW) )
    return UINT32_MAX;
#endif
  if( 0 != Wire.endTransmission() )
    return UINT32_MAX;

  if( 4 > Wire.requestFrom(TSL2591_ADDR, 4) )
    return UINT32_MAX;

  uint32_t x;
#if ARDUINO >= 100
  // ch0
  x = Wire.read();
  x |= (uint32_t)Wire.read() <<8;
  // ch1
  x |= (uint32_t)Wire.read() <<16;
  x |= (uint32_t)Wire.read() <<24;
#else
  // ch0
  x = Wire.receive();
  x |= (uint32_t)Wire.receive() <<8;
  // ch1
  x |= (uint32_t)Wire.receive() <<16;
  x |= (uint32_t)Wire.receive() <<24;
#endif

  return x;
}

void Sensor_TSL2591::registerInterrupt(uint16_t lowerThreshold, uint16_t upperThreshold)
{
  enable();
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAILTL, lowerThreshold);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAILTH, lowerThreshold >> 8);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAIHTL, upperThreshold);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_NPAIHTH, upperThreshold >> 8);
}

void Sensor_TSL2591::registerInterrupt(uint16_t lowerThreshold, uint16_t upperThreshold, tsl2591Persist_t persist)
{
  enable();
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_PERSIST_FILTER,  persist);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AILTL, lowerThreshold);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AILTH, lowerThreshold >> 8);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AIHTL, upperThreshold);
  write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_THRESHOLD_AIHTH, upperThreshold >> 8);
}

void Sensor_TSL2591::clearInterrupt()
{
  enable();
  write8(TSL2591_CLEAR_INT);
}


uint8_t Sensor_TSL2591::getStatus()
{
  // Enable the device
  enable();

  return read8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_STATUS);
}


uint8_t Sensor_TSL2591::read8(uint8_t reg)
{
  uint8_t x;

  Wire.beginTransmission(TSL2591_ADDR);
#if ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif
  Wire.endTransmission();

  Wire.requestFrom(TSL2591_ADDR, 1);
#if ARDUINO >= 100
  x = Wire.read();
#else
  x = Wire.receive();
#endif
  // while (! Wire.available());
  // return Wire.read();
  return x;
}

uint16_t Sensor_TSL2591::read16(uint8_t reg)
{
  uint16_t x;
  uint16_t t;

  Wire.beginTransmission(TSL2591_ADDR);
#if ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif
  Wire.endTransmission();

  Wire.requestFrom(TSL2591_ADDR, 2);
#if ARDUINO >= 100
  t = Wire.read();
  x = Wire.read();
#else
  t = Wire.receive();
  x = Wire.receive();
#endif
  x <<= 8;
  x |= t;
  return x;
}

void Sensor_TSL2591::write8 (uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(TSL2591_ADDR);
#if ARDUINO >= 100
  Wire.write(reg);
  Wire.write(value);
#else
  Wire.send(reg);
  Wire.send(value);
#endif
  Wire.endTransmission();
}


void Sensor_TSL2591::write8 (uint8_t reg)
{
  Wire.beginTransmission(TSL2591_ADDR);
#if ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif
  Wire.endTransmission();
}

