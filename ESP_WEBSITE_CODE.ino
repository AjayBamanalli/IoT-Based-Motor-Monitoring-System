#include <ESP8266WiFi.h>           // Correct WiFi library for ESP8266
#include <ESP8266WebServer.h>       // WebServer library for ESP8266

// Set up the Access Point SSID and password
const char* ap_ssid = "motor_protection_system";  // Name of the access point
const char* ap_password = "123456789";        // Password for the access point

// Create web server on port 80
ESP8266WebServer server(80);

// Variables to store parsed sensor data
float voltage = 0;
float current = 0;
  float tempMotor = 0;
float tempSurround = 0;
int rpm = 0;
String motorState = "";

// Setup: Initializes Serial communication and starts Access Point
void setup() {
  // Start Serial communication
  Serial.begin(9600);
  delay(1000);  // Wait for Serial Monitor to start

  // Start the Access Point (AP)
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ap_ssid, ap_password);  // Start the ESP8266 as an Access Point
  
  // Print the IP address of the Access Point
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP Address: ");
  Serial.println(IP);

  // Set up the web server route
  server.on("/", handleRoot);  // Serve the web page when accessing root URL
  
  // Start the web server
  server.begin();
}

void loop() {
  // Handle incoming client requests for the web server
  server.handleClient();

  // Check for incoming serial data from Arduino (or other microcontroller)
  if (Serial.available()) {
    String incomingData = Serial.readStringUntil('\n');  // Read incoming serial data
    Serial.println("incomingData");
   Serial.println(incomingData);
    parseData(incomingData);
    // Parse the data
  }
}

// Function to parse the comma-separated sensor data from Arduino
void parseData(String data) {
  // Find the indices of the commas to split the data
  int commaIndex1 = data.indexOf(',');
  int commaIndex2 = data.indexOf(',', commaIndex1 + 1);
  int commaIndex3 = data.indexOf(',', commaIndex2 + 1);
  int commaIndex4 = data.indexOf(',', commaIndex3 + 1);
  int commaIndex5 = data.indexOf(',', commaIndex4 + 1);

  // Parse the data and assign values to variables
  voltage = data.substring(0, commaIndex1).toFloat();
  current = data.substring(commaIndex1 + 1, commaIndex2).toFloat();
  tempMotor = data.substring(commaIndex2 + 1, commaIndex3).toInt();
  tempSurround = data.substring(commaIndex3 + 1, commaIndex4).toInt();
  rpm = data.substring(commaIndex4 + 1).toInt();

  // The motor state is the last item in the data
 motorState = data.substring(commaIndex5 + 1);  // Get the motor state ("Running" or "Stopped")

  // Optionally print parsed data for debugging
  Serial.println("Parsed Data:");
  Serial.print("Voltage: "); Serial.println(voltage);
  Serial.print("Current: "); Serial.println(current);
  Serial.print("Motor Temp: "); Serial.println(tempMotor);
  Serial.print("Surround Temp: "); Serial.println(tempSurround);
  Serial.print("RPM: "); Serial.println(rpm);
  Serial.print("Motor State: "); Serial.println(motorState);
}

// Function to handle the root URL of the web server (display data)
void handleRoot() {
  String html = "<html><head>";
  
  // Adding some basic styling for a modern and clean look
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; color: #333; }";
  html += "header { background-color: #4CAF50; padding: 15px; color: white; text-align: center; font-size: 24px; }";
  html += "h1 { margin: 0; font-size: 28px; }";
  html += "p { font-size: 18px; margin: 10px 0; }";
  html += ".container { max-width: 800px; margin: 20px auto; padding: 20px; background-color: white; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); border-radius: 8px; }";
  html += ".data-item { font-size: 20px; color: #555; }";
  html += ".data-item b { color: #4CAF50; }";
  html += ".footer { text-align: center; margin-top: 20px; font-size: 14px; color: #888; }";
  html += "</style>";

  // Auto-refresh the page every 1 second
  html += "<meta http-equiv='refresh' content='1'>";  // This line forces a refresh every 1 second
  
  // Header of the page
  html += "</head><body>";
  html += "<header><h1>Motor Protection System</h1></header>";
  
  // Main content container
  html += "<div class='container'>";
  html += "<p class='data-item'><b>Voltage:</b> " + String(voltage) + " V</p>";
  html += "<p class='data-item'><b>Current:</b> " + String(current) + " A</p>";
  html += "<p class='data-item'><b>Motor Temperature:</b> " + String(tempMotor) + " °C</p>";
  html += "<p class='data-item'><b>Surrounding Temperature:</b> " + String(tempSurround) + " °C</p>";
  html += "<p class='data-item'><b>RPM:</b> " + String(rpm) + "</p>";
  html += "<p class='data-item'><b>Motor State:</b> " + motorState + "</p>";
  
  // Footer
  html += "<div class='footer'>Motor Protection System - ESP8266</div>";

  html += "</div>"; // Close container div
  html += "</body></html>";

  // Send the HTML content as a response
  server.send(200, "text/html", html);
}
