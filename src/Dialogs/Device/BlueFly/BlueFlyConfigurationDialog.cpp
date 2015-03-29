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

#include "BlueFlyDialogs.hpp"
#include "Compiler.h"
#include "Dialogs/WidgetDialog.hpp"
#include "Dialogs/Message.hpp"
#include "Device/Driver/BlueFly/Internal.hpp"
#include "Language/Language.hpp"
#include "Look/DialogLook.hpp"
#include "Operation/MessageOperationEnvironment.hpp"
#include "Operation/PopupOperationEnvironment.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Widget/ArrowPagerWidget.hpp"

#include "HardwareParameters.hpp"

#include <assert.h>

static const TCHAR *const captions[] = {
  _T(" 1 Hardware"),
  _T(" 2 Thermal Settings"),
};

static bool changed;

class BlueFlyConfigurationExtraButtons final
  : public NullWidget, ActionListener {
  enum Buttons {
    SAVE,
  };

  WidgetDialog &dialog;

  WndButton save_button;

public:
  BlueFlyConfigurationExtraButtons(WidgetDialog &_dialog)
    :dialog(_dialog),
     save_button(dialog.GetLook().button) {}

protected:
  /* virtual methods from Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override {

    ButtonWindowStyle style;
    style.Hide();
    style.TabStop();

    save_button.Create(parent, _("Save"), rc, style, *this, SAVE);
  }

  virtual void Show(const PixelRect &rc) override {
    save_button.MoveAndShow(rc);
  }

  virtual void Hide() override {
    save_button.FastHide();
  }

  virtual void Move(const PixelRect &rc) override {
    save_button.Move(rc);
  }

private:
  void OnSave();

  /* virtual methods from ActionListener */
  virtual void OnAction(int id) override {
    switch (id) {
    case SAVE:
      OnSave();
      break;
    }
  }
};

static void
UpdateCaption(WndForm &form, unsigned page)
{
  form.SetCaption(captions[page]);
}

inline void
BlueFlyConfigurationExtraButtons::OnSave()
{
  bool _changed = false;
  if (!dialog.GetWidget().Save(_changed))
    return;

  changed |= _changed;
}

/**
 * Request all parameter values from the BlueFly Vario.
 */
static void
RequestAll(BlueFlyDevice &device)
{
  PopupOperationEnvironment env;
  int retry = 3;

  while (retry--) {
    device.RequestSettings(env);
    if (device.WaitForSettings(500))
      break;
  }
}

static void
FillPager(PagerWidget &pager, BlueFlyDevice &device)
{
  const DialogLook &look = UIGlobals::GetDialogLook();

  pager.Add(new BlueFlyHardwareParametersWidget(look, device));
}

bool
dlgConfigurationBlueFlyVarioShowModal(Device &_device)
{
  BlueFlyDevice &device = (BlueFlyDevice &)_device;
  changed = false;

  RequestAll(device);

  const DialogLook &look = UIGlobals::GetDialogLook();

  WidgetDialog dialog(look);

  ArrowPagerWidget widget(dialog, look.button,
                          new BlueFlyConfigurationExtraButtons(dialog));
  FillPager(widget, device);

  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Vario Configuration"),
                    &widget);

  widget.SetPageFlippedCallback([&dialog, &widget](){
      UpdateCaption(dialog, widget.GetCurrentIndex());
    });
  UpdateCaption(dialog, widget.GetCurrentIndex());

  dialog.ShowModal();
  dialog.StealWidget();

  return changed || dialog.GetChanged();
}
