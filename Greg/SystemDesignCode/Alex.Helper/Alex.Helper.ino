const int analogPin = 13;   // Example ADC pin
const int digitalPin = 12;  // Example digital output pin

void setup() {
  pinMode(analogPin, INPUT);    // Set analog pin as input
  pinMode(digitalPin, OUTPUT);  // Set digital pin as output
  Serial.begin(115200);         // For debugging
}

void loop() {
  int analogValue = analogRead(analogPin);       // Read analog value
  float voltage = analogValue * (3.3 / 4095.0);  // Convert to voltage (approximately)
  int outputState;
  if (voltage > 1.5) {               // Example threshold
    digitalWrite(digitalPin, HIGH);  // Set digital output high
    
    outputState = HIGH;
  } else {
    digitalWrite(digitalPin, LOW);  // Set digital output low
    outputState = LOW;
  }


  Serial.print("Analog Value: ");
  Serial.println(analogValue);

  Serial.print("Voltage: ");
  Serial.println(voltage);

  Serial.print("Digital Pin: ");
  if (outputState == HIGH) {
    Serial.println("HIGH");
  } else {
    Serial.println("LOW");
  }

  delay(1000);
}