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

#ifndef XCSOAR_NMEA_INFO_H
#define XCSOAR_NMEA_INFO_H

#include "DateTime.hpp"
#include "Navigation/GeoPoint.hpp"
#include "Navigation/Aircraft.hpp"
#include "Atmosphere/Pressure.hpp"
#include "FLARM/State.hpp"
#include "Sizes.h"

/**
 * State of external switch devices (esp Vega)
 */
struct SWITCH_INFO
{
  bool AirbrakeLocked;
  bool FlapPositive;
  bool FlapNeutral;
  bool FlapNegative;
  bool GearExtended;
  bool Acknowledge;
  bool Repeat;
  bool SpeedCommand;
  bool UserSwitchUp;
  bool UserSwitchMiddle;
  bool UserSwitchDown;

  enum {
    MODE_UNKNOWN,
    MODE_CIRCLING,
    MODE_CRUISE,
  } FlightMode;

  bool FlapLanding;
  // bool Stall;
};

/**
 * State of GPS fix
 */
struct GPS_STATE
{
  //############
  //   Status
  //############

  /**
   * Is a GPS unit connected?
   *
   * 0 = not connected
   * 1 = connected, waiting for fix
   * 2 = connected, fix found
   */
  unsigned Connected;

  /** GPS fix not valid */
  int NAVWarning;

  /**
   * FixQuality
   * 1 = GPS fix (SPS)
   * 2 = DGPS fix
   * 3 = PPS fix
   * 4 = Real Time Kinematic
   * 5 = Float RTK
   * 6 = estimated (dead reckoning) (2.3 feature)
   * 7 = Manual input mode
   * 8 = Simulation mode
   */
  int FixQuality;

  /** Number of satellites used for gps fix */
  int SatellitesUsed;

  /** GPS Satellite ids */
  int SatelliteIDs[MAXSATELLITES];

  /** Horizontal dilution of precision */
  fixed HDOP;

  /** Is the GPS unit moving? (Speed > 2.0) */
  bool MovementDetected;

  /** Is XCSoar in replay mode? */
  bool Replay;

  /**
   * Did the simulator provide the GPS position?
   */
  bool Simulator;

  /**
   * Adds data from the specified object, unless already present in
   * this one.
   */
  void complement(const GPS_STATE &add) {
    if (add.Connected > Connected)
      *this = add;
  }
};

/**
 * State of acceleration of aircraft, with calculated pseudo-attitude reference
 */
struct ACCELERATION_STATE
{
  //##################
  //   Acceleration
  //##################

  /** Estimated bank angle */
  Angle BankAngle;
  /** Estimated pitch angle */
  Angle PitchAngle;

  /**
   * Is G-load information available?
   * @see Gload
   */
  bool Available;

  /**
   * G-Load information of external device (if available)
   * or estimated (assuming balanced turn) 
   * @see AccelerationAvailable
   */
  fixed Gload;

  /**
   * Adds data from the specified object, unless already present in
   * this one.
   */
  void complement(const ACCELERATION_STATE &add) {
    /* calculated: BankAngle, PitchAngle */

    if (!Available && add.Available) {
      Gload = add.Gload;
      Available = add.Available;
    }
  }
};


/**
 * A struct that holds all the parsed data read from the connected devices
 */
struct NMEA_INFO {

  /**
   * @todo Reset to cleared state
   */
  void reset() {};

  GPS_STATE gps;

  ACCELERATION_STATE acceleration;

  FLYING_STATE flight;

  /** location of aircraft */
  GeoPoint Location;

  /** track angle in degrees true */
  Angle TrackBearing;

  /** Bearing including wind factor */
  Angle Heading;

  /** Turn rate based on heading (including wind) */
  fixed TurnRateWind;

  /** Turn rate based on track */
  fixed TurnRate;

  //############
  //   Speeds
  //############

  /**
   * Is air speed information available?
   * If not, will be estimated from ground speed and wind estimate
   * @see TrueAirspeed in Aircraft
   */
  bool AirspeedAvailable;

  /**
   * Speed over ground in m/s
   * @see TrueAirspeed
   * @see IndicatedAirspeed
   */
  fixed GroundSpeed; 

  /**
   * True air speed (m/s)
   * @see Speed
   * @see IndicatedAirspeed
   */
  fixed TrueAirspeed;

  /**
   * Indicated air speed (m/s)
   * @see Speed
   * @see TrueAirspeed
   * @see AirDensityRatio
   */
  fixed IndicatedAirspeed;

  fixed TrueAirspeedEstimated;

  //##############
  //   Altitude
  //##############

  fixed GPSAltitude; /**< GPS altitude AMSL (m) */

  /**
   * Is a barometric altitude available?
   * @see BaroAltitude
   */
  bool BaroAltitudeAvailable;

  /**
   * Specifies the last-known source for the barometric altitude.
   * This is used to define a priority of sources.
   */
  enum BaroAltitudeOrigin {
    /**
     * Unknown origin or not available.  This is the initial value.
     */
    BARO_ALTITUDE_UNKNOWN,

    /**
     * Parsed from proprietary Garmin sentence "PGRMZ".
     */
    BARO_ALTITUDE_GARMIN,

    /**
     * Parsed from proprietary Garmin sentence "PGRMZ", emitted by
     * a FLARM.
     */
    BARO_ALTITUDE_FLARM,

    /**
     * Parsed from proprietary EW sentence "PGRMZ".
     */
    BARO_ALTITUDE_EW,

    /**
     * Parsed from proprietary Volkslogger sentence "PGCS1".
     */
    BARO_ALTITUDE_VOLKSLOGGER,

    /**
     * Parsed from proprietary Tasman Instruments sentence "PTAS1".
     */
    BARO_ALTITUDE_TASMAN,

    /**
     * Parsed from proprietary Ilec sentence "PDA1".
     */
    BARO_ALTITUDE_ILEC,

    /**
     * Parsed from proprietary Leonardo sentence "C".
     */
    BARO_ALTITUDE_LEONARDO,

    /**
     * Parsed from proprietary Flytec sentence "VMVABD".
     */
    BARO_ALTITUDE_FLYTEC,

    /**
     * Parsed from proprietary Flymaster sentence "VARIO".
     */
    BARO_ALTITUDE_FLYMASTER,

    /**
     * Parsed from proprietary PosiGraph sentence "GPWIN".
     */
    BARO_ALTITUDE_POSIGRAPH,

    /**
     * Parsed from proprietary LX sentence "LXWP0".
     */
    BARO_ALTITUDE_LX,

    /**
     * Parsed from proprietary Triadis sentence "PGRMZ" (Altair Pro).
     */
    BARO_ALTITUDE_TRIADIS_PGRMZ,

    /**
     * Parsed from proprietary Triadis sentence "PDVDV" (Vega).
     */
    BARO_ALTITUDE_TRIADIS_PDVDV,

    /**
     * Parsed from the Zander sentence "PZAN1".
     */
    BARO_ALTITUDE_ZANDER,

    /**
     * Parsed from the Cambridge CAI302 sentence "PCAID".
     */
    BARO_ALTITUDE_CAI302_PCAID,

    /**
     * Parsed from the Cambridge CAI302 sentence "!w".
     */
    BARO_ALTITUDE_CAI302_W,
  } BaroAltitudeOrigin;

  /**
   * Barometric altitude (if available)
   * @see BaroAltitudeAvailable
   * @see Altitude
   */
  fixed BaroAltitude;

  /** Energy height excess to slow to best glide speed @author JMW */
  fixed EnergyHeight;

  /** Nav Altitude + Energy height (m) */
  fixed TEAltitude;

  /** Height above working band/safety (m) */
  fixed working_band_height;

  /** Altitude used for navigation (GPS or Baro) */
  fixed NavAltitude;

  /** Fraction of working band height */
  fixed working_band_fraction;

  /** Altitude over terrain */
  fixed AltitudeAGL;

  /**
   * Troposhere atmosphere model for QNH correction
   */
  AtmosphericPressure pressure;

  //##########
  //   Time
  //##########

  fixed Time; /**< global time (seconds UTC) */

  /** GPS date and time */
  BrokenDateTime DateTime;

  //###########
  //   Vario
  //###########

  fixed GliderSinkRate;

  /** GPS-based vario */
  fixed GPSVario;
  /** GPS-based vario including energy height */
  fixed GPSVarioTE;

  /**
   * Is an external vario signal available?
   * @see TotalEnergyVario
   */
  bool TotalEnergyVarioAvailable;

  /**
   * Is an external netto vario signal available?
   * @see NettoVario
   */
  bool NettoVarioAvailable;

  /**
   * Rate of change of total energy of aircraft (m/s, up positive)
   * @see TotalEnergyVarioAvailable
   */
  fixed TotalEnergyVario;

  /**
   * Vertical speed of air mass (m/s, up positive)
   * @see NettoVarioAvailable
   */
  fixed NettoVario;

  //##############
  //   Settings
  //##############

  /** MacCready value of external device (if available) */
  fixed MacCready;

  /** Ballast information of external device (if available) */
  fixed Ballast;

  /** Bugs information of external device (if available) */
  fixed Bugs;

  //################
  //   Atmosphere
  //################

  /**
   * Is external wind information available?
   * @see ExternalWindSpeed
   * @see ExternalWindDirection
   */
  bool ExternalWindAvailable;

  /**
   * The wind read from the device.  If ExternalWindAvailable is
   * false, then this value is undefined.
   */
  SpeedVector ExternalWind;

  /**
   * The effective wind vector; depending on the settings, this is
   * either ExternalWind, calculated wind or manual wind.
   */
  SpeedVector wind;

  /**
   * Is temperature information available?
   * @see OutsideAirTemperature
   */
  bool TemperatureAvailable;
  /**
   * Temperature of outside air (if available)
   * @see TemperatureAvailable
   */
  fixed OutsideAirTemperature;

  /**
   * Is humidity information available?
   * @see RelativeHumidity
   */
  bool HumidityAvailable;
  /**
   * Humidity of outside air (if available)
   * @see HumidityAvailable
   */
  fixed RelativeHumidity;

  //###########
  //   Other
  //###########

  /** Battery supply voltage information (if available) */
  fixed SupplyBatteryVoltage;

  bool SwitchStateAvailable;

  /** Switch state of the user inputs */
  SWITCH_INFO SwitchState;

  fixed StallRatio;

  FLARM_STATE flarm;

  /**
   * Sets the "true" barometric altitude (i.e. above NN, not above
   * 1013 hPa).
   */
  void SetBaroAltitudeTrue(enum BaroAltitudeOrigin origin, fixed value) {
    BaroAltitude = value;
    BaroAltitudeOrigin = origin;
    BaroAltitudeAvailable = true;
  }

  /**
   * Sets the barometric altitude above 1013 hPa.
   */
  void SetBaroAltitude1013(enum BaroAltitudeOrigin origin, fixed value) {
    SetBaroAltitudeTrue(origin, pressure.AltitudeToQNHAltitude(value));
  }

  /**
   * Sets the barometric altitude from a static pressure sensor.
   */
  void SetStaticPressure(enum BaroAltitudeOrigin origin, fixed value) {
    SetBaroAltitudeTrue(origin, pressure.StaticPressureToQNHAltitude(value));
  }

  /**
   * Provide a "true" barometric altitude, but only use it if the
   * previous altitude was not present or the same/lower priority.
   */
  void ProvideBaroAltitudeTrue(enum BaroAltitudeOrigin origin, fixed value) {
    if (!BaroAltitudeAvailable || BaroAltitudeOrigin <= origin)
      SetBaroAltitudeTrue(origin, value);
  }

  /**
   * Provide barometric altitude above 1013 hPa, but only use it if
   * the previous altitude was not present or the same/lower priority.
   */
  void ProvideBaroAltitude1013(enum BaroAltitudeOrigin origin, fixed value) {
    if (!BaroAltitudeAvailable || BaroAltitudeOrigin <= origin)
      SetBaroAltitude1013(origin, value);
  }

  /**
   * Provide barometric altitude from a static pressure sensor, but
   * only use it if the previous altitude was not present or the
   * same/lower priority.
   */
  void ProvideStaticPressure(enum BaroAltitudeOrigin origin, fixed value) {
    if (!BaroAltitudeAvailable || BaroAltitudeOrigin <= origin)
      SetStaticPressure(origin, value);
  }

  /**
   * Returns the barometric altitude, and falls back to the GPS
   * altitude.
   */
  fixed GetAltitudeBaroPreferred() const {
    return BaroAltitudeAvailable
      ? BaroAltitude
      : GPSAltitude;
  }

  /**
   * Adds data from the specified object, unless already present in
   * this one.
   *
   * Note that this does not copy calculated values which are managed
   * outside of the NMEA parser.
   */
  void complement(const NMEA_INFO &add) {
    gps.complement(add.gps);

    acceleration.complement(add.acceleration);

    /* calculated: flight */

    if (add.gps.Connected > gps.Connected) {
      gps = add.gps;
      Location = add.Location;
      TrackBearing = add.TrackBearing;
      GroundSpeed = add.GroundSpeed;
      GPSAltitude = add.GPSAltitude;
      Time = add.Time;
      DateTime = add.DateTime;
    }

    if (!AirspeedAvailable && add.AirspeedAvailable) {
      TrueAirspeed = add.TrueAirspeed;
      IndicatedAirspeed = add.IndicatedAirspeed;
      AirspeedAvailable = true;
    }

    /* calculated: Heading, TurnRateWind, TurnRate */
    /* calculated: TrueAirspeedEstimated */

    if ((!BaroAltitudeAvailable ||
         (BaroAltitudeOrigin <= BARO_ALTITUDE_UNKNOWN &&
          add.BaroAltitudeOrigin > BaroAltitudeOrigin)) &&
        add.BaroAltitudeAvailable) {
      BaroAltitude = add.BaroAltitude;
      BaroAltitudeOrigin = add.BaroAltitudeOrigin;
      BaroAltitudeAvailable = true;
    }

    /* calculated: EnergyHeight, TEAltitude, working_band_height,
       NavAltitude,working_band_fraction, AltitudeAGL */

    /* managed by DeviceBlackboard: pressure */

    /* calculated: GliderSinkRate, GPSVario, GPSVarioTE */

    if (!TotalEnergyVarioAvailable && add.TotalEnergyVarioAvailable) {
      TotalEnergyVario = add.TotalEnergyVario;
      TotalEnergyVarioAvailable = add.TotalEnergyVarioAvailable;
    }

    if (!NettoVarioAvailable && add.NettoVarioAvailable) {
      NettoVario = add.NettoVario;
      NettoVarioAvailable = add.NettoVarioAvailable;
    }

    // XXX MacCready, Ballast, Bugs

    if (!ExternalWindAvailable && add.ExternalWindAvailable) {
      ExternalWind = add.ExternalWind;
      ExternalWindAvailable = add.ExternalWindAvailable;
    }

    if (!TemperatureAvailable && add.TemperatureAvailable) {
      OutsideAirTemperature = add.OutsideAirTemperature;
      TemperatureAvailable = add.TemperatureAvailable;
    }

    if (!HumidityAvailable && add.HumidityAvailable) {
      RelativeHumidity = add.RelativeHumidity;
      HumidityAvailable = add.HumidityAvailable;
    }

    if (!positive(SupplyBatteryVoltage) && positive(add.SupplyBatteryVoltage))
      SupplyBatteryVoltage = add.SupplyBatteryVoltage;

    if (!SwitchStateAvailable && add.SwitchStateAvailable)
      SwitchState = add.SwitchState;

    flarm.complement(add.flarm);
  }
};

static inline const AIRCRAFT_STATE
ToAircraftState(const NMEA_INFO &info)
{
  AIRCRAFT_STATE aircraft;

  /* SPEED_STATE */
  aircraft.Speed = info.GroundSpeed;
  aircraft.TrueAirspeed = info.TrueAirspeed;
  aircraft.IndicatedAirspeed = info.IndicatedAirspeed;

  /* ALTITUDE_STATE */
  aircraft.NavAltitude = info.NavAltitude;
  aircraft.working_band_fraction = info.working_band_fraction;
  aircraft.AltitudeAGL = info.AltitudeAGL;
  if (info.BaroAltitudeAvailable) {
    aircraft.AirspaceAltitude = info.BaroAltitude;
  } else {
    aircraft.AirspaceAltitude = info.GPSAltitude;
  }

  /* VARIO_INFO */
  aircraft.Vario = info.TotalEnergyVario;
  aircraft.NettoVario = info.NettoVario;

  /* FLYING_STATE */
  (FLYING_STATE &)aircraft = info.flight;

  /* AIRCRAFT_STATE */
  aircraft.Time = info.Time;
  aircraft.Location = info.Location;
  aircraft.TrackBearing = info.TrackBearing;
  aircraft.Gload = info.acceleration.Gload;
  aircraft.wind = info.wind;

  return aircraft;
}

#endif
