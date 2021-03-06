/* Copyright_License {

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

#include "Replay/IgcReplay.hpp"

#include "Engine/Trace/Trace.hpp"
#include "Engine/Task/TaskStats/CommonStats.hpp"
#include "Engine/Task/Tasks/ContestManager.hpp"
#include "Engine/Navigation/Aircraft.hpp"
#include "Printing.hpp"

#include <windef.h>
#include <assert.h>
#include <cstdio>

Trace full_trace;
Trace sprint_trace(9000, 2, 300);
unsigned handicap = 100;

ContestResult stats_classic;
ContestManager olc_classic(OLC_Classic, handicap, 
                           stats_classic,
                           full_trace, sprint_trace);

ContestResult stats_fai;
ContestManager olc_fai(OLC_FAI, handicap, stats_fai,
                       full_trace, sprint_trace);

ContestResult stats_sprint;
ContestManager olc_sprint(OLC_Sprint, handicap, 
                          stats_sprint,
                          full_trace, sprint_trace);

ContestResult stats_league;
ContestManager olc_league(OLC_League, handicap, 
                          stats_league,
                          full_trace, sprint_trace);

ContestResult stats_plus;
ContestManager olc_plus(OLC_Plus, handicap, 
                        stats_plus,
                        full_trace, sprint_trace);

class IgcReplayGlue:
  public IgcReplay
{
public:
  bool error;

  IgcReplayGlue() : error(false) {}

  void SetFilename(const char *name);

protected:
  virtual bool update_time();
  virtual void reset_time();
  virtual void on_reset() {}
  virtual void on_stop() {}
  virtual void on_bad_file() { error = true; }
  virtual void on_advance(const GeoPoint &loc, const fixed speed,
                          const Angle bearing, const fixed alt,
                          const fixed baroalt, const fixed t);
};

bool
IgcReplayGlue::update_time()
{
  t_simulation += fixed_one;
  t_simulation = std::max(GetMinTime(), t_simulation);
  return true;
}

void
IgcReplayGlue::reset_time()
{
  t_simulation = fixed_zero;
}

void
IgcReplayGlue::on_advance(const GeoPoint &loc, const fixed speed,
                          const Angle bearing, const fixed alt,
                          const fixed baroalt, const fixed t)
{
  AIRCRAFT_STATE new_state;
  new_state.Location = loc;
  new_state.Speed = speed;
  new_state.NavAltitude = alt;
  new_state.TrackBearing = bearing;
  new_state.Time = t;
  new_state.AltitudeAGL = alt;

  full_trace.append(new_state);
  sprint_trace.append(new_state);

  full_trace.optimise_if_old();
  sprint_trace.optimise_if_old();
}

void
IgcReplayGlue::SetFilename(const char *name)
{
#ifdef _UNICODE
  TCHAR tname[MAX_PATH];
  int length = ::MultiByteToWideChar(CP_ACP, 0, name, -1, tname, MAX_PATH);
  if (length == 0)
    return;

  IgcReplay::SetFilename(tname);
#else
  IgcReplay::SetFilename(name);
#endif
}

static void
TestOLC(const char *filename)
{
  IgcReplayGlue replay;
  replay.SetFilename(filename);
  replay.Start();
  assert(!replay.error);

  for (int i = 1; replay.Update(); i++) {
    if (i % 500 == 0) {
      putchar('.');
      fflush(stdout);
    }

    olc_classic.update_idle();
    olc_fai.update_idle();
    olc_sprint.update_idle();
    olc_league.update_idle();
    olc_plus.update_idle();
  }

  putchar('\n');

  std::cout << "classic\n";
  PrintHelper::print(stats_classic);
  std::cout << "league\n";
  PrintHelper::print(stats_league);
  std::cout << "fai\n";
  PrintHelper::print(stats_fai);
  std::cout << "sprint\n";
  PrintHelper::print(stats_sprint);
  std::cout << "plus\n";
  PrintHelper::print(stats_plus);
}


int main(int argc, char **argv)
{
  assert(argc >= 2);

  TestOLC(argv[1]);

  return 0;
}
