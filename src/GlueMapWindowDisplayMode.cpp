/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2010 The XCSoar Project
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

#include "GlueMapWindow.hpp"

#include "Units.hpp"

ZoomClimb_t::ZoomClimb_t():
  CruiseMapScale(fixed_ten),
  ClimbMapScale(fixed(0.25)),
  last_isclimb(false),
  last_targetpan(false) {}

void
GlueMapWindow::SwitchZoomClimb()
{
  bool isclimb = (visible_projection.GetDisplayMode() == dmCircling);

  bool my_target_pan = SettingsMap().TargetPan;

  if (my_target_pan != zoomclimb.last_targetpan) {
    if (my_target_pan) {
      // save starting values
      if (isclimb)
        zoomclimb.ClimbMapScale = visible_projection.GetMapScale();
      else
        zoomclimb.CruiseMapScale = visible_projection.GetMapScale();
    } else {
      // restore scales
      if (isclimb)
        visible_projection.RequestMapScale(zoomclimb.ClimbMapScale);
      else
        visible_projection.RequestMapScale(zoomclimb.CruiseMapScale);
    }
    zoomclimb.last_targetpan = my_target_pan;
    return;
  }

  if (!my_target_pan && SettingsMap().CircleZoom) {
    if (isclimb != zoomclimb.last_isclimb) {
      if (isclimb) {
        // save cruise scale
        zoomclimb.CruiseMapScale = visible_projection.GetMapScale();
        // switch to climb scale
        visible_projection.RequestMapScale(zoomclimb.ClimbMapScale);
      } else {
        // leaving climb
        // save cruise scale
        zoomclimb.ClimbMapScale = visible_projection.GetMapScale();
        // switch to climb scale
        visible_projection.RequestMapScale(zoomclimb.CruiseMapScale);
      }

      zoomclimb.last_isclimb = isclimb;
    }
  }
}

static DisplayMode_t
GetNewDisplayMode(const SETTINGS_MAP &settings_map,
                  const DERIVED_INFO &derived_info)
{
  if (settings_map.UserForceDisplayMode != dmNone)
    return settings_map.UserForceDisplayMode;
  else if (derived_info.Circling)
    return dmCircling;
  else if (derived_info.task_stats.flight_mode_final_glide)
    return dmFinalGlide;
  else
    return dmCruise;
}

void
GlueMapWindow::UpdateDisplayMode()
{
  DisplayMode_t old_mode = visible_projection.GetDisplayMode();
  DisplayMode_t new_mode = GetNewDisplayMode(SettingsMap(), Calculated());

  if (new_mode != old_mode) {
    visible_projection.SetDisplayMode(new_mode);
    SwitchZoomClimb();
  }
}