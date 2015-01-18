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

#include "BlueFlyVarioDialog.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Dialogs/Message.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Form/ActionListener.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Enum.hpp"

#include "Device/Port/Port.hpp"
#include "Device/Port/ConfiguredPort.hpp"
#include "Device/Config.hpp"
#include "Operation/ConsoleOperationEnvironment.hpp"
#include "IO/Async/GlobalIOThread.hpp"
#include "IO/DataHandler.hpp"

static constexpr StaticEnumChoice volume_values[] = {
  { 0, N_("0 - Muted"), NULL },
  { 1, N_("0.001"), NULL },
  { 2, N_("0.002"), NULL },
  { 5, N_("0.005"), NULL },
  { 10, N_("0.01"), NULL },
  { 20, N_("0.02"), NULL },
  { 50, N_("0.05"), NULL },
  { 100, N_("0.1"), NULL },
  { 1000, N_("1.0 - Max"), NULL },
  { 0 }
};

static constexpr StaticEnumChoice output_mode_values[] = {
  { 0, N_("BlueFlyVario"), NULL },
  { 1, N_("LK8EX1"), NULL },
  { 2, N_("LX"), NULL },
  { 3, N_("FlyNet"), NULL },
  { 0 }
};

static void
MyLog(std::string data)
{
  printf("%s\n", data.c_str());
}

static void
MyLog(const char *data)
{
  std::string sdata = data;
  MyLog(sdata);
}

class MyHandler final : public DataHandler {
public:
  virtual void DataReceived(const void *data, size_t length) {
    const char *cdata = (const char *)data;
    printf("%s\n", cdata);
  }
};

class BlueFlyVarioDialog final
  : public RowFormWidget {
  enum ControlIndex {
    Volume,
  };

public:
  BlueFlyVarioDialog(const DialogLook &look):RowFormWidget(look) {
    OpenSerial("/dev/ttymxc0", 57600);
  }
  ~BlueFlyVarioDialog() {
    if (port) {
      delete port;
      DeinitialiseIOThread();
    }
  }

private:
 /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;


  void OpenSerial(const TCHAR *path, unsigned baud_rate);
  void SendCommand(const char *cmd, int value);

  MyHandler handler;
  Port *port = nullptr;
  unsigned int volume = 0;
  unsigned int output_mode = 0;
};

void
BlueFlyVarioDialog::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  AddEnum(_("Volume"),
          _("The volume of beeps -> 0.1 is only about 1/2 as loud as 1.0."),
          volume_values, volume);
  AddEnum(_("outputMode"),
          _("The output mode."),
          output_mode_values, output_mode);
}

void
BlueFlyVarioDialog::OpenSerial(const TCHAR *path, unsigned baud_rate)
{
  if (port)
    return;

  MyLog("Attempting to open serial port");
  MyLog(path);

  DeviceConfig config;
  config.Clear();

  config.port_type = DeviceConfig::PortType::SERIAL;
  config.path = path;
  config.baud_rate = baud_rate;

  InitialiseIOThread();

  port = OpenPort(config, nullptr, handler);
  if (port == NULL) {
    MyLog("Failed to open COM port");
    return;
  }

  ConsoleOperationEnvironment env;

  if (!port->WaitConnected(env)) {
    delete port;
    DeinitialiseIOThread();
    MyLog("Failed to connect the port");
    return;
  }

  if (!port->StartRxThread()) {
    delete port;
    MyLog("Failed to start the port thread");
    return;
  }

  MyLog("Port opened");
}

void
BlueFlyVarioDialog::SendCommand(const char *cmd, int value)
{
  std::string command;
  if (!port) {
    MyLog("Port not opened, sth went wrong");
    return;
  }

  command = "$" + std::string(cmd) + " " + std::to_string(value) + "*";
  MyLog(command);
  port->Write(command.c_str());
}

bool
BlueFlyVarioDialog::Save(bool &_changed)
{
  bool changed = false;
  unsigned value;

  MyLog("Save()");

  MyLog("vol:" + std::to_string(volume));
  if (SaveValueEnum(Volume, value)) {
    SendCommand("BVL", value);
    changed = true;
  }
  MyLog("vol:" + std::to_string(value));

  _changed |= changed;

  return true;
}

void
ShowBlueFlyVarioDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  BlueFlyVarioDialog widget(look);
  WidgetDialog dialog(look);
  dialog.CreateFull(UIGlobals::GetMainWindow(), "System", &widget);
  dialog.AddButton(_("Close"), mrOK);
  dialog.ShowModal();
  dialog.StealWidget();
}
