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

#ifndef XCSOAR_PROTECTED_AIRSPACE_WARNING_MANAGER_HPP
#define XCSOAR_PROTECTED_AIRSPACE_WARNING_MANAGER_HPP

#include "Thread/Guard.hpp"

struct AIRCRAFT_STATE;
class AirspaceWarning;
class AirspaceWarningManager;
class AirspaceWarningVisitor;
class AbstractAirspace;
class AtmosphericPressure;

class ProtectedAirspaceWarningManager : public Guard<AirspaceWarningManager> {
public:
  ProtectedAirspaceWarningManager(AirspaceWarningManager &awm):
    Guard<AirspaceWarningManager>(awm) {}

  void clear();
  void clear_warnings();

  // warning manager
  void visit_warnings(AirspaceWarningVisitor& visitor) const;

  void acknowledge_day(const AbstractAirspace& airspace,
                       const bool set=true);
  void acknowledge_warning(const AbstractAirspace& airspace,
                           const bool set=true);
  void acknowledge_inside(const AbstractAirspace& airspace,
                          const bool set=true);

  size_t warning_size() const;
  bool warning_empty() const;
  int get_warning_index(const AbstractAirspace& airspace) const;

  void reset_warning(const AIRCRAFT_STATE& as);
  bool update_warning(const AIRCRAFT_STATE &state, const bool circling);
};


#endif
