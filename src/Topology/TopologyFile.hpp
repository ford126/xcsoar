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

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "shapelib/mapshape.h"
#include "Geo/GeoBounds.hpp"
#include "Util/NonCopyable.hpp"
#include "Util/AllocatedArray.hpp"
#include "Math/fixed.hpp"
#include "Screen/Color.hpp"

struct GeoPoint;
class Canvas;
class WindowProjection;
struct GeoBounds;
class LabelBlock;
struct SETTINGS_MAP;
class XShape;
struct zzip_dir;

class TopologyFile : private NonCopyable {
  struct zzip_dir *dir;

  AllocatedArray<XShape *> shpCache;

  int label_field, icon;

  Color color;

public:
  /**
   * The constructor opens the given shapefile and clears the cache
   * @param shpname The shapefile to open (*.shp)
   * @param threshold the zoom threshold for displaying this object
   * @param thecolor The color to use for drawing
   * @param label_field The field in which the labels should be searched
   * @param icon the resource id of the icon, 0 for no icon
   * @return
   */
  TopologyFile(struct zzip_dir *dir, const char *shpname,
               fixed threshold, const Color color,
               int label_field=-1, int icon=0);

  /**
   * The destructor clears the cache and closes the shapefile
   */
  ~TopologyFile();

  bool is_visible(fixed map_scale) const {
    return map_scale <= scaleThreshold;
  }

  int get_label_field() const {
    return label_field;
  }

  int get_icon() const {
    return icon;
  }

  Color get_color() const {
    return color;
  }

  bool empty() const {
    return shpCache.size() == 0;
  }

  unsigned size() const {
    return shpCache.size();
  }

  const XShape *operator[](unsigned i) const {
    return shpCache[i];
  }

  gcc_pure
  unsigned GetSkipSteps(fixed map_scale) const;

  /**
   * @return true if new data from the topology file has been loaded
   */
  bool updateCache(const WindowProjection &map_projection);

  /**
   * The threshold value for the visibility check. If the current scale
   * is below this value the contents of this TopologyFile will be drawn.
   */
  fixed scaleThreshold;

private:
  /**
   * The current scope of the shape cache.  If the screen exceeds this
   * rectangle, then we need to update the cache.
   */
  GeoBounds cache_bounds;

  static rectObj ConvertRect(const GeoBounds &br);

protected:
  void ClearCache();

  shapefileObj shpfile;
  bool shapefileopen;
};

#endif
