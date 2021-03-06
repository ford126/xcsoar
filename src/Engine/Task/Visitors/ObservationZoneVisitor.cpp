/* Copyright_License {

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

#include "ObservationZoneVisitor.hpp"
#include "Task/ObservationZones/LineSectorZone.hpp"
#include "Task/ObservationZones/SectorZone.hpp"
#include "Task/ObservationZones/FAISectorZone.hpp"
#include "Task/ObservationZones/CylinderZone.hpp"
#include "Task/ObservationZones/KeyholeZone.hpp"
#include "Task/ObservationZones/BGAFixedCourseZone.hpp"
#include "Task/ObservationZones/BGAEnhancedOptionZone.hpp"
#include "Task/Tasks/BaseTask/ObservationZonePoint.hpp"

void
ObservationZoneConstVisitor::Visit(const ObservationZonePoint &ozp)
{
  switch (ozp.shape) {
  case ObservationZonePoint::FAI_SECTOR:
    Visit((const FAISectorZone &)ozp);
    break;

  case ObservationZonePoint::SECTOR:
    Visit((const SectorZone &)ozp);
    break;

  case ObservationZonePoint::LINE:
    Visit((const LineSectorZone &)ozp);
    break;

  case ObservationZonePoint::CYLINDER:
    Visit((const CylinderZone &)ozp);
    break;

  case ObservationZonePoint::KEYHOLE:
    Visit((const KeyholeZone &)ozp);
    break;

  case ObservationZonePoint::BGAFIXEDCOURSE:
    Visit((const BGAFixedCourseZone &)ozp);
    break;

  case ObservationZonePoint::BGAENHANCEDOPTION:
    Visit((const BGAEnhancedOptionZone &)ozp);
    break;
  }
}

void
ObservationZoneVisitor::Visit(ObservationZonePoint &ozp)
{
  switch (ozp.shape) {
  case ObservationZonePoint::FAI_SECTOR:
    Visit((FAISectorZone &)ozp);
    break;

  case ObservationZonePoint::SECTOR:
    Visit((SectorZone &)ozp);
    break;

  case ObservationZonePoint::LINE:
    Visit((LineSectorZone &)ozp);
    break;

  case ObservationZonePoint::CYLINDER:
    Visit((CylinderZone &)ozp);
    break;

  case ObservationZonePoint::KEYHOLE:
    Visit((KeyholeZone &)ozp);
    break;

  case ObservationZonePoint::BGAFIXEDCOURSE:
    Visit((BGAFixedCourseZone &)ozp);
    break;

  case ObservationZonePoint::BGAENHANCEDOPTION:
    Visit((BGAEnhancedOptionZone &)ozp);
    break;
  }
}
