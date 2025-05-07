/************
4/3/25
Dorjee Tenzing
Overview:
Create premade protocolInstructions that consist of three operations: agitate, pause, bind
Write a state machine that calls existing motor functions with defined inputs and outputs
*************/

#include <ezButton.h>
#include <AccelStepper.h>
#include <Adafruit_NeoPixel.h>


#define MAX_LINE_LENGTH 32
#define MAX_LINES 100

// Define the pin and number of LEDs
#define LED_PIN 8
#define NUM_LEDS 1
// Create a NeoPixel object
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const int DIAG = 15;  // diag pin on TMC

const int AGITATION_MOTOR_STEP = 23;
const int AGITATION_MOTOR_DIR = 22;

const int MOTOR_X_STEP = 21;
const int MOTOR_X_DIR = 20;

const int MOTOR_Y_STEP = 19;  //
const int MOTOR_Y_DIR = 18;   //
const int AGITATION_MOTOR_STEP_CHANNEL = 5;
const int MOTOR_X_STEP_CHANNEL = 4;
const int MOTOR_Y_STEP_CHANNEL = 3;

ezButton limitSwitchX(4);          // create ezButton object that attach to ESP32 pin GPIO5
ezButton limitSwitchY(5);          // create ezButton object that attach to ESP32 pin GPIO6
ezButton limitSwitchAgitation(6);  // create ezButton object that attach to ESP32 pin GPIO7

// HardwareSerial mySerial(1);

// Initialize the tempBuffer with nulls ('\0')
char tempBuffer[MAX_LINES][MAX_LINE_LENGTH] = { { '\0' } };
// memset(tempBuffer, 0, sizeof(tempBuffer));
uint8_t startFlag = 0;
uint8_t i = 0;
uint8_t j = 0;

// types of protocolInstructions
enum ProtocolType {
  AGITATION,  // 'B'
  PAUSING,    // 'P'
  MOVING,     // 'M'
  INVALID     // For unknown protocol types
};
// Structure to store protocol properties
struct Protocol {
  ProtocolType type;
  uint8_t volume;
  uint8_t percentVolume;
  uint8_t speed;
  uint8_t duration;
  uint8_t initialSurfaceTime;
  uint8_t stopAtSequences;
  uint8_t sequencePauseTime;
};

// Protocol Array
char *protocolInstructions[] = {
  "M827395", "B491762", "P3", "M615829", "B728341", "P7", "B983251", "M147693", "P5", "M283741",
  "B672849", "P2", "M518392", "B739251", "P9", "M924873", "B356721", "P4", "M837462", "B671285",
  "P6", "B924173", "M462839", "P8", "M183627", "B519284", "P1", "B832749", "M746218", "P7",
  "M382615", "B721493", "P5", "B629471", "M548293", "P9", "M192847", "B364927", "P2", "M847362",
  "B512874", "P6", "B937415", "M284759", "P4", "M682391", "B715428", "P3", "B492671", "M531827",
  "P8", "M914275", "B267843", "P5", "B839472", "M362491", "P1", "M472681", "B619427", "P9",
  "B738429", "M294783", "P6", "M483729", "B175392", "P3", "B926471", "M571839", "P7", "M318472",
  "B827149", "P2", "B571382", "M649273", "P8", "M417629", "B385291", "P4", "B927416", "M518734",
  "P6", "M742193", "B618492", "P1", "B574829", "M392847", "P9", "M673284", "B941752", "P5",
  "B738192", "M512679", "P7", "M294678", "B415926", "P3", "B689472", "M471382", "P2", "M527491"
};

// global size of protocolInstructions
int size = sizeof(protocolInstructions) / sizeof(protocolInstructions[0]);  // Get number of elements

// init functions
Protocol parseProtocol(char *protocol);
ProtocolType getProtocolType(char *protocol);

// Initialize the tempBuffer with nulls ('\0')

void setup() {
  // Set up UART and GPIO
  Serial.begin(115200);
  pixels.begin();  // for led side quest

  limitSwitchAgitation.setDebounceTime(50);  // set debounce time of limit switch to 50 milliseconds
  limitSwitchX.setDebounceTime(50);
  limitSwitchY.setDebounceTime(50);
  pinMode(AGITATION_MOTOR_DIR, OUTPUT);
  pinMode(MOTOR_Y_DIR, OUTPUT);
  pinMode(MOTOR_X_DIR, OUTPUT);
  //limit switch pins set to input
  pinMode(4, INPUT_PULLDOWN);
  pinMode(5, INPUT_PULLDOWN);
  pinMode(6, INPUT_PULLDOWN);

  // Setup Agitation Motor Step Signal
  ledcAttachChannel(AGITATION_MOTOR_STEP, 1000, 8, AGITATION_MOTOR_STEP_CHANNEL);
  ledcWrite(AGITATION_MOTOR_STEP, 0);  // set duty cycle
  // Setup Agitation Motor Dir Signal
  ledcWrite(AGITATION_MOTOR_DIR, 0);  // set duty cycle
  ledcAttachChannel(MOTOR_X_STEP, 1000, 8, MOTOR_X_STEP_CHANNEL);
  ledcWrite(MOTOR_X_STEP, 0);

  ledcWrite(MOTOR_X_DIR, 0);
  // Y direction Configuration
  ledcAttachChannel(MOTOR_Y_STEP, 1000, 8, MOTOR_Y_STEP_CHANNEL);
  ledcWrite(MOTOR_Y_STEP, 0);

  ledcWrite(MOTOR_Y_DIR, 0);

  /** USEFUL FUNCTIONS:
   * ledcWriteTone(AGITATION_MOTOR_STEP, 1000);
   * ledcChangeFrequency(AGITATION_MOTOR_DIR, 10, 8);
   */

  /*********** RUN MAIN TEST ***********/

  // Fill in tempBuffer with premade protocol
  strcpy(tempBuffer[0], "P3");
  strcpy(tempBuffer[1], "A340532");
  uint8_t i = 0;

  // for (i = 0; i, MAX_LINES; i++) {
  //   //handle pause operation: P7. Input: Duration
  //   if (tempBuffer[i][0] == 'P') {
  //     uint8_t pauseDuration = tempBuffer[i][1] - '0';  //convert from ascii to int
  //     pauseMotors(pauseDuration);
  //   }

  //   //handle agitate operation: A74. Inputs: Agitation speed, agitation duration
  //   if (tempBuffer[i][0] == 'A') {
  //     uint8_t agitateSpeed = tempBuffer[i][4] - '0';                                         //extract speed
  //     uint8_t agitateDuration = ((tempBuffer[i][5] - '0') * 10) + (tempBuffer[i][6] - '0');  //extract duration
  //     uint8_t totalVolume = tempBuffer[i][1] - '0';                                          //extract total volume
  //     uint8_t percentVolume = ((tempBuffer[i][2] - '0') * 10) + (tempBuffer[i][3] - '0');    //extract percent volumt
  //     agitateMotors(agitateSpeed, agitateDuration, totalVolume, percentVolume);
  //   }

  //   //handle bind operation: B7. Inputs: Bind duration
  //   if (tempBuffer[i][0] == 'B') {
  //     uint8_t bindDuration = tempBuffer[i][0];
  //   }
  // }

  /*Main Test CASE*/

  // loop through array of protocolInstructions
  // for (int i = 0; i < size; i++)
  // {
  //   Protocol parsed = parseProtocol(protocolInstructions[i]); // Parse protocol

  //   // print out list
  //   Serial.print("Protocol: ");
  //   Serial.println(protocolInstructions[i]);
  //   // for each parsed protocol print out its information based on type
  //   switch (parsed.type)
  //   {
  //   // call the agitation function
  //   case AGITATION:
  //     agitateMotors(parsed.speed, parsed.duration, parsed.volume, parsed.percentVolume); // agitate the motors
  //                                                                                        // debug for information correctness
  //     Serial.println("Type: Agitation");
  //     Serial.print("Volume: ");
  //     Serial.println(parsed.volume);
  //     Serial.print("Percent Volume: ");
  //     Serial.println(parsed.percentVolume);
  //     Serial.print("Speed: ");
  //     Serial.println(parsed.speed);
  //     Serial.print("Duration: ");
  //     Serial.println(parsed.duration);
  //     break;

  //   case PAUSING:
  //     pauseMotors(parsed.duration);
  //     Serial.println("Type: Pausing");
  //     Serial.print("Duration: ");
  //     Serial.println(parsed.duration);
  //     break;

  //   case MOVING: // moving function not tested yet.
  //     moveSample(parsed.initialSurfaceTime, parsed.speed, parsed.stopAtSequences, parsed.sequencePauseTime);
  //     Serial.println("Type: Moving");
  //     Serial.print("Initial Surface Time: ");
  //     Serial.println(parsed.initialSurfaceTime);
  //     Serial.print("Speed: ");
  //     Serial.println(parsed.speed);
  //     Serial.print("Stop at Sequences: ");
  //     Serial.println(parsed.stopAtSequences);
  //     Serial.print("Sequence Pause Time: ");
  //     Serial.println(parsed.sequencePauseTime);
  //     break;
  //   }
  // }
  // /*Limit Switch Test Case*/
  //fun
  //sidequest

  // while(1){
  //   for (int i = 0; i < 256; i++) {
  //   // Set the pixel to the current color in the wheel
  //   pixels.setPixelColor(0, Wheel(i));
  //   pixels.show();
  //   delay(100);
  // }

  // while(1){
  //   moveMotor(MOTOR_X_STEP,MOTOR_X_DIR,HIGH,2,1);
  //   delay(100);
  //   moveMotor(MOTOR_X_STEP,MOTOR_X_DIR,LOW,2,1);
  //   delay(100);
  // }
  // //more fun
  autoHome();  //starting point
  while (1) {
    moveMotorY(HIGH, 5, 8);
    moveMotorX(HIGH, 3, 8.75);  //HIGH = RIght
    moveMotorY(HIGH, 5, 8);     //LOW = UP ???????

    moveMotorX(HIGH, 3, 8.75);  //HIGH = RIght
    moveMotorY(LOW, 5, 8);      //LOW = UP ???????

    moveMotorX(HIGH, 3, 8.75);  //HIGH = RIght
    moveMotorY(HIGH, 5, 8);     //LOW = UP ???????

    moveMotorX(HIGH, 3, 8.75);  //HIGH = RIght
    moveMotorY(LOW, 5, 8);      //LOW = UP ???????
    autoHome();
  }


  // moveMotorA(LOW,9,600000000);
  // agitateMotors(12000, 60, 1, 90);
}

// unused
void loop() {
  // //
  limitSwitchAgitation.loop();
  limitSwitchX.loop();
  limitSwitchY.loop();
}
/**
 * @brief:runs motor in home direction until a limit switch is hit
 * @param none
 * @retval: none
 */
void autoHome() {
  // Home MOtor X

  while (1) {
    limitSwitchX.loop();                //check status of limit switch
    ledcWriteTone(MOTOR_X_STEP, 1000);  // Drive motor
    digitalWrite(MOTOR_X_DIR, LOW);     //home direction is left = LOW
    //debug
    //determine output of the status of limit switchS

    uint8_t state = digitalRead(4);
    Serial.print("Limit switch status => ");
    Serial.println(state);

    //if the state of the limit switch is high that means it has been pressed
    if (state != 1) {
      Serial.println("Limit switch is currently Touched");
      //turn off motor in this event
      digitalWrite(MOTOR_X_DIR, HIGH);
      ledcWriteTone(MOTOR_X_STEP, 0);  //

      moveMotorX(HIGH, 2, 4);  //nudge motor to the right so it's not on the limit switch
      break;

    } else {
      //Serial.println("Limit switch is currently Untouched");
      continue;
    }
  }


  // Home MOtor Y
  while (1) {
    limitSwitchY.loop();               //check status of limit switch
    ledcWriteTone(MOTOR_Y_STEP, 800);  // Drive motor
    digitalWrite(MOTOR_Y_DIR, HIGH);   //home direction is Down = LOW
                                       //debug
    uint8_t state = digitalRead(5);
    Serial.print("Limit switch status => ");
    Serial.println(state);

    //if the state of the limit switch is high that means it has been pressed
    if (state != 1) {
      Serial.println("Limit switch is currently Touched");
      //turn off motor in this event
      digitalWrite(MOTOR_Y_DIR, HIGH);
      ledcWriteTone(MOTOR_Y_STEP, 0);  //

      moveMotorY(HIGH, 4, 16);  //nudge motor to the right so it's not on the limit switch
      break;

    } else {
      //Serial.println("Limit switch is currently Untouched");
      continue;
    }
  }
}





/**
 * @brief: take in a distance and move the gantry head that amount
 * note this particular motor driver is quarter microsteped.
 * @param distance
 * @retval: none
 */
void moveMotorY(int DIR, uint32_t speed, float distance) {  // 1 step is 1.8 degrees
  // convert distance to steps. for now i'm keeping it in number of revolutions
  uint32_t steps = distanceToStepsY(distance);
  uint16_t stepFrequency = mapSpeedY(speed);          // adjust to control speed (Hz)
  float secondsToRun = (float)steps / stepFrequency;  // time = steps / freq
  uint32_t durationMs = (uint32_t)(secondsToRun * 1000);

  digitalWrite(MOTOR_Y_DIR, DIR);              // set direction
  ledcWriteTone(MOTOR_Y_STEP, stepFrequency);  // start step pulses

  delay(durationMs);  // run long enough to cover desired steps

  ledcWriteTone(MOTOR_Y_STEP, 0);  // stop step pulses
}
// angle (degrees) = (arc length / radius) * (180 / π)
uint32_t distanceToStepsY(float distance)  // about 8.75mm
{
  //return distance * 200;
  return (uint32_t)(distance * (200.0 / 8.0) + 0.5f);
}
// num1 and num2 are the integer ranges of speed, num3 and num4 are the frequency ranges
unsigned int mapSpeedY(float value) {
  return (value - 1) * (1000 - 400) / (9 - 1) + 400;
}





/**
 * @brief: take in a distance and move the gantry head that amount
 * note this particular motor driver is quarter microsteped.
 * @param distance
 * @retval: none
 */
void moveMotorA(int DIR, uint32_t speed, float distance) {  // 1 step is 1.8 degrees
  // convert distance to steps. for now i'm keeping it in number of revolutions
  uint32_t steps = distanceToStepsA(distance);
  uint16_t stepFrequency = mapSpeedA(speed);          // adjust to control speed (Hz)
  float secondsToRun = (float)steps / stepFrequency;  // time = steps / freq
  uint32_t durationMs = (uint32_t)(secondsToRun * 1000);

  Serial.print("Agitation Frequency");
  Serial.println(stepFrequency);
  Serial.print("Agitation moving delay");
  Serial.println(durationMs);


  digitalWrite(AGITATION_MOTOR_DIR, DIR);              // set direction
  ledcWriteTone(AGITATION_MOTOR_STEP, stepFrequency);  // start step pulses

  delay(durationMs);  // run long enough to cover desired steps

  ledcWriteTone(AGITATION_MOTOR_STEP, 0);  // stop step pulses
}
// angle (degrees) = (arc length / radius) * (180 / π)
uint32_t distanceToStepsA(float distance)  // about 8.75mm
{
  // return distance * 200 * 16; // one rotation
  return (uint32_t)(distance * (3200.0 / 8.0) + 0.5f);
}
// num1 and num2 are the integer ranges of speed, num3 and num4 are the frequency ranges
unsigned int mapSpeedA(float value) {
  return (value - 1) * (1000 - 400) / (9 - 1) + 400;
}



/**
 * @brief: perform the agitation operation of moving motors up and down very quickly
 * @param agitateSpeed: agitation speed on a scale of 1-9
 * @param agitateDuration: agitation duration given in seconds
 * @param percentDepth: percentage of well volume to be displaced
 * @retval: a useful professor petersen
 */
void agitateMotors(uint16_t agitateSpeed, uint8_t agitateDuration, uint8_t totalVolume, uint8_t percentDepth) {
  //homeAgitation();
  // set motor speed
  //int agitationFrequency = mapSpeed(agitateSpeed); // newmap

  //int toggleDelay = 1000 / (depth);                // milliseconds per half-cycle (back and forth)

  // setup for changing directions
  bool DIR = LOW;                        // init direction
  unsigned long startTime = millis();    // capture a start time
  pinMode(AGITATION_MOTOR_DIR, OUTPUT);  // ensure pin is set to output

  while (millis() - startTime < (agitateDuration * 1000)) {
    digitalWrite(AGITATION_MOTOR_DIR, DIR);             // set new direction
    delay(1);                                           //delay before next iteration
    ledcWriteTone(AGITATION_MOTOR_STEP, agitateSpeed);  // Drive motor
    delay(300);                                         //controls depth of penetration
    ledcWriteTone(AGITATION_MOTOR_STEP, 0);             // stop motor


    DIR = (DIR == HIGH) ? LOW : HIGH;  // change directions after every toggleDelay iteration
  }
  // turn motors off afrer delay is finished
  ledcWriteTone(AGITATION_MOTOR_STEP, 0);
}






/**
 * @brief: take in a distance and move the gantry head that amount
 * note this particular motor driver is quarter microsteped.
 * @param distance
 * @retval: none
 */
void moveMotorX(int DIR, uint32_t speed, float distance) {  // 1 step is 1.8 degrees
  // convert distance to steps. for now i'm keeping it in number of revolutions
  uint32_t steps = distanceToStepsX(distance);
  uint16_t stepFrequency = mapSpeedX(speed);          // adjust to control speed (Hz)
  float secondsToRun = (float)steps / stepFrequency;  // time = steps / freq
  uint32_t durationMs = (uint32_t)(secondsToRun * 1000);


  Serial.print("Frequency => ");
  Serial.println(stepFrequency);
  Serial.print("delay => ");
  Serial.println(durationMs);

  digitalWrite(MOTOR_X_DIR, DIR);              // set direction
  ledcWriteTone(MOTOR_X_STEP, stepFrequency);  // start step pulses

  delay(durationMs);  // run long enough to cover desired steps

  ledcWriteTone(MOTOR_X_STEP, 0);  // stop step pulses
}
// angle (degrees) = (arc length / radius) * (180 / π)
uint32_t distanceToStepsX(float distance)  // about 8.75mm
{
  // 200 steps = 1 revolution
  //for returning # of steps per rotation use distance * 200
  //30mm per 1 revolution
  //1mm = 20/3

  //return distance * 200;
  //the number i devide 200 by is what i'm changing to get the best accuracy
  return (uint32_t)(distance * (200.0 / 31.24) + 0.5f);
}
// num1 and num2 are the integer ranges of speed, num3 and num4 are the frequency ranges
unsigned int mapSpeedX(float value) {
  return (value - 1) * (1000 - 400) / (9 - 1) + 400;
}

// /**
//  * @brief:slowly moves the magnets into the sampletray and transports sample to next well"M742193"
//  * @param   initialSurfaceTime first 3 numbers
//  * @param speed fourth number
//  * @param stopAtSequences fifth number
//  * @param sequencePauseTime sixth number
//  * @retval: none
//  */
// void moveSample(uint8_t initialSurfaceTime, uint8_t speed, uint8_t stopAtSequences, uint8_t sequencePauseTime)
// {
//   homeAgitation();                                     // home the agitation motor and wait
//   moveMotor(MOTOR_Y_STEP, MOTOR_Y_DIR, LOW, speed, 1); // must determine distance from idle Y value to  where the surface is
//   pauseMotors(initialSurfaceTime);
//   // fulltraydepth is about 41mm
//   uint8_t segs = 41 / stopAtSequences; // franctions of segment distances
//   // incrementaly lower magnets
//   for (int i = 0; i < stopAtSequences; i++)
//   {
//     moveMotor(MOTOR_Y_STEP, MOTOR_Y_DIR, LOW, speed, segs);
//     delay(1000 * sequencePauseTime);
//   }

//   moveMotor(MOTOR_Y_STEP, MOTOR_Y_DIR, HIGH, speed, 4); // back to idle distance
//   delay(100);                                           // delay for smoothness
//   moveMotor(MOTOR_X_STEP, MOTOR_X_DIR, HIGH, speed, 1); // must determine distance between wells and relate to rotations
// }

void homeAgitation() {

  // run motor Agitaion MOtor
  while (1) {
    limitSwitchAgitation.loop();
    ledcWriteTone(AGITATION_MOTOR_STEP, 500);  // Drive motor
    digitalWrite(AGITATION_MOTOR_DIR, HIGH);
    // determine if it is pressed
    if (limitSwitchAgitation.isPressed()) {
      Serial.println("The limit switch: UNTOUCHED -> TOUCHED");
    }
    if (limitSwitchAgitation.isReleased()) {
      Serial.println("The limit switch: TOUCHED -> UNTOUCHED");
    }

    int state = limitSwitchAgitation.getState();  // determine the state of the limit switch
    if (state == HIGH) {
      Serial.println("The limit switch: Untouched");
    } else {
      Serial.println("The limit switch: Touched");
      ledcWriteTone(AGITATION_MOTOR_STEP, 0);
      break;
    }
  }
}

/**
 * @brief: Pause the motor for a number of seconds
 * @param pauseDuration: pause duration in seconds
 * @retval: none
 */
void pauseMotors(uint8_t pauseDuration) {
  homeAgitation();  // home the agitator
  // analogWrite(AGITATION_MOTOR_STEP, 255);  //no pwm signal, use high for debugging
  ledcWriteTone(AGITATION_MOTOR_STEP, 0);
  delay(pauseDuration * 1000);  // convert from seconds to milliseconds for delay function
}



// maping ranges of depths to a fequency for direction channel
unsigned int mapDepth(float value1, float value2) {
  // Map value1 (1-6) directly to 5-20
  float mapped1 = ((value1 - 1) * (20 - 5) / (6 - 1)) + 5;

  // Map value2 (1-100) directly to 5-20
  float mapped2 = ((value2 - 1) * (20 - 5) / (100 - 1)) + 5;

  // Weighted blend (change weights if needed)
  float finalValue = (mapped1 * 0.6) + (mapped2 * 0.4);

  // Clamp to 5-20
  if (finalValue < 5)
    finalValue = 5;
  if (finalValue > 20)
    finalValue = 20;

  return (unsigned int)finalValue;
}

/**
 * @brief: iterate through the array and extract all information
 * @param char *protocol : pause duration in seconds
 * @retval: none
 */
Protocol parseProtocol(char *protocol) {
  Protocol parsed;  // object of protocol struct with the elements of all protocolInstructions contained
  parsed.type = getProtocolType(protocol);
  // for each type extract its respective information
  switch (parsed.type) {
    case AGITATION:
      parsed.volume = (protocol[1] - '0');                                      // Extract volume
      parsed.percentVolume = ((protocol[2] - '0') * 10) + (protocol[3] - '0');  // % of volume
      parsed.speed = (protocol[4] - '0');                                       // Speed
      parsed.duration = ((protocol[5] - '0') * 10) + (protocol[6] - '0');       // Duration
      break;

    case PAUSING:
      parsed.duration = (protocol[1] - '0');  // Only duration for pausing
      break;

    case MOVING:
      parsed.initialSurfaceTime = ((protocol[1] - '0') * 100) + ((protocol[2] - '0') * 10) + (protocol[3] - '0');  // Initial surface time
      parsed.speed = (protocol[4] - '0');                                                                          // Speed
      parsed.stopAtSequences = (protocol[5] - '0');                                                                // Stop at sequences
      parsed.sequencePauseTime = (protocol[6] - '0');                                                              // Sequence pause time
      break;
  }

  return parsed;
}

// Function to determine protocol type
ProtocolType getProtocolType(char *protocol) {
  // based on the first character of the protocolInstructions you can see what type of protocol it is
  switch (protocol[0]) {  // Check the first character
    case 'B':
      return AGITATION;
      break;
    case 'P':
      return PAUSING;
      break;
    case 'M':
      return MOVING;
      break;
    default:
      Serial.print("Error: Invalid protocol type '");
      Serial.print(protocol[0]);
      Serial.println("'");
      return INVALID;
      break;
  }
}

// Helper function to convert HSV to RGB
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
