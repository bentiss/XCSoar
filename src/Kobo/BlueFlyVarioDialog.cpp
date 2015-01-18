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
#include "Thread/Trigger.hpp"
#include <vector>
#include <sstream>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

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

struct BlueFlyVarioSettingsDeclaration {
  const TCHAR *cmd;
  const TCHAR *label;
  const TCHAR *help;
  const StaticEnumChoice *list;
  unsigned *value;
};

struct BlueFlyVarioSettings {
  unsigned version;
  unsigned volume;
  unsigned outputMode;
};

static char settings_version[512] = "";
static char settings_keys[512] = "";
static char settings_values[512] = "";
static Trigger triggerSettings;

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

static bool
line_startswith(const char *line, const char *prefix)
{
  unsigned line_length = strlen(line);
  unsigned prefix_length = strlen(prefix);

  if (prefix_length > line_length)
    return false;

  return !strncmp(line, prefix, prefix_length);
}

class MyHandler final : public DataHandler {
public:
  virtual void DataReceived(const void *data, size_t length) {
    const char *cdata = (const char *)data;
    unsigned first = 0;

    for (unsigned i = 0; i < length ; i++) {
      if (cdata[i] == '\n') {
        strncat(buffer, &cdata[first], i - first);
        ProcessLine(buffer);
        strcpy(buffer, "");
        first = i + 1;
      }
    }
    if (first < length) {
      strncat(buffer, &cdata[first], length - first);
    }
  }

private:
  char buffer[512] = "";

  void ProcessLine(const char *line) {
    if (line[0] == '$')
      return;

    if (line_startswith(line, "BFV ")) {
      strcpy(settings_version, line);
      return;
    }

    if (line_startswith(line, "BST ")) {
      strcpy(settings_keys, line);
      return;
    }

    if (line_startswith(line, "SET ")) {
      strcpy(settings_values, line);
      if (line_startswith(settings_keys, "BST "))
        /* trigger only if we already read the keys */
        triggerSettings.Signal();
      return;
    }

  }
};

class BlueFlyVarioDialog final
  : public RowFormWidget {

public:
  BlueFlyVarioDialog(const DialogLook &look):RowFormWidget(look) {
    OpenSerial("/dev/ttymxc0", 57600);
    if (!port) {
      ShowMessageBox(_T("Unable to open the serial port..."), _("Error"),
                     MB_OK);
      return;
    }

    connected = ReadCurrentConfig();

    if (!connected) {
      MyLog("Unable to read parameters, is the BlueFlyVario connected?");
      ShowMessageBox(_T("Unable to read parameters, "
                        "is the BlueFlyVario connected?"), _("Error"),
                    MB_OK);
    }
  }
  ~BlueFlyVarioDialog() {
    if (port) {
      delete port;
      DeinitialiseIOThread();
    }
  }

  bool IsReady() { return connected; }

private:
 /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

  bool ReadCurrentConfig();
  void ParseCurrentVersion(const char *line);
  void ParseCurrentKeys(const char *line);
  bool ParseCurrentValues(const char *line);
  void OpenSerial(const TCHAR *path, unsigned baud_rate);
  void SendCommand(const char *cmd, int value);

  MyHandler handler;
  Port *port = nullptr;
  bool connected = false;
  struct BlueFlyVarioSettings settings;
  std::vector<std::string> keys;

  const struct BlueFlyVarioSettingsDeclaration decl[2] = {
    { .cmd = "BVL",
      .label = _("Volume"),
      .help = _("The volume of beeps \n"
                "-> 0.1 is only about 1/2 as loud as 1.0."),
      .list = volume_values,
      .value = &settings.volume,
    },
    { .cmd = "BOM",
      .label = _("outputMode"),
      .help = _("The output mode."),
      .list = output_mode_values,
      .value = &settings.outputMode,
    },
  };
};

bool
BlueFlyVarioDialog::ReadCurrentConfig()
{
  unsigned retry = 3;
  triggerSettings.Reset();
  while (retry--) {
    port->Write("$BST*");
    if (triggerSettings.Wait(500)) {
      ParseCurrentVersion(settings_version);
      ParseCurrentKeys(settings_keys);
      return ParseCurrentValues(settings_values);
    }
  }
  return false;
}

void
BlueFlyVarioDialog::ParseCurrentVersion(const char *line)
{
  if (!line_startswith(line, "BFV "))
    return;

  settings.version = atoi(&line[4]);
  MyLog("BlueFlyVario connected, version " + std::to_string(settings.version));
}

void
BlueFlyVarioDialog::ParseCurrentKeys(const char *line)
{
  if (!line_startswith(line, "BST "))
    return;

  std::istringstream ss(line);
  std::string token;

  while(std::getline(ss, token, ' '))
    keys.push_back(token);
}

bool
BlueFlyVarioDialog::ParseCurrentValues(const char *line)
{
  if (!line_startswith(line, "SET "))
    return false;

  std::vector<std::string> tokens;
  std::istringstream ss(line);
  std::string token;

  while(std::getline(ss, token, ' '))
    tokens.push_back(token);

  if (keys.size() != tokens.size() - 1)
    return false;

  MyLog(line);
  for (unsigned  i = 1; i < keys.size(); i++) {
    int value = std::stoi(tokens[i + 1]);
    MyLog(keys[i] + " -> " + tokens[i + 1]);
    for (unsigned param = 0 ; param < ARRAY_SIZE(decl); param++) {
      if (keys[i] == decl[param].cmd) {
        *decl[param].value = value;
        break;
      }
    }
  }

  return true;
}

void
BlueFlyVarioDialog::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  for (unsigned param = 0 ; param < ARRAY_SIZE(decl); param++)
    AddEnum(decl[param].label, decl[param].help, decl[param].list,
            *decl[param].value);
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

  command = "$" + std::string(cmd) + " " + std::to_string(value) + "*";
  MyLog(command);
  port->Write(command.c_str());
}

bool
BlueFlyVarioDialog::Save(bool &_changed)
{
  bool changed = false;

  MyLog("Save()");

  MyLog("vol:" + std::to_string(settings.volume));

  for (unsigned param = 0 ; param < ARRAY_SIZE(decl); param++) {
    if (SaveValueEnum(param, *decl[param].value)) {
      SendCommand(decl[param].cmd, *decl[param].value);
      changed = true;
    }
  }
  MyLog("vol:" + std::to_string(settings.volume));

  _changed |= changed;

  return true;
}

void
ShowBlueFlyVarioDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  BlueFlyVarioDialog widget(look);
  if (!widget.IsReady()) {
    MyLog("Leaving sooner");
    return;
  }
  WidgetDialog dialog(look);
  dialog.CreateFull(UIGlobals::GetMainWindow(), "System", &widget);
  dialog.AddButton(_("Close"), mrOK);
  dialog.ShowModal();
  dialog.StealWidget();
}
