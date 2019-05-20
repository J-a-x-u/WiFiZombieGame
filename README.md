# WiFiZombieGame

Simple Arduino version of the game idea from https://github.com/adangert/SpyTag-Wifi-game

This version is really simple. The first powered on ESP8266 becomes the zombie. Additional players start as humans. As soon as they go hiding and their signal drops, the game starts and the zombie starts seeking other players using the LEDs as range detector. On close encounter with a zombie the humans get infected and play on as zombies.

# Hardware

Wemos D1 mini

W2812b LED-Strip 8 LEDs

# Quickstart

Connect W2812b to pin3, 5v and GND

Setup Arduino IDE

1) Open the preferences window from the Arduino IDE. Go to File > Preferences
2) Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into the
 “Additional Board Manager URLs” field. Then click the “OK” button.
3) Open boards manager. Go to Tools > Board > Boards Manager…
4) Scroll down, select the ESP8266 board menu and install “esp8266”
5) Choose "LOLIN(WEMOS) D1 R2 & mini" from Tools > Board
6) Open WiFiZombieGame.ino
