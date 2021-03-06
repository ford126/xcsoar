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

#include "GlueMapWindow.hpp"
#include "Components.hpp"
#include "DrawThread.hpp"

GlueMapWindow::GlueMapWindow()
  :idle_robin(2),
   drag_mode(DRAG_NONE),
   ignore_single_click(false),
   DisplayMode(dmCruise) {}

void
GlueMapWindow::set(ContainerWindow &parent, const RECT &rc)
{
  MapWindow::set(parent, rc);

  LoadDisplayModeScales();
  visible_projection.SetScale(zoomclimb.CruiseScale);
}

void
GlueMapWindow::QuickRedraw(const SETTINGS_MAP &_settings_map)
{
  assert(&_settings_map != &SettingsMap());

  ReadSettingsMap(_settings_map);

#ifndef ENABLE_OPENGL
  /* update the Projection */

  UpdateScreenAngle();
  UpdateProjection();

  ++ui_generation;

  /* quickly stretch the existing buffer into the window */

  scale_buffer = 2;
#endif

  invalidate();
}

/**
 * This idle function allows progressive scanning of visibility etc
 */
bool
GlueMapWindow::Idle()
{
  bool still_dirty;
  bool topology_dirty = true; /* scan topology in every Idle() call */
  bool terrain_dirty = true;
  bool weather_dirty = true;

  // StartTimer();

  do {
    idle_robin = (idle_robin + 1) % 3;
    switch (idle_robin) {
    case 0:
      topology_dirty = UpdateTopology(1) > 0;
      break;

    case 1:
      terrain_dirty = UpdateTerrain();
      break;

    case 2:
      weather_dirty = UpdateWeather();
      break;
    }

    still_dirty = terrain_dirty || topology_dirty || weather_dirty;
  } while (RenderTimeAvailable() &&
#ifndef ENABLE_OPENGL
           !draw_thread->is_triggered() &&
#endif
           still_dirty);

  return still_dirty;
}
