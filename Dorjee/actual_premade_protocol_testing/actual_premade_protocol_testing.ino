/************
WHAT THIS CODE DOES, nvm its outdated
This is a simple test for sending data from the STM32 to the ESP32 via UART.
The STM32 sends two lines of data. The first line is a title and the second is the data.
The data is formatted as step type, # of repeats, mix speed, mix duration and dry time.
For example, "B9954" is bind step, 9 repeats, speed of 9, mix duration of 5, and dry time of 4.
The first line is ignored by using the startFlag and checking for a newline.
The startFlag is reset after the second line is terminated by the tab character.
*************/

#include "esp32-hal-timer.h"

hw_timer_t *timer = NULL;  // declaration of timer
static int dirTimerTicks = 0;
static int totalTimerTicks = 0;
// function call by the timer interruption
void IRAM_ATTR onTimer() {
  dirTimerTicks++;
  totalTimerTicks++;
}

const int AGITATION_STEP_PIN = 12;
const int AGITATION_DIR_PIN = 13;

const int VERTICAL_STEP_PIN = 27;
const int VERTICAL_DIR_PIN = 14;

const int HORIZONTAL_STEP_PIN = 25;
const int HORIZONTAL_DIR_PIN = 26;

#define MAX_LINE_LENGTH 32
#define MAX_LINES 100

// Initialize the tempBuffer with nulls ('\0')
char tempBuffer[MAX_LINES][MAX_LINE_LENGTH] = { { '\0' } };
uint8_t i = 0;
uint8_t j = 0;

void setup() {
  Serial.begin(115200);

  //agitation motor
  pinMode(AGITATION_DIR_PIN, OUTPUT);  // Set the pin as an output
  ledcAttachChannel(AGITATION_STEP_PIN, 5000, 8, 8);
  ledcWrite(AGITATION_STEP_PIN, 0);

  //vertical motor
  pinMode(VERTICAL_DIR_PIN, OUTPUT);  // Set the pin as an output
  ledcAttachChannel(VERTICAL_STEP_PIN, 100, 8, 9);
  ledcWrite(VERTICAL_STEP_PIN, 0);

  //horizontal motor
  pinMode(HORIZONTAL_DIR_PIN, OUTPUT);  // Set the pin as an output
  ledcAttachChannel(HORIZONTAL_STEP_PIN, 100, 8, 10);
  ledcWrite(HORIZONTAL_STEP_PIN, 0);

  //setup timer interrupt
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  if (timer == NULL) {
    Serial.println("Error with the start of the timer");
    while (1)
      ;
  }
  timerAlarm(timer, 10000, true, 0);  // 1000000 µs = 10ms = 0.01s
  timerStart(timer);

  //Fill in tempBuffer with premade protocol
  strcpy(tempBuffer[1], "A945");  //mix time, mix speed, mix depth
  strcpy(tempBuffer[2], "P5");
  strcpy(tempBuffer[3], "A563");
  //strcpy(tempBuffer[4], "B5");
  //strcpy(tempBuffer[5], "B8");

  Serial.println("Begin Test");
}

void loop() {

  //go line by line through tempBuffer and execute the protocol
  for (int a = 1; a < MAX_LINES; a++) {  //skip first line cuz it is the title
    //handle pause
    if (tempBuffer[a][0] == 'P') {
      pauseMotors(tempBuffer[a][1]);
    }
    //handle binding
    if (tempBuffer[a][0] == 'B') {
      //beingBinding(tempBuffer[1][1], tempBuffer[1][2]);
    }
    //handle agitation
    if (tempBuffer[a][0] == 'A') {
      startAgitating(tempBuffer[a][1], tempBuffer[a][2], tempBuffer[a][3]);
    }
  }
}

//pause the motor for specified time in milliseconds
void pauseMotors(char input_duration) {
  int intDuration = input_duration - '0';  //convert from ascii to int
  ledcWrite(AGITATION_STEP_PIN, 255);
  delay(intDuration * 1000);         //convert from seconds to milliseconds for delay function
  ledcWrite(AGITATION_STEP_PIN, 0);  //this is just for debugging to see the pause function bring the pin high and then low
}

//three timing constraints: total agitation time, direction switching time, acceleration time
void startAgitating(unsigned long mix_time, uint8_t mix_speed, uint8_t mix_depth) {
  //ascii to int conversions
  int int_mix_time = mix_time - '0';    //convert from ascii to int
  int int_mix_speed = mix_speed - '0';  //convert from ascii to int
  int int_mix_depth = mix_depth - '0';  //convert from ascii to int

  //map inputs to usable numbers
  int mix_time_ms = int_mix_time * 1000;  //get the mix_time in milliseconds
  int agitateStepFrequency = mapSpeedtoFreq(int_mix_speed);
  int agitateHalfPeriod = mapDepthtoTime(int_mix_depth);  //half period cuz thats what it is? what do i name this

  /** begin actual agitation process ***/

  //start the interrupt for chef time
  dirTimerTicks = 0;
  totalTimerTicks = 0;
  //timerRestart(timer);

  //get start time and start the motor up
  uint32_t curTime = millis();
  ledcChangeFrequency(AGITATION_STEP_PIN, 5000, 8);
  ledcWrite(AGITATION_STEP_PIN, 128);

  //stay in this agitation loop until the total agitation time is finished
  while ((millis() - curTime) < mix_time_ms) {
    if (dirTimerTicks == 10) {
      ledcChangeFrequency(12, 5000 + 500 * int_mix_speed / 3, 8);
    }
    if (dirTimerTicks == 20) {
      ledcChangeFrequency(12, 5000 + 500 * int_mix_speed * 2 / 3, 8);
    }
    if (dirTimerTicks == 30) {
      ledcChangeFrequency(12, 5000 + 500 * int_mix_speed, 8);
    }
    if (dirTimerTicks == agitateHalfPeriod - 20) {
      ledcChangeFrequency(12, 5000 + 500 * int_mix_speed * 2 / 3, 8);
    }
    if (dirTimerTicks == agitateHalfPeriod - 10) {
      ledcChangeFrequency(12, 5000 + 500 * int_mix_speed / 3, 8);
    }
    if (dirTimerTicks == agitateHalfPeriod) {
      ledcChangeFrequency(12, 5000, 8);
      digitalWrite(13, !digitalRead(13));
      dirTimerTicks = 0;
    }
  }
  //finished agitating, turn the motor off
  ledcWrite(AGITATION_STEP_PIN, 0);
}

int mapSpeedtoFreq(int input_speed) {
  int output_frequency = 5500 + input_speed * 500;  //input speed of 9 maxes out at 10khz
  return output_frequency;
}

int mapDepthtoTime(int input_depth) {
  int output_time = 55 + input_depth * 5;  //input depth of 9 maxes out at 100ms of movement. min depth is 55ms of movement
  return output_time;
}
