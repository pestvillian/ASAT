/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-limit-switch
 */

#include <ezButton.h>

ezButton limitSwitch(5);  // create ezButton object that attach to ESP32 pin GPIO16

void setup() {
  Serial.begin(115200);
  limitSwitch.setDebounceTime(50); // set debounce time to 50 milliseconds
}

void loop() {
  limitSwitch.loop(); // MUST call the loop() function first

  if(limitSwitch.isPressed())
    Serial.println("The limit switch: UNTOUCHED -> TOUCHED");

  if(limitSwitch.isReleased())
    Serial.println("The limit switch: TOUCHED -> UNTOUCHED");

  int state = limitSwitch.getState();
  if(state == HIGH)
    Serial.println("The limit switch: UNTOUCHED");
  else
    Serial.println("The limit switch: TOUCHED");
}