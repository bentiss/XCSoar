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

#include "IconLook.hpp"
#include "Screen/Layout.hpp"
#include "Resources.hpp"

void
IconLook::Initialise()
{
  hBmpTabTask.Load(Layout::scale > 1 ? IDB_TASK_HD : IDB_TASK);
  hBmpTabWrench.Load(Layout::scale > 1 ? IDB_WRENCH_HD : IDB_WRENCH);
  hBmpTabSettings.Load(Layout::scale > 1 ? IDB_SETTINGS_HD : IDB_SETTINGS);
  hBmpTabCalculator.Load(Layout::scale > 1 ? IDB_CALCULATOR_HD : IDB_CALCULATOR);

  hBmpTabFlight.Load(Layout::scale > 1 ? IDB_GLOBE_HD : IDB_GLOBE);
  hBmpTabSystem.Load(Layout::scale > 1 ? IDB_DEVICE_HD : IDB_DEVICE);
  hBmpTabRules.Load(Layout::scale > 1 ? IDB_RULES_HD : IDB_RULES);
  hBmpTabTimes.Load(Layout::scale > 1 ? IDB_CLOCK_HD : IDB_CLOCK);
}
