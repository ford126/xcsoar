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
#ifndef FlatGeoPoint_HPP
#define FlatGeoPoint_HPP

#include "Math/FastMath.h"
#include "Compiler.h"

/**
 * Integer projected (flat-earth) version of Geodetic coordinates
 */
struct FlatGeoPoint {
/** 
 * Constructor (default) at origin
 * 
 * @return Initialised object at origin
 */
  FlatGeoPoint():Longitude(0),Latitude(0) {};

/** 
 * Constructor at specified location (x,y)
 * 
 * @param x x location
 * @param y y location
 *
 * @return Initialised object at origin
 */
  FlatGeoPoint(const int x,
                const int y):
    Longitude(x),Latitude(y) {};

  int Longitude; /**< Projected x (Longitude) value (undefined units) */
  int Latitude; /**< Projected y (Latitude) value (undefined units) */

/** 
 * Find distance from one point to another
 * 
 * @param sp That point
 * 
 * @return Distance in projected units
 */
  gcc_pure
  unsigned distance_to(const FlatGeoPoint &sp) const;

/** 
 * Find squared distance from one point to another
 * 
 * @param sp That point
 * 
 * @return Squared distance in projected units
 */
  gcc_pure
  unsigned distance_sq_to(const FlatGeoPoint &sp) const;

/** 
 * Add one point to another
 * 
 * @param p2 Point to add
 * 
 * @return Added value
 */
  gcc_pure
  FlatGeoPoint operator+ (const FlatGeoPoint &p2) const {
    FlatGeoPoint res= *this;
    res.Longitude += p2.Longitude;
    res.Latitude += p2.Latitude;
    return res;
  };

/** 
 * Subtract one point from another
 * 
 * @param p2 Point to subtract
 * 
 * @return Subtracted value
 */
  gcc_pure
  FlatGeoPoint operator- (const FlatGeoPoint &p2) const {
    FlatGeoPoint res= *this;
    res.Longitude -= p2.Longitude;
    res.Latitude -= p2.Latitude;
    return res;
  };

/** 
 * Multiply point by a constant
 * 
 * @param t Value to multiply
 * 
 * @return Scaled value
 */
  gcc_pure
  FlatGeoPoint operator* (const fixed t) const {
    FlatGeoPoint res= *this;
    res.Longitude = (int)(res.Longitude*t);
    res.Latitude = (int)(res.Latitude*t);
    return res;
  };

/** 
 * Calculate cross product of one point with another
 * 
 * @param other That point
 * 
 * @return Cross product
 */
  gcc_pure
  int cross(const FlatGeoPoint &other) const {
    return Longitude*other.Latitude-Latitude*other.Longitude;
  }

/** 
 * Calculate dot product of one point with another
 * 
 * @param other That point
 * 
 * @return Dot product
 */
  gcc_pure
  int dot(const FlatGeoPoint &other) const {
    return Longitude*other.Longitude+Latitude*other.Latitude;
  }

/** 
 * Test whether two points are co-located
 * 
 * @param other Point to compare
 * 
 * @return True if coincident
 */
  gcc_pure
  bool operator== (const FlatGeoPoint &other) const {
    return (Longitude == other.Longitude) && (Latitude == other.Latitude);
  };
};

#endif
