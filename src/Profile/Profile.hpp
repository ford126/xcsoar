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

#ifndef XCSOAR_PROFILE_HPP
#define XCSOAR_PROFILE_HPP

#include "Profile/ProfileKeys.hpp"
#include "Math/fixed.hpp"

#if defined(WIN32) && !defined(USE_PROFILE_MAP)
#include "Profile/Registry.hpp"
namespace ProfileImpl = Registry;
#else
#include "Profile/ProfileMap.hpp"
namespace ProfileImpl = ProfileMap;
#endif

#include <stddef.h>
#include <tchar.h>

struct GeoPoint;

namespace Profile
{
  using namespace ProfileImpl;

  static inline bool
  use_files()
  {
#ifdef PROFILE_NO_FILE
    return false;
#else
    return true;
#endif
  }

  /**
   * Loads the profile files
   */
  void Load();
  /**
   * Loads the given profile file
   */
  void LoadFile(const TCHAR *szFile);

  /**
   * Saves the profile into the profile files
   */
  void Save();
  /**
   * Saves the profile into the given profile file
   */
  void SaveFile(const TCHAR *szFile);

  /**
   * Sets the profile files to load when calling Load()
   * @param override NULL or file to load when calling Load()
   */
  void SetFiles(const TCHAR* override);

  /**
   * Reads a configured path from the profile, and expands it with
   * ExpandLocalPath().
   *
   * @param value a buffer which can store at least MAX_PATH
   * characters
   */
  bool GetPath(const TCHAR *key, TCHAR *value);

  /**
   * Load a GeoPoint from the profile.
   */
  bool GetGeoPoint(const TCHAR *key, GeoPoint &value);

  /**
   * Save a GeoPoint from the profile.  It is stored as a string,
   * longitude and latitude formatted in degrees separated by a space
   * character.
   */
  bool SetGeoPoint(const TCHAR *key, const GeoPoint &value);

  /**
   * Adjusts the application settings according to the profile settings
   */
  void Use();

  /**
   * Saves the sound settings to the profile
   */
  void SetSoundSettings();
};

#endif
