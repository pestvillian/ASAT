#define DELAY 10
#define BIG_DELAY 50

void setup() {
  //set dir pin
  pinMode(13, OUTPUT);  //vertical motor dir pin is a digital gpio
  digitalWrite(13, 0);

  //step
  ledcAttachChannel(12, 11000, 8, 8);
  ledcWrite(12, 0);
  delay(1000);
}

void loop() {
  //motor starts off and going forward (0)
  ledcChangeFrequency(12, 5000, 8);  //start slow
  ledcWrite(12, 180);                //turn on motor
  delay(DELAY);
  //speed up
  ledcChangeFrequency(12, 6000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 7000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 8000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 9000, 8);  //speed up
  delay(DELAY);
;
  ledcChangeFrequency(12, 10000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 11000, 8);  //max speed 
  delay(BIG_DELAY);                   //big delay

  //turn motor off
  ledcWrite(12, 0);     //stop
  digitalWrite(13, 1);  //prepare direction switch (1)
  delay(DELAY);

  //turn motor on
  ledcChangeFrequency(12, 5000, 8);  //start slow
  ledcWrite(12, 180);
  delay(DELAY);
    //speed up
  ledcChangeFrequency(12, 6000, 8);  //speed up
    delay(DELAY);
  ledcChangeFrequency(12, 7000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 8000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 9000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 10000, 8);  //speed up
  delay(DELAY);
  ledcChangeFrequency(12, 11000, 8);  //max speed
  delay(BIG_DELAY);

  //turn motor off and prep switch directions (0)
  ledcWrite(12, 0);
  digitalWrite(13, 0);
  delay(DELAY);
}