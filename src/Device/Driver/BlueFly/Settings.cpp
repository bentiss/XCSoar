/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Internal.hpp"
#include "Device/Util/NMEAWriter.hpp"
#include "LogFile.hpp"
#include "NMEA/Derived.hpp"

#include <stdio.h>

#ifdef _UNICODE
#include <windows.h>
#endif

bool
BlueFlyDevice::WriteDeviceSetting(const char *name, int value, OperationEnvironment &env)
{
  char buffer[64];

  assert(strlen(name) == 3);

  sprintf(buffer, "%s %d", name, value);
  LogFormat("BlueFLy: sending: '$%s'", buffer);
  return PortWriteNMEA(port, buffer, env);
}

bool
BlueFlyDevice::RequestSettings(OperationEnvironment &env)
{
  trigger_settings_ready.Reset();
  return PortWriteNMEA(port, "BST", env);
}

bool
BlueFlyDevice::WaitForSettings(unsigned int timeout)
{
  return trigger_settings_ready.Wait(timeout);
}

void
BlueFlyDevice::GetSettings(struct BlueFlySettings *new_settings)
{
  mutex_settings.Lock();
  *new_settings = settings;
  mutex_settings.Unlock();
}

gcc_pure
static inline
unsigned ComputeVolume(fixed value)
{
  assert(value >= 0);
  unsigned v = value * BlueFlyDevice::BlueFlySettings::VOLUME_MULTIPLIER;

  assert(v <= BlueFlyDevice::BlueFlySettings::VOLUME_MAX);
  return v;
}

gcc_pure
static inline
unsigned ComputeOutputMode(unsigned value)
{
  assert(value <= BlueFlyDevice::BlueFlySettings::OUTPUT_MODE_MAX);
  return value;
}

void
BlueFlyDevice::WriteDeviceSettings(struct BlueFlySettings *new_settings, OperationEnvironment &env)
{
  if (new_settings->volume != settings.volume)
    WriteDeviceSetting(settings.VOLUME_NAME,
                       ComputeVolume(new_settings->volume), env);
  if (new_settings->output_mode != settings.output_mode)
    WriteDeviceSetting(settings.OUTPUT_MODE_NAME,
                       ComputeOutputMode(new_settings->output_mode), env);

  /* update the old values from the new settings.
   * The BlueFly Vario does not send back any ACK. */
  mutex_settings.Lock();
  settings = *new_settings;
  mutex_settings.Unlock();
}
