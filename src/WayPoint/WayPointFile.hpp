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


#ifndef WAYPOINTFILE_HPP
#define WAYPOINTFILE_HPP

#include "Engine/Waypoint/Waypoint.hpp"

#include <tchar.h>
class Waypoints;
class RasterTerrain;
class TextWriter;

class WayPointFile 
{
protected:
  WayPointFile(const TCHAR* file_name, const int _file_num,
               const bool _compressed=false);

public:
  /**
   * Creates a file according to the extension
   * @param filename The filename (will be converted to filepath later)
   * @param filenum Index of file
   * @return File type if extension is known to XCSoar
   */
  static WayPointFile* create(const TCHAR* filename,
                              int filenum=0);

  /**
   * Parses the waypoint file provided by SetFile() into the given waypoint list
   * @param way_points The waypoint list to fill
   * @param terrain RasterTerrain (for automatic waypoint height)
   * @return True if the waypoint file parsing was okay, False otherwise
   */
  bool Parse(Waypoints &way_points, const RasterTerrain *terrain);

  /**
   * Saves the given waypoint list into the waypoint file provided by SetFile()
   * @param way_points The waypoint list to save
   * @return True if saving was successful, False otherwise
   */
  bool Save(const Waypoints &way_points);

  virtual bool IsWritable() = 0;

  static short AltitudeFromTerrain(GeoPoint &location,
                                   const RasterTerrain &terrain);

protected:
  TCHAR file[255];
  const int file_num;
  const bool compressed;

  void check_altitude(Waypoint &new_waypoint, 
                      const RasterTerrain *terrain,
                      bool alt_ok);

  void add_waypoint(Waypoints &way_points, const Waypoint &new_waypoint);

  /**
   * Parse a file line
   * @param line The line to parse
   * @param linenum The line number in the file
   * @param way_points The waypoint list to fill
   * @param terrain RasterTerrain (for automatic waypoint height)
   * @return True if the line was parsed correctly or ignored, False if
   * parsing error occured
   */
  virtual bool parseLine(const TCHAR* line, unsigned linenum,
                         Waypoints &way_points, const RasterTerrain *terrain) = 0;

  virtual void saveFile(TextWriter &writer, const Waypoints &way_points) {};

  // Helper functions

  /**
   * Split line (comma separated fields) in individual fields.
   * @param src The source line of comma separated fields
   * @param dst Destination buffer containing processed '\0' separated fields.
   * @param arr Array of pointers pointing to individual fields of dst
   * @param quote_char Optional character used for quoting of individual fields.
   * @return number of fields returned. Note: an empty src returns 1 for
   * for consistency (i.e. "" -> 1, "," -> 2)
   */
  static size_t extractParameters(const TCHAR *src, TCHAR *dst,
                                  const TCHAR **arr, size_t sz,
                                  const TCHAR quote_char = _T('\0'));

  friend void TestExtractParameters();
};

#endif
