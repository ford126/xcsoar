/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
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

#include "Polar/Loader.hpp"
#include "Polar/Historical.hpp"
#include "Polar/WinPilot.hpp"
#include "Polar/BuiltIn.hpp"
#include "LogFile.hpp"
#include "Language.hpp"
#include "Dialogs/Message.hpp"

#include "GlideSolvers/GlidePolar.hpp"
#include "Polar/Polar.hpp"

static bool
LoadPolarById2(unsigned id, Polar &a_polar)
{
  if (id < POLARUSEWINPILOTFILE)
    // polar data from historical table
    return LoadHistoricalPolar(id, a_polar);
  else if (id == POLARUSEWINPILOTFILE)
    // polar data from winpilot file
    return ReadWinPilotPolar(a_polar);
  else
    // polar data from built-in table
    return ReadWinPilotPolarInternal(id - POLARUSEWINPILOTFILE - 1, a_polar);
}


void
setGlidePolar(const Polar &polar, GlidePolar& gp)
{
  gp.empty_mass = fixed(polar.WEIGHTS[0] + polar.WEIGHTS[1]);
  gp.ballast_ratio = fixed(polar.WEIGHTS[2]) / gp.empty_mass;
  gp.wing_area = fixed(polar.WingArea);

  gp.ideal_polar_a = -fixed(polar.POLAR[0]) / sqrt(gp.empty_mass);
  gp.ideal_polar_b = -fixed(polar.POLAR[1]);
  gp.ideal_polar_c = -fixed(polar.POLAR[2]) * sqrt(gp.empty_mass);

  gp.update();
}

bool
LoadPolarById(unsigned id, GlidePolar& gp)
{
  LogStartUp(_T("Load polar"));

  Polar polar;
  if (!LoadPolarById2(id, polar)) {
    MessageBoxX(_("Error loading Polar file!\nUsing LS8 Polar."),
                _("Warning"), MB_OK | MB_ICONERROR);

    if (!LoadHistoricalPolar(2, polar))
      return false;
  }

  setGlidePolar(polar, gp);
  return true;
}
