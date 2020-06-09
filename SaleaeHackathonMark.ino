#include <SPI.h> // include libraries
#include <Wire.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <Adafruit_Keypad.h>
#include <LoRa.h>
#include <TinyGPS.h>

#include "greatcircle.h"

// define your pins here
// can ignore ones that don't apply
#define R1 A0
#define R2 A1
#define R3 A2
#define R4 10
#define C1 13
#define C2 12
#define C3 11
// leave this import after the above configuration

const byte ROWS = 4; // rows
const byte COLS = 3; // columns
// define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
    {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
byte rowPins[ROWS] = {R1, R2, R3,
                      R4};         // connect to the row pinouts of the keypad
byte colPins[COLS] = {C1, C2, C3}; // connect to the column pinouts of the
                                   // keypad

// initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad =
    Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

SSD1306AsciiAvrI2c oled;
#define I2C_ADDRESS 0x3C

TinyGPS gps;

const uint32_t max_gps_age = 10000;

uint32_t last_key_press_ms = 0;
char last_key = '\0';
int last_key_count = 0;

#define TX_BUFFER_SIZE 20
char tx_buffer[TX_BUFFER_SIZE];
char tx_buffer_length = 0;
char rx_buffer[TX_BUFFER_SIZE];
// LoR32u4 433MHz V1.2 (white board)
#define SCK 15
#define MISO 14
#define MOSI 16
#define SS 8
#define RST 4
#define DI0 7
#define BAND 433E6
#define PABOOST true

String outgoing;          // outgoing message
byte msgCount = 0;        // count of outgoing messages
byte localAddress = 0xBB; // address of this device
byte destination = 0xFF;  // destination to send to
long lastSendTime = 0;    // last send time
int interval = 2000;      // interval between sends

greatcircle_coordinates input_cords;

void setup() {
  input_cords.latitude1_degrees = 40.785980;
  input_cords.longitude1_degrees = -119.205840;

  for (int i = 0; i < TX_BUFFER_SIZE; ++i)
    tx_buffer[i] = '\0';

  last_key_press_ms = millis();
  Serial.begin(9600);
  Serial1.begin(9600);

  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
  oled.displayRemap(true);
  oled.clear();
  oled.setCursor(0, 2);
  oled.print("loading...");

  customKeypad.begin();

  DrawGps(0, 0, max_gps_age + 1);

  LoRa.setPins(SS, RST, DI0); // set CS, reset, IRQ pin

  if (!LoRa.begin(BAND, PABOOST)) {
    Serial1.println("LoRa init failed. Check your connections.");
    while (true)
      ; // if failed, do nothing
  }
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");
}

void loop() {

  customKeypad.tick();
  char letter = HandleKeypad();
  if (letter == '!' && tx_buffer_length > 0) {
    TransmitTxBuffer();
    oled.setCursor(0, 2);
    oled.clearToEOL();
    oled.print("sent!");

  } else if (letter != '\0' && letter != '!') {
    if (tx_buffer_length >= TX_BUFFER_SIZE - 1) {
      TransmitTxBuffer();
    }
    tx_buffer[tx_buffer_length++] = letter;
    tx_buffer[tx_buffer_length] = '\0';
    oled.setCursor(0, 2);
    oled.clearToEOL();
    oled.print(tx_buffer);
  }

  bool newData = false;
  // int count = 0;
  while (Serial1.available()) {
    // if(count++ > 2000)
    // break;
    char c = Serial1.read();
    if (gps.encode(c)) // Did a new valid sentence come in?
      newData = true;
  }

  if (newData) {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    DrawGps(flat, flon, age);
  }
}

void TransmitTxBuffer() {
  Serial1.print("TX: ");
  Serial1.println(tx_buffer);
  sendMessage(tx_buffer);
  LoRa.receive();
  tx_buffer_length = 0;
  tx_buffer[tx_buffer_length] = '\0';
  oled.setCursor(0, 3);
  oled.clearToEOL();
}

void DrawGps(float lat, float lon, uint32_t age) {
  oled.setCursor(0, 0);
  oled.clearToEOL();
  if (age > max_gps_age) {
    oled.print("GPS Lost");
  } else {
    input_cords.latitude2_degrees = lat;
    input_cords.longitude2_degrees = lon;
    greatcircle_solution solution = greatcircle_calculate(input_cords);
    auto angle = bearing(input_cords);
    oled.print(solution.distance_miles, 1);
    oled.print(" miles ");
    oled.print(angle, 1);
    oled.print(" degrees");
    /*
    oled.print("LAT=");
    oled.print(lat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lat, 6);
    oled.print(" LON=");
    oled.print(lon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lon, 6);
    */
    // 2019 spike location: 40.785980, -119.205840
  }
}

void DrawTextOnLine(const char *message, int line) {
  oled.setCursor(0, line);
  oled.clearToEOL();
  oled.print(message);
}

char GetLetter(char key, char count) {
  if (key == '2')
    return 'a' + count - 1;
  if (key == '3')
    return 'd' + count - 1;
  if (key == '4')
    return 'g' + count - 1;
  if (key == '5')
    return 'j' + count - 1;
  if (key == '6')
    return 'm' + count - 1;
  if (key == '7') // no Q (PRS)
  {
    if (count == 1)
      return 'p';
    else
      return 'q' + count - 1;
  }
  if (key == '8')
    return 't' + count - 1;
  if (key == '9')
    return 'w' + count - 1; // no Z (WXY)
  if (key == '0')
    return ' ';
  return '!';
}

char HandleKeypad() {
  uint32_t now = millis();
  if (last_key_count != 0 && (now - last_key_press_ms > 500)) {
    // commit and move on.
    if (last_key_count > 0) {
      char letter = GetLetter(last_key, last_key_count);
      last_key = '\0';
      last_key_count = 0;
      return letter;
    }
  }

  while (customKeypad.available()) {
    keypadEvent e = customKeypad.read();
    if (e.bit.EVENT == KEY_JUST_RELEASED) {
      customKeypad.clear(); // ignore anything in the queue (should not have
                            // anything left)
      if (e.bit.KEY == last_key) {
        last_key_count++;
        last_key_press_ms = now;
        if (last_key_count == 3) {
          // commit, and reset.
          char letter = GetLetter(last_key, last_key_count);
          last_key = '\0';
          last_key_count = 0;
          last_key_press_ms = now;
          return letter;
        }
        return;
      } else if (last_key_count > 0) {
        // commit last key, start new key.
        char letter = GetLetter(last_key, last_key_count);
        last_key = e.bit.KEY;
        last_key_count = 1;
        last_key_press_ms = now;
        return letter;
      } else {
        last_key = e.bit.KEY;
        last_key_count = 1;
        last_key_press_ms = now;
        return;
      }
    }
  }
  return '\0';
}

void onReceive(int packetSize) {
  if (packetSize == 0)
    return; // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();       // recipient address
  byte sender = LoRa.read();         // sender address
  byte incomingMsgId = LoRa.read();  // incoming msg ID
  byte incomingLength = LoRa.read(); // incoming msg length

  String incoming = ""; // payload of packet

  while (LoRa.available()) {       // can't use readString() in callback, so
    incoming += (char)LoRa.read(); // add bytes one by one
  }

  if (incomingLength != incoming.length()) { // check length for error
    Serial1.println("error: message length does not match length");
    return; // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return; // skip rest of function
  }
  incoming.toCharArray(rx_buffer, TX_BUFFER_SIZE);
  DrawTextOnLine(rx_buffer, 1);

  // if message is for this device, or broadcast, print details:
  Serial1.println("Received from: 0x" + String(sender, HEX));
  Serial1.println("Sent to: 0x" + String(recipient, HEX));
  Serial1.println("Message ID: " + String(incomingMsgId));
  Serial1.println("Message length: " + String(incomingLength));
  Serial1.println("Message: " + incoming);
  Serial1.println("RSSI: " + String(LoRa.packetRssi()));
  Serial1.println("Snr: " + String(LoRa.packetSnr()));
  Serial1.println();
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();            // start packet
  LoRa.write(destination);       // add destination address
  LoRa.write(localAddress);      // add sender address
  LoRa.write(msgCount);          // add message ID
  LoRa.write(outgoing.length()); // add payload length
  LoRa.print(outgoing);          // add payload
  LoRa.endPacket();              // finish packet and send it
  msgCount++;                    // increment message ID
}