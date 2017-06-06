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

Adafruit_TSL2591::Adafruit_TSL2591(int32_t sensorID)
{
  _initialized = false;
  _sensorID    = sensorID;
}

boolean Adafruit_TSL2591::begin(void)
{
  if( ! tsl.begin() )
	return false;

  disable();
  _initialized = true;

  return true;
}

void Adafruit_TSL2591::enable(void)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  tsl.enable();
}

void Adafruit_TSL2591::disable(void)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  tsl.disable();
}

void Adafruit_TSL2591::setGain(tsl2591Gain_t gain)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  tsl.setGain( gain );
  disable();
}

tsl2591Gain_t Adafruit_TSL2591::getGain()
{
  return tsl.getGain();
}

void Adafruit_TSL2591::setTiming(tsl2591IntegrationTime_t integration)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  tsl.setTiming( integration );
  disable();
}

tsl2591IntegrationTime_t Adafruit_TSL2591::getTiming()
{
  return tsl.getTiming();
}

uint32_t Adafruit_TSL2591::calculateLux(uint16_t ch0, uint16_t ch1)
{
  return tsl.calculateLux( ch0, ch1 );
}

uint32_t Adafruit_TSL2591::getFullLuminosity (void)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return 0;
    }
  }

  // Enable the device
  enable();

  // Wait x ms for ADC to complete
  for (uint8_t d=0; d<=tsl.getTiming(); d++)
  {
    delay(120);
  }

  uint32_t x = tsl.getFullLuminosity();
  // keep old API braindamage
  if( x == UINT32_MAX )
	x = 0;

  disable();

  return x;
}

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

void Adafruit_TSL2591::registerInterrupt(uint16_t lowerThreshold, uint16_t upperThreshold)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  tsl.registerInterrupt( lowerThreshold, upperThreshold );
  disable();
}

void Adafruit_TSL2591::registerInterrupt(uint16_t lowerThreshold, uint16_t upperThreshold, tsl2591Persist_t persist)
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  tsl.registerInterrupt( lowerThreshold, upperThreshold, persist );
  disable();
}

void Adafruit_TSL2591::clearInterrupt()
{
  if (!_initialized)
  {
    if (!begin())
    {
      return;
    }
  }

  tsl.clearInterrupt();
  disable();
}


uint8_t Adafruit_TSL2591::getStatus()
{
  if (!_initialized)
  {
    if (!begin())
    {
      return 0;
    }
  }

  uint8_t x = tsl.getStatus();
  disable();
  return x;
}


/**************************************************************************/
/*!
    @brief  Gets the most recent sensor event
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
    @brief  Gets the sensor_t data
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
  sensor->resolution  = 1.0;
}
