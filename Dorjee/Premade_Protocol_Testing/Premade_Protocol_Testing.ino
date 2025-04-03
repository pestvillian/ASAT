/************
4/3/25
Dorjee Tenzing
Overview:
Create premade protocols that consist of three operations: agitate, pause, bind
Write a state machine that calls existing motor functions with defined inputs and outputs
*************/
const int MOTOR_ONE_STEP = 12;
const int MOTOR_ONE_DIR = 13;

#define MAX_LINE_LENGTH 32
#define MAX_LINES 100

// Initialize the tempBuffer with nulls ('\0')
char tempBuffer[MAX_LINES][MAX_LINE_LENGTH] = { { '\0' } };
void setup() {
  //Set up UART and GPIO
  Serial.begin(115200);

  //initialize motor step and dir pins
  pinMode(MOTOR_ONE_STEP, OUTPUT);
  pinMode(MOTOR_ONE_DIR, OUTPUT);
  analogWriteResolution(MOTOR_ONE_STEP, 8);
  analogWriteResolution(MOTOR_ONE_DIR, 8);


  /*********** RUN MAIN TEST ***********/
  
  // Fill in tempBuffer with premade protocol
  strcpy(tempBuffer[0], "P3");
  strcpy(tempBuffer[1], "A94");
  uint8_t i = 0;

  for (i = 0; i, MAX_LINES; i++) {
    //handle pause operation: P7. Input: Duration
    if (tempBuffer[i][0] == 'P') {
      uint8_t pauseDuration = tempBuffer[i][1] - '0';  //convert from ascii to int
      pauseMotors(pauseDuration);
    }

    //handle agitate operation: A74. Inputs: Agitation speed, agitation duration
    if (tempBuffer[i][0] == 'A') {
      uint8_t agitateSpeed = tempBuffer[i][1] - '0';
      uint8_t agitateDuration = tempBuffer[i][2] - '0';
      agitateMotors(agitateSpeed, agitateDuration);
    }

    //handle bind operation: B7. Inputs: Bind duration
    if (tempBuffer[i][0] == 'B') {
      uint8_t bindDuration = tempBuffer[i][0];
    }
  }
}

void loop() {
}
/**
 * @brief: Pause the motor for a number of seconds
 * @param pauseDuration: pause duration in seconds
 * @retval: none
 */
void pauseMotors(uint8_t pauseDuration) {
  analogWrite(MOTOR_ONE_STEP, 255);  //no pwm signal, use high for debugging
  delay(pauseDuration * 1000);         //convert from seconds to milliseconds for delay function
  analogWrite(MOTOR_ONE_STEP, 0);    //drive back low for debugging
}

/**
 * @brief: perform the agitation operation of moving motors up and down very quickly
 * @param agitateSpeed: agitation speed on a scale of 1-9
 * @param agitateDuration: agitation duration given in seconds
 * @retval: none
 */
void agitateMotors(uint8_t agitateSpeed, uint8_t agitateDuration) {
  //set motor speed
  int agitationFrequency = mapAgitationSpeed(agitateSpeed);
  analogWriteFrequency(MOTOR_ONE_STEP, agitationFrequency);
  analogWrite(MOTOR_ONE_STEP, 128);
  //set oscillation frequency
  analogWriteFrequency(MOTOR_ONE_DIR, 10);  //for testing, have direction switch every 100ms
  analogWrite(MOTOR_ONE_STEP, 128);
  //set agitation duration
  delay(agitationDuration * 1000);
}

/**
 * @brief: Map agitationSpeed value to a PWM frequency
 * @param agitateSpeed: agitation speed on a scale of 1-9
 * @retval: frequency of a PWM signal given in Hz 
 */
int mapAgitationSpeed(uint8_t agitateSpeed) {
  if (agitateSpeed == 9) {
    return 1000;
  } else if (agitateSpeed == 1) {
    return 500;
  } else {
    return 800;
  }
}
