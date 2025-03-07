

#define STEP_PIN 5
#define DIR_PIN 6
//psudo code for toggling direction
  // Toggle direction every 1000 steps (adjust as needed)
    // if (counter % 1000000 == 0) {  
    //     DIR = (DIR == HIGH) ? LOW : HIGH;  // Toggle between HIGH and LOW
    //     digitalWrite(DIR_PIN, DIR);  // Apply new direction
    // }
char protocols[] = {
 "B9954","P9","M999959"
};
void parseProtocol(char protocols[]);

void parseProtocol(char protocols[]){
  serialPrint("helloWorld");
}

int DIR = HIGH;
int counter = 0;
//declare my runmotor function
void runmotor(uint32_t,int,uint32_t);

// operable frequency is about 400 to 1000
void runmotor(uint32_t frequency,int direction,uint32_t steps){
  uint32_t del = 1/frequency; //convert a frequency into a delay
  digitalWrite(DIR_PIN,direction);
//imitate the pwm library
  for(int i = 0; i < steps; i++){ //this should pulse the motor for a desired distance
    digitalWrite(STEP_PIN,HIGH); //high
    delay(del);
    digitalWrite(STEP_PIN,LOW); //low
    delay(del);
  }

}


void setup() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(DIR_PIN, DIR);  // Set initial direction
}

void loop() {
  
    // //this should run the motor in alternating directions continuously
    // runmotor(600,HIGH,500);
    // delay(50);
    // runmotor(600,LOW,500);
    // delay(50);
    // // Step pulse


    parseProtocols(Protocols);
}