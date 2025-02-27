#define STEP_PIN 5
#define DIR_PIN 6

void setup() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(DIR_PIN, HIGH);  // Set initial direction
}

void loop() {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500); // Adjust speed
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);
}