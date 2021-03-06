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

#include "ClimbAverageCalculator.hpp"

ClimbAverageCalculator::ClimbAverageCalculator()
{
	newestValIndex = -1;

	for (int i = 0; i < MAX_HISTORY; i++) {
    history[i].altitude = fixed(-99999);
    history[i].time = fixed(-99999);
	}
}

fixed
ClimbAverageCalculator::GetAverage(fixed curTime,
                                   fixed curAltitude,
                                   fixed averageTime)
{
  int bestHistory;

  newestValIndex = newestValIndex < MAX_HISTORY - 1 ? newestValIndex + 1 : 0;

  // add the new sample
  history[newestValIndex].time = curTime;
  history[newestValIndex].altitude = curAltitude;

  // initially bestHistory is the current...
  bestHistory = newestValIndex;

  // now run through the history and find the best sample for average period within the average time period
  for (int i = 0; i < MAX_HISTORY; i++) {
    if (history[i].time == fixed(-99999))
      continue;

    // inside the period ?
    if (history[i].time + averageTime < curTime)
      continue;

    // is the sample older (and therefore better) than the current found ?
    if (history[i].time < history[bestHistory].time)
      bestHistory = i;
  }

  // calculate the average !
  if (bestHistory != newestValIndex)
    return (curAltitude - history[bestHistory].altitude) /
           (curTime - history[bestHistory].time);

  return fixed_zero;
}
