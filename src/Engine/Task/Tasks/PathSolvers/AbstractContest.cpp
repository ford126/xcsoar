/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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

#include "ContestDijkstra.hpp"
#include "Task/TaskStats/ContestResult.hpp"
#include "Trace/Trace.hpp"


AbstractContest::AbstractContest(const Trace &_trace,
                                 const unsigned &_handicap,
                                 const unsigned finish_alt_diff):
  trace_master(_trace),
  contest_handicap(_handicap),
  m_finish_alt_diff(finish_alt_diff),
  best_result()
{
  reset();
}


void
AbstractContest::reset()
{
  best_result.reset();
}

bool
AbstractContest::score(ContestResult &result)
{
  if (positive(calc_time())) {
    result = best_result;
    return true;
  }
  return false;
}


bool 
AbstractContest::update_score()
{
  // for normal contests, nothing needs to be done
  return false;
}


bool
AbstractContest::save_solution()
{
  const fixed score = calc_score();
  const bool improved = (score>best_result.score);

  if (improved) {
    best_result.score = score;
    best_result.distance = calc_distance();
    best_result.time = calc_time();
    if (positive(best_result.time))
      best_result.speed = best_result.distance / best_result.time;
    else
      best_result.speed = fixed_zero;
    return true;
  }
  return false;
}

bool
AbstractContest::finish_altitude_valid(const TracePoint& start,
                                       const TracePoint& finish) const
{
  return (finish.NavAltitude + fixed(m_finish_alt_diff) >=
          start.NavAltitude);
}

fixed 
AbstractContest::apply_handicap(const fixed& unhandicapped_score,
                                const bool shifted) const
{
  if (shifted) {
    return (200*unhandicapped_score/(100+contest_handicap));
  } else {
    return (100*unhandicapped_score/contest_handicap);
  }
}