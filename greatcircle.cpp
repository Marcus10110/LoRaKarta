
#include <Arduino.h>
#include <math.h>

#include "greatcircle.h"

#define DEGREES_IN_RADIAN 57.29577951
#define MEAN_EARTH_RADIUS_KM 6371
#define KILOMETRES_IN_MILE 1.60934
#define YARDS_IN_MILE 1760

// true north is the 4:30 street.
// https://burningman.org/event/black-rock-city-guide/2019-black-rock-city-plan/
// TODO: correct for road widths as per the website.
int street_width_yards = 40;
int letter_streets_yards[] = {2500, 400, 250, 250, 250, 250, 200,
                              200,  200, 200, 200, 150, 150};
constexpr int letter_street_count = sizeof(letter_streets_yards) / sizeof(int);
/*
0 = center
1.0 = esplanade
2.0 = A
3.0 = B
*/
float GetLetterStreet(float miles) {
  float yards = miles * YARDS_IN_MILE;
  int start_distance = 0;
  int street = 0;
  int start = 0;
  for (; street < letter_street_count; ++street) {
    if (street > 0) {
      start += letter_streets_yards[street - 1];
    }
    // int start = street == 0 ? 0 : letter_streets_yards[street - 1];
    int end = letter_streets_yards[street] + start;
    if (yards > end) {
      continue;
    }
    float fraction = (yards - start) / (end - start);
    return street + fraction;
  }
  return 25;
}

float GetTimeStreet(float angle) {
  // North is 45 degrees off the main axis. (4:30 / 10:30 axis)
  // 0 => 10:30.
  // 180 => 4:30.
  // 12 hours total. 360 / 12 = 30
  // 12:00 = 45 degrees.
  float time = angle / 30;
  time -= 1.5;
  if (time < 0)
    time += 12;
  return time;
}

Address GetAddress(float miles, float angle) {
  //
  Address address;
  address.letter_street = GetLetterStreet(miles);
  address.time_street = GetTimeStreet(angle);
  return address;
}

greatcircle_solution
greatcircle_calculate(greatcircle_coordinates &coordinates) {
  float lat1 = coordinates.latitude1_degrees / DEGREES_IN_RADIAN;
  float long1 = coordinates.longitude1_degrees / DEGREES_IN_RADIAN;
  float lat2 = coordinates.latitude2_degrees / DEGREES_IN_RADIAN;
  float long2 = coordinates.longitude2_degrees / DEGREES_IN_RADIAN;

  double dLong = long1 - long2;
  double dLat = lat1 - lat2;

  double aHarv = pow(sin(dLat / 2.0), 2.0) +
                 cos(lat1) * cos(lat2) * pow(sin(dLong / 2), 2);
  double cHarv = 2 * atan2(sqrt(aHarv), sqrt(1.0 - aHarv));

  greatcircle_solution solution;

  float central_angle_radians = cHarv;

  float distance_kilometres = MEAN_EARTH_RADIUS_KM * central_angle_radians;

  solution.central_angle_degrees = central_angle_radians * DEGREES_IN_RADIAN;
  solution.distance_miles = distance_kilometres / KILOMETRES_IN_MILE;

  return solution;
}

float bearing(greatcircle_coordinates &coordinates) {
  float lat1 = coordinates.latitude1_degrees / DEGREES_IN_RADIAN;
  float long1 = coordinates.longitude1_degrees / DEGREES_IN_RADIAN;
  float lat2 = coordinates.latitude2_degrees / DEGREES_IN_RADIAN;
  float long2 = coordinates.longitude2_degrees / DEGREES_IN_RADIAN;

  float y = sin(long2 - long1) * cos(lat2);
  float x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat1) * cos(long2 - long1);
  float theta = atan2(y, x);
  return fmod((theta * 180 / PI + 360), 360.0); // in degrees
}