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

#include "GlidePolar.hpp"
#include "GlideState.hpp"
#include "GlideResult.hpp"
#include "MacCready.hpp"
#include "Util/ZeroFinder.hpp"
#include "Util/Tolerances.hpp"
#include "Navigation/Aircraft.hpp"
#include "Util/Quadratic.hpp"
#include <assert.h>

#define fixed_75 fixed(75.0)

GlidePolar::GlidePolar(const fixed _mc, const fixed _bugs, const fixed _ballast) :
  mc(_mc),
  bugs(_bugs),
  ballast(_ballast),
  cruise_efficiency(fixed_one),
  VbestLD(fixed_zero),
  Vmax(fixed_75),
  Vmin(fixed_zero),
  ideal_polar_a(0.00157),
  ideal_polar_b(-0.0734),
  ideal_polar_c(1.48),
  ballast_ratio(0.3),
  empty_mass(300),
  wing_area(fixed_zero)
{
  update();
}

void
GlidePolar::update()
{
  update_polar();
  Smax = SinkRate(Vmax);
  solve_min();
  set_mc(mc);
}

void
GlidePolar::update_polar()
{
  assert(positive(bugs));

  const fixed loading_factor = sqrt(fixed_one+ballast*ballast_ratio);
  const fixed inv_bugs = fixed_one/bugs;

  polar_a = inv_bugs * ideal_polar_a / loading_factor;
  polar_b = inv_bugs * ideal_polar_b;
  polar_c = inv_bugs * ideal_polar_c * loading_factor;

  assert(positive(polar_a));
  assert(negative(polar_b));
  assert(positive(polar_c));
}

void
GlidePolar::set_bugs(const fixed clean)
{
  bugs = clean;
  update();
}

void
GlidePolar::set_ballast(const fixed bal)
{
  ballast = bal;
  update();
}

void
GlidePolar::set_mc(const fixed _mc)
{
  mc = _mc;

  if (positive(mc))
    inv_mc = fixed_one/mc;
  else
    inv_mc = fixed_zero;

  solve_ld();
}

fixed
GlidePolar::MSinkRate(const fixed V) const
{
  return SinkRate(V) + mc;
}

fixed
GlidePolar::SinkRate(const fixed V) const
{
  return V * (V * polar_a + polar_b) + polar_c;
}

fixed
GlidePolar::SinkRate(const fixed V, const fixed n) const
{
  const fixed w0 = SinkRate(V);
  const fixed vl = VbestLD / max(VbestLD * fixed_half, V);
  return max(fixed_zero,
             w0 + (V / (fixed_two * bestLD)) * (n * n - fixed_one) * vl * vl);
}

/**
 * Finds VOpt for a given MacCready setting
 * Intended to be used temporarily.
 */
class GlidePolarVopt: public ZeroFinder
{
public:
  /**
   * Constructor.
   *
   * @param _polar Glide polar to optimise
   * @param vmin Minimum speed to search (m/s)
   * @param vmax Maximum speed to search (m/s)
   *
   * @return Initialised object (no search yet)
   */
  GlidePolarVopt(const GlidePolar &_polar, const fixed& vmin, const fixed &vmax):
    ZeroFinder(vmin, vmax, fixed(TOLERANCE_POLAR_BESTLD)),
    polar(_polar)
  {
  }

  /**
   * Glide ratio function
   *
   * @param V Speed (m/s)
   *
   * @return MacCready-adjusted inverse glide ratio
   */
  fixed
  f(const fixed V)
  {
    return polar.MSinkRate(V) / V;
  }

private:
  const GlidePolar &polar;
};

void
GlidePolar::solve_ld()
{
/*
  // this method to be used if polar is not parabolic
  GlidePolarVopt gpvopt(*this, Vmin, Vmax);
  VbestLD = gpvopt.find_min(VbestLD);
*/
  assert(!negative(mc));
  VbestLD = max(Vmin, min(Vmax, sqrt((polar_c+mc)/polar_a)));
  SbestLD = SinkRate(VbestLD);
  bestLD = VbestLD / SbestLD;
}

/**
 * Finds min sink speed.
 * Intended to be used temporarily.
 */
class GlidePolarMinSink: public ZeroFinder
{
public:
  /**
   * Constructor.
   *
   * @param _polar Glide polar to optimise
   * @param vmax Maximum speed to search (m/s)
   *
   * @return Initialised object (no search yet)
   */
  GlidePolarMinSink(const GlidePolar &_polar, const fixed &vmax):
    ZeroFinder(fixed_one, vmax, fixed(TOLERANCE_POLAR_MINSINK)),
    polar(_polar)
  {
  }

  fixed
  f(const fixed V)
  {
    return polar.SinkRate(V);
  }

private:
  const GlidePolar &polar;
};

void 
GlidePolar::solve_min()
{
/*
  // this method to be used if polar is not parabolic
  GlidePolarMinSink gpminsink(*this, Vmax);
  Vmin = gpminsink.find_min(Vmin);
*/
  Vmin = min(Vmax, -polar_b/(fixed_two*polar_a));
  Smin = SinkRate(Vmin);
}

// global, used for test harness
long count_mc = 0;

GlideResult
GlidePolar::solve(const GlideState &task) const
{
#ifdef INSTRUMENT_TASK
  count_mc++;
#endif
  MacCready mac(*this, cruise_efficiency);
  return mac.solve(task);
}

GlideResult
GlidePolar::solve_sink(const GlideState &task, const fixed S) const
{
#ifdef INSTRUMENT_TASK
  count_mc++;
#endif
  MacCready mac(*this, cruise_efficiency);
  return mac.solve_sink(task,S);
}

bool 
GlidePolar::possible_glide(const GlideState &task) const
{
  if (!positive(task.AltitudeDifference))
    return false;

  // broad test assuming tailwind at best LD (best case)
  if ((VbestLD + task.EffectiveWindSpeed) * task.AltitudeDifference
      < task.Vector.Distance * SbestLD)
    return false;

  return true;
}

/**
 * Finds speed to fly for a given MacCready setting
 * Intended to be used temporarily.
 *
 * This finds the speed that maximises the glide angle over the ground
 */
class GlidePolarSpeedToFly: public ZeroFinder
{
public:
  /**
   * Constructor.
   *
   * @param _polar Glide polar to optimise
   * @param net_sink_rate Instantaneous netto sink rate (m/s), positive down
   * @param head_wind Head wind component (m/s)
   * @param vmin Minimum speed to search (m/s)
   * @param vmax Maximum speed to search (m/s)
   *
   * @return Initialised object (no search yet)
   */
  GlidePolarSpeedToFly(const GlidePolar &_polar, const fixed& net_sink_rate,
      const fixed& head_wind, const fixed& vmin, const fixed &vmax) :
    ZeroFinder(max(fixed_one, vmin - head_wind), vmax - head_wind,
               fixed(TOLERANCE_POLAR_DOLPHIN)),
    polar(_polar),
    m_net_sink_rate(net_sink_rate),
    m_head_wind(head_wind)
  {
  }

  /**
   * Glide ratio function
   *
   * @param V Speed over ground (m/s)
   *
   * @return MacCready-adjusted inverse glide ratio
   */
  fixed
  f(const fixed V)
  {
    return (polar.MSinkRate(V + m_head_wind) + m_net_sink_rate) / V;
  }

  /**
   * Find best speed to fly
   *
   * @param Vstart Initial search speed (m/s)
   *
   * @return Speed to fly (m/s)
   */
  fixed
  solve(const fixed Vstart)
  {
    fixed Vopt = find_min(Vstart);
    return Vopt + m_head_wind;
  }

private:
  const GlidePolar &polar;
  const fixed& m_net_sink_rate;
  const fixed& m_head_wind;
};

fixed
GlidePolar::speed_to_fly(const AIRCRAFT_STATE &state,
    const GlideResult &solution, const bool block_stf) const
{
  fixed V_stf;
  const fixed g_scaling (block_stf ? fixed_one : sqrt(fabs(state.Gload))); 

  if (!block_stf && (state.NettoVario > mc + Smin)) {
    // stop to climb
    V_stf = Vmin;
  } else {
    const fixed head_wind (solution.is_final_glide() ? solution.HeadWind : fixed_zero);
    const fixed stf_sink_rate (block_stf ? fixed_zero : -state.NettoVario);

    GlidePolarSpeedToFly gp_stf(*this, stf_sink_rate, head_wind, Vmin, Vmax);
    V_stf = gp_stf.solve(Vmax);
  }

  return max(Vmin, V_stf*g_scaling);
}

fixed
GlidePolar::get_all_up_weight() const
{
  return empty_mass + get_ballast_litres();
}

fixed
GlidePolar::get_wing_loading() const
{
  if (positive(wing_area))
    return get_all_up_weight() / wing_area;

  return fixed_zero;
}

fixed
GlidePolar::get_ballast_litres() const
{
  return ballast * ballast_ratio * empty_mass;
}

bool
GlidePolar::is_ballastable() const
{
  return positive(ballast_ratio);
}

static fixed
FRiskFunction(const fixed x, const fixed k)
{
  return fixed_two / (fixed_one + exp(-x * k)) - fixed_one;
}

fixed
GlidePolar::mc_risk(const fixed height_fraction, const fixed riskGamma) const
{
#define fixed_low_limit fixed(0.1)
#define fixed_up_limit fixed(0.9)

  fixed x = max(fixed_zero, min(fixed_one, height_fraction));

  if (riskGamma < fixed_low_limit)
    return mc;
  else if (riskGamma > fixed_up_limit)
    return x * mc;

  const fixed k = fixed_one / (riskGamma * riskGamma) - fixed_one;
  return mc * FRiskFunction(x, k) / FRiskFunction(fixed_one, k);
}

fixed
GlidePolar::get_Vtakeoff() const
{
  return fixed_half * get_Vmin();
}

fixed
GlidePolar::get_ld_over_ground(const AIRCRAFT_STATE &state) const
{
  if (state.wind.is_zero())
    return bestLD;

  const fixed c_theta = (state.wind.bearing - state.TrackBearing).cos();

  Quadratic q(-fixed_two * state.wind.norm * c_theta,
              state.wind.norm * state.wind.norm - bestLD * bestLD);

  if (q.check())
    return max(fixed_zero, q.solution_max());

  return fixed_zero;
}

fixed 
GlidePolar::get_V_for_sinkrate(const fixed S) const
{
  /// @todo unimplemented
  return VbestLD;
}
