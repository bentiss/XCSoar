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

class BlueFlyVarioDialog final
  : public RowFormWidget {
  enum ControlIndex {
    Volume,
  };

public:
  BlueFlyVarioDialog(const DialogLook &look):RowFormWidget(look) {}

private:
 /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;


  void SendCommand(const char *cmd, int value);

  bool tty = false;
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
BlueFlyVarioDialog::SendCommand(const char *cmd, int value)
{
  std::string command;
  if (!tty) {
    system("stty ospeed 57600 ispeed 57600 -F /dev/ttymxc0");
    system("stty -F /dev/ttymxc0 raw");
    tty = true;
  }

  command = "echo '\\$" + std::string(cmd) + " " + std::to_string(value) + "*' > /dev/ttymxc0";
  system(command.c_str());
  MyLog(command);
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
