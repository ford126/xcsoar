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

#include "harness_flight.hpp"
#include "harness_airspace.hpp"
#include "TaskEventsPrint.hpp"
#include "Util/AircraftStateFilter.hpp"
#include <fstream>

Airspaces *airspaces = NULL;
AirspaceWarningManager *airspace_warnings = NULL;
AircraftStateFilter *aircraft_filter = NULL;

double time_elapsed = 0.0;
double time_planned = 1.0;
double time_remaining = 0;
double calc_cruise_efficiency = 1.0;
double calc_effective_mc = 1.0;

fixed
aat_min_time(int test_num)
{
  OrderedTaskBehaviour beh;
  switch (test_num) {
  case 2:
    return fixed(3600 * 3.8);
  default:
    return beh.aat_min_time;
  }
}

static fixed
wind_to_mag(int n_wind)
{
  if (n_wind)
    return (fixed(n_wind - 1) / 4 + fixed_one) * 5;

  return fixed_zero;
}

static Angle
wind_to_dir(int n_wind)
{
  if (n_wind)
    return Angle::degrees(fixed(90 * ((n_wind - 1) % 4))).as_bearing();

  return Angle::native(fixed(0));
}

const char*
wind_name(int n_wind)
{
  static char buffer[80];
  sprintf(buffer,"%d m/s @ %d", (int)wind_to_mag(n_wind),
          (int)wind_to_dir(n_wind).value_degrees());
  return buffer;
}


bool run_flight(TaskManager &task_manager,
                const bool goto_target,
                const AutopilotParameters &parms,
                const int n_wind,
                const double speed_factor) 
{
  AircraftSim ac(0, task_manager, parms, goto_target);
  unsigned print_counter=0;
  if (n_wind)
    ac.set_wind(wind_to_mag(n_wind), wind_to_dir(n_wind));

  ac.set_speed_factor(fixed(speed_factor));

  std::ofstream f4("results/res-sample.txt");
  std::ofstream f5("results/res-sample-filtered.txt");

  bool do_print = verbose;
  bool first = true;

  time_elapsed = 0.0;
  time_planned = 1.0;
  time_remaining = 0;
  calc_cruise_efficiency = 1.0;
  calc_effective_mc = 1.0;

  static const fixed fixed_10(10);

  AirspaceAircraftPerformanceGlide perf(task_manager.get_glide_polar());

  if (aircraft_filter)
    aircraft_filter->reset(ac.get_state());

  if (airspaces) {
    airspace_warnings =
        new AirspaceWarningManager(*airspaces, task_manager);
    airspace_warnings->reset(ac.get_state());
  }

  do {
    if ((task_manager.getActiveTaskPointIndex() == 1) &&
        first && (task_manager.get_stats().total.TimeElapsed > fixed_10)) {
      time_remaining = task_manager.get_stats().total.TimeRemaining;
      first = false;

      time_planned = task_manager.get_stats().total.TimePlanned;

      if (verbose > 1) {
        printf("# time remaining %g\n", time_remaining);
        printf("# time planned %g\n", time_planned);
      }
    }

    if (do_print) {
      PrintHelper::taskmanager_print(task_manager, ac.get_state());
      ac.print(f4);
      f4.flush();
      if (aircraft_filter) {
        f5 << aircraft_filter->get_speed() << " " 
           << aircraft_filter->get_bearing() << " " 
           << aircraft_filter->get_climb_rate() << "\n"; 
        f5.flush();
      }
    }

    if (airspaces) {
      scan_airspaces(ac.get_state(), *airspaces, perf,
                     do_print, 
                     ac.target());
    }
    if (airspace_warnings) {
      if (verbose > 1) {
        bool warnings_updated = airspace_warnings->update(ac.get_state(), 
                                                          false);
        if (warnings_updated) {
          printf("# airspace warnings updated, size %d\n",
                 (int)airspace_warnings->size());
          print_warnings();
          wait_prompt(ac.get_state().Time);
        }
      }
    }

    n_samples++;

    do_print = (++print_counter % output_skip == 0) && verbose;

    if (aircraft_filter)
      aircraft_filter->update(ac.get_state());

  } while (ac.Update());

  if (verbose) {
    PrintHelper::taskmanager_print(task_manager, ac.get_state());
    ac.print(f4);
    f4 << "\n";
    f4.flush();
    task_report(task_manager, "end of task\n");
  }
  wait_prompt(0);

  time_elapsed = task_manager.get_stats().total.TimeElapsed;
  time_planned = task_manager.get_stats().total.TimePlanned;
  calc_cruise_efficiency = task_manager.get_stats().cruise_efficiency;
  calc_effective_mc = task_manager.get_stats().effective_mc;

  if (verbose)
    distance_counts();

  if (airspace_warnings)
    delete airspace_warnings;

  return true;
}


bool
test_flight(int test_num, int n_wind, const double speed_factor,
            const bool auto_mc)
{
  // multipurpose flight test

  GlidePolar glide_polar(fixed_two);
  Waypoints waypoints;
  setup_waypoints(waypoints);

  if (verbose)
    distance_counts();

  TaskEventsPrint default_events(verbose);
  TaskManager task_manager(default_events, waypoints);
  task_manager.set_glide_polar(glide_polar);

  TaskBehaviour& task_behaviour = task_manager.get_task_behaviour();

  task_manager.get_ordered_task_behaviour().aat_min_time = aat_min_time(test_num);

  task_behaviour.enable_trace = false;
  task_behaviour.auto_mc = auto_mc;
  task_behaviour.calc_glide_required = false;
  if ((test_num == 0) || (test_num == 2))
    task_behaviour.optimise_targets_bearing = false;

  bool goto_target = false;

  switch (test_num) {
  case 0:
  case 2:
  case 7:
    goto_target = true;
    break;
  };

  test_task(task_manager, waypoints, test_num);

  waypoints.clear(); // clear waypoints so abort wont do anything

  return run_flight(task_manager, goto_target, autopilot_parms, n_wind,
                    speed_factor);
}

bool
test_flight_times(int test_num, int n_wind)
{
  // tests whether elapsed/planned times are consistent
  // there will be small error due to start location

  bool fine = test_flight(test_num, n_wind);
  const double t_rat = fabs(time_elapsed / time_planned - 1.0);
  fine &= t_rat < 0.02;

  if ((verbose) || !fine) {
    printf("# time remaining %g\n", time_remaining);
    printf("# time elapsed %g\n", time_elapsed);
    printf("# time planned %g\n", time_planned);
    printf("# time ratio error (elapsed/planned) %g\n", t_rat);
  }

  return fine;
}
