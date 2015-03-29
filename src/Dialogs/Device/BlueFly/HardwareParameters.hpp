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

#ifndef XCSOAR_BLUEFLY_HARDWARE_PARAMETERS_HPP
#define XCSOAR_BLUEFLY_HARDWARE_PARAMETERS_HPP

#include "Device/Driver/BlueFly/Internal.hpp"
#include "Form/DataField/Enum.hpp"
#include "Language/Language.hpp"
#include "Operation/PopupOperationEnvironment.hpp"
#include "Widget/RowFormWidget.hpp"

static const StaticEnumChoice modes[] = {
  { 0, N_("BlueFlyVario") },
  { 1, N_("LK8EX1") },
  { 2, N_("LX") },
  { 3, N_("FlyNet") },
  { 0 }
};

class BlueFlyHardwareParametersWidget : public RowFormWidget {
private:
  enum Parameters {
    VOLUME,
    OUTPUT_MODE,

    LAST_PARAM = OUTPUT_MODE,
  };

  struct BlueFlyDevice::BlueFlySettings params;
  BlueFlyDevice &device;

public:
  BlueFlyHardwareParametersWidget(const DialogLook &look, BlueFlyDevice &_device)
    : RowFormWidget(look), device(_device) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override
  {
    RowFormWidget::Prepare(parent, rc);

    AddFloat(N_("Volume"), nullptr,
             _T("%.2f"),
             _T("%.2f"),
             0, 1.0, 0.1, true, 0);
    
    AddEnum(N_("Output mode"), nullptr, modes);
  }

  /* methods from Widget */
  virtual void Show(const PixelRect &rc) override
  {
    device.GetSettings(&params);

    LoadValue(VOLUME, params.volume);
    LoadValueEnum(OUTPUT_MODE, params.output_mode);

    RowFormWidget::Show(rc);
  }

  virtual bool Save(bool &changed) override
  {
    PopupOperationEnvironment env;

    changed |= SaveValue(VOLUME, params.volume);
    changed |= SaveValue(OUTPUT_MODE, params.output_mode);

    device.WriteDeviceSettings(&params, env);

    return true;
  }
};

#endif
