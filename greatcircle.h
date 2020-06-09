
// --------------------------------------------------------
// STRUCT greatcircle
// --------------------------------------------------------

struct greatcircle_coordinates {
  float latitude1_degrees;
  float longitude1_degrees;
  float latitude2_degrees;
  float longitude2_degrees;
};

struct greatcircle_solution {
  float central_angle_degrees;
  float distance_miles;
};

struct Address {
  float letter_street;
  float time_street;
};

Address GetAddress(float miles, float angle);

// --------------------------------------------------------
// FUNCTION PROTOTYPES
// --------------------------------------------------------

greatcircle_solution
greatcircle_calculate(greatcircle_coordinates &coordinates);

float bearing(greatcircle_coordinates &coordinates);