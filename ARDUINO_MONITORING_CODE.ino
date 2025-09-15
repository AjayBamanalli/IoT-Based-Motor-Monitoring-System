#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h> // Include the I2C library




// Pin definitions
const int relayStopPin = 8;           // Relay to stop the motor (example pin number)
const int buzzerPin = 9;             // Buzzer pin (example pin number)
const int voltagepin = A0;           // Voltage sensor pin (example pin number)
const int IR_sensor_pin = 2;         // IR sensor pin (example pin number)
const int temp1 = 3;                 // Temp sensor 1 pin (example pin number)
const int temp2 = 4;                 // Temp sensor 2 pin (example pin number)
const int currentpin = A1;           // Current sensor pin (example pin number)
const int b1 = 5;                    // Button 1 pin (example pin number)
const int b2 = 6;                    // Button 2 pin (example pin number)
const int b3 = 7;                    // Button 3 pin (example pin number)
const int b4 = 10;                   // Button 4 pin (example pin number)

const int oneWireBusPin = 11;        // Pin for OneWire bus for temperature sensors (example pin number)


LiquidCrystal_I2C lcd(0x27, 16, 2); 
// OneWire bus for DS18B20 sensors
OneWire oneWire(oneWireBusPin);
DallasTemperature sensors(&oneWire);
// Addresses for DS18B20 sensors
DeviceAddress tempMotorSensor, tempSurroundSensor;

// Variables to store sensor readings
float voltage = 0;
float current = 0;
float tempMotor = 0;
float tempSurround = 0;
int rpm = 0;
int pulseCount = 0;
unsigned long start_time = 0;

const unsigned long interval = 1000;  // Interval for RPM calculation

// Motor control state
String motorState = "Stopped";

void setup() {
  Serial.begin(9600);

  pinMode(relayStopPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(voltagepin, INPUT);
  pinMode(IR_sensor_pin, INPUT); 
  pinMode(temp1, INPUT);
  pinMode(temp2, INPUT);
  pinMode(currentpin, INPUT);
  pinMode(b1, INPUT_PULLUP);
  pinMode(b2, INPUT_PULLUP);
  pinMode(b3, INPUT_PULLUP);
  pinMode(b4, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(IR_sensor_pin), countPulse, FALLING);

  // Initially started
  digitalWrite(relayStopPin, HIGH);

  // Initialize the DS18B20 temperature sensors
  sensors.begin();

  // Locate devices on the bus
  if (!sensors.getAddress(tempMotorSensor, 0)) {
    Serial.println("Motor temperature sensor not found!");
  }
  if (!sensors.getAddress(tempSurroundSensor, 1)) {
    Serial.println("Surround temperature sensor not found!");
  }

  // Set resolution of DS18B20 sensors to 12 bits
  sensors.setResolution(tempMotorSensor, 12);
  sensors.setResolution(tempSurroundSensor, 12);


  lcd.begin(16, 2); 
  lcd.backlight();
}

void loop() {
  // Periodically read sensor data and send it to the ESP8266
  readSensors();
  calculateRPM();
  displayed();
  checkhealth();
  sendSensorDataToESP();
  
  // Reset variables for the next measurement
  pulseCount = 0;
  start_time = millis();

  // Re-enable interrupt
  attachInterrupt(digitalPinToInterrupt(IR_sensor_pin), countPulse, FALLING); 
}

// Count pulses from the E18-D80NK sensor
void countPulse() {
  pulseCount++;
}

void calculateRPM() {
  unsigned long current_time = millis();
  if (current_time - start_time >= interval) {  // Calculate RPM every interval
    detachInterrupt(digitalPinToInterrupt(IR_sensor_pin));  // Disable interrupt for accurate count

    // Calculate RPM (pulses in one second * 60)
    rpm = pulseCount * 60;

    // Re-enable interrupt
    attachInterrupt(digitalPinToInterrupt(IR_sensor_pin), countPulse, FALLING);  
  }
}

void stopmotor() {
  digitalWrite(relayStopPin, HIGH);
}

void startmotor() {
  digitalWrite(relayStopPin, LOW);
}

void readSensors() {
  // Read DS18B20 temperature sensors
  sensors.requestTemperatures();  // Send the command to get temperatures
  tempMotor = sensors.getTempC(tempMotorSensor);  // Get motor temperature in °C
  tempSurround = sensors.getTempC(tempSurroundSensor);  // Get surrounding temperature in °C

  if (digitalRead(b1) == LOW) {
    current = 3.5;
    delay(500); 
  } else if (digitalRead(b2) == LOW) {   
    current = 0;
    delay(500); 
  } else {
    current = 1.8;
    delay(500); 
  }

  if (digitalRead(b3) == LOW) {
    voltage = 330;
    delay(500); 
  } else if (digitalRead(b4) == LOW) {   
    voltage = 500;
    delay(500); 
  } else {
    voltage = 415;
    delay(500); 
  }
}

// Send sensor data to ESP8266 via Serial
void sendSensorDataToESP() {
  String data = String(voltage) + "," + String(current) + "," + String(tempMotor) + "," + String(tempSurround) + "," + String(rpm) + "," + String(motorState);
  Serial.println(data);
}

void buz() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(500);
  }
}

void displayed() {
  // LCD display code here

 lcd.clear();
    lcd.setCursor(0, 0);  // Set cursor to first row
    lcd.print("Volt: ");
    lcd.print(voltage);   // Display voltage

    lcd.setCursor(0, 1);  // Set cursor to second row
    lcd.print("Current: ");
    lcd.print(current);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);  // Set cursor to first row
    lcd.print("temp motor : ");
    lcd.print(tempMotor);   // Display voltage

    lcd.setCursor(0, 1);  // Set cursor to second row
    lcd.print("temp surr: ");
    lcd.print(tempSurround);

  delay(1000);
}



void displayalert() {
  // LCD display code here

lcd.clear();
    lcd.setCursor(0, 0);  // Set cursor to first row
    lcd.print("Stopped !!");
    lcd.setCursor(0, 1);  // Set cursor to second row
    lcd.print(motorState);
    
  
}

void checkhealth() {
  float temp = 155 - tempSurround;

  if (current == 0) {
    stopmotor();
    motorState = "Over-current";
    buz();
    displayalert();
  } else if (current > 2.24) {   
    stopmotor();
    motorState = "Over-current";
    buz();
    displayalert();
  } else if (voltage < 376) {   
    stopmotor();
    motorState = "Under-voltage";
    buz();
    displayalert();
  } else if (voltage > 456) {   
    stopmotor();
    motorState = "Over-voltage";
    buz();
    displayalert();
  } else if (tempMotor > temp) {   
    stopmotor();
    motorState = "Over-motor temperature";
    buz();
    displayalert();
  } else {   
    startmotor();
    motorState = "Running";
  }
}
