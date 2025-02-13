#define LED_PIN 4       // Define LED_PIN as pin 4, which is connected to the LED
#define POT_PIN 15      // Define POT_PIN as pin 15, which is connected to the potentiometer

// Set the parameters for PWM (Pulse Width Modulation) control
const int freq = 5000;       // Set the PWM frequency to 5000 Hz
const int ledChannel = 0;    // Use PWM channel 0 for controlling the LED
const int resolution = 8;    // Set the resolution of PWM to 8 bits (values range from 0 to 255)

void setup() {
  Serial.begin(9600);        // Initialize serial communication at a baud rate of 9600
  // Setup LED PWM functionality
  ledcSetup(ledChannel, freq, resolution);  // Configure PWM channel with the specified frequency and resolution
  
  ledcAttachPin(LED_PIN, ledChannel);       // Attach the LED_PIN to the PWM channel so the LED can be controlled by PWM
}

void loop() {
  // Read the potentiometer value
  //int potValue = analogRead(POT_PIN);  // Read the analog value from the potentiometer (ranges from 0 to 4095 for 12-bit ADC)
  
  // Map the potentiometer value to PWM range (0-255)
  //int pwmWave = map(potValue, 0, 4095, 0, 255);  // Map the potentiometer value from its range (0-4095) to the PWM range (0-255)
  
  // Write PWM signal to LED
  ledcWrite(ledChannel, 255);  // Write the mapped PWM value to the LED, adjusting its brightness accordingly
  
  // Optional: Print the PWM value to Serial for debugging
  //Serial.println(pwmWave);  // Print the current PWM value to the serial monitor for debugging purposes
}
