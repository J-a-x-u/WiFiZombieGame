
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

const char *password = "thereisnospoon..."; // not important, we only scan

// WS2812 - LED-strip
#define DATA_PIN 3
#define NUM_LEDS 8
CRGB leds[NUM_LEDS];

int rssis[3];

// ADJUST USED CHANNEL ACCORDING TO LOCAL REGULATIONS!
#define GAME_CHANNEL 13

#define ZOMBIE 0
const char *ZOMBIE_SSID = "zombie";
#define HUMAN  1
const char *HUMAN_SSID = "human";

// rssi equal/lower than this means no LED is on
#define FAR_LIMIT     -100
// rssi equal/higher than this means all LEDs are on
#define NEAR_LIMIT    -60
// game starts whenn other players are this 'far' away
#define START_LIMIT   -90
// infection is transmitted of rssi is equal/higher than this
#define INFECT_LIMIT  -50

int i_am;
const char *my_ssid;
const char *other_ssid;

int scanWifi(const char *ssid) {
  // scan WiFi for ssid, select strongest signal, return average over the last three scans
  int rssi = FAR_LIMIT;
  int n = WiFi.scanNetworks(false, false, GAME_CHANNEL);
  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) == ssid && WiFi.RSSI(i) > rssi)
        rssi = WiFi.RSSI(i);
    }
  }
  rssis[2] = rssis[1];  rssis[1] = rssis[0];  rssis[0] = rssi;  // store last three scans
  return (rssis[0] + rssis[1] + rssis[2] ) / 3; // average
}

void   resetRSSIs() { // reset stored rssis
  rssis[0] = rssis[1] = rssis[2] = FAR_LIMIT;
}

void flashLEDs(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    FastLED.show();
    delay(10);
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(10);
  }
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 10); // limit power used by LEDs

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();

  resetRSSIs();
  int i, rssi;
  for (i = 0; i < 10; i++) { // scan for max ten times (~10s)
    flashLEDs(CRGB::Blue);
    if ((rssi = scanWifi(ZOMBIE_SSID)) > FAR_LIMIT)
      break;
  }
  if (rssi == FAR_LIMIT) { // No zombie arround... I'm the zombie
    i_am = ZOMBIE;
    my_ssid = ZOMBIE_SSID;
    other_ssid = HUMAN_SSID;
  } else {
    i_am = HUMAN;
    my_ssid = HUMAN_SSID;
    other_ssid = ZOMBIE_SSID;
  }

  resetRSSIs();
  CRGB color = i_am == ZOMBIE ? CRGB::Red : CRGB::Green;
  flashLEDs(color); // show one flash sequence early, beacaus WiFi.softAP might need some seconds
  WiFi.softAP(my_ssid, password, GAME_CHANNEL, 0, 0);

  for (int j = 0; j < 10; j++) // show what we play as
    flashLEDs(color);

  color = i_am == ZOMBIE ? CRGB::Red : CRGB::Green;
  while (scanWifi(other_ssid) <= FAR_LIMIT)  flashLEDs(color); // wait for other players to show up
  color = i_am == ZOMBIE ? CRGB::DarkRed : CRGB::DarkGreen;
  while (scanWifi(other_ssid) > START_LIMIT)    flashLEDs(color); // wait for other players go hiding

  // turn builtin led off
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  resetRSSIs();
}

void loop() {
  int rssi;

  rssi = scanWifi(other_ssid);

  if (i_am == HUMAN && rssi > INFECT_LIMIT) { // become a zombie
    // inform of infection
    for (int i = 0; i < 10; i++)
      flashLEDs(CRGB::Red);
    i_am = ZOMBIE;
    my_ssid = ZOMBIE_SSID;
    other_ssid = HUMAN_SSID;
    WiFi.softAP(my_ssid, password, GAME_CHANNEL, 0, 0);
    resetRSSIs();
    rssi = scanWifi(other_ssid);
  }

  if (rssi < FAR_LIMIT) rssi = FAR_LIMIT;
  if (rssi > NEAR_LIMIT) rssi = NEAR_LIMIT;
  // calculate number of LEDs to light
  // linear range transformation from FAR_LIMIT..NEAR_LIMIT to 0..NUM_LEDS
  int numleds = (rssi - FAR_LIMIT) * NUM_LEDS / (NEAR_LIMIT - FAR_LIMIT);

  leds[NUM_LEDS - 1] = i_am == ZOMBIE ? CRGB::Red : CRGB::Green; // top LED is 'what am i' marker if unused

  CRGB color = i_am == HUMAN ? CRGB::Red : CRGB::Green;
  int i = 0;
  for (; i < numleds; i++)
    leds[i] = color;
  for (; i < NUM_LEDS - 1; i++) // remaining black/dark
    leds[i] = CRGB::Black;

  FastLED.show();
}
