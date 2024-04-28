#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>
#include <WiFi.h>
#include <HTTPClient.h>




// WiFi credentials
const char* ssid = "";      
const char* password = "";    




// Create an instance of the sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);




// States to remember if red or blue was recently detected
bool redDetected = false;
bool blueDetected = false;


bool copsDetected = false;




// Timeframe in milliseconds in which both colors must be detected
const unsigned long detectionWindow = 5000; // 5 seconds
unsigned long lastRedTime = 0;
unsigned long lastBlueTime = 0;
unsigned long lastCopTime = 0;




void setup() {
  Serial.begin(115200);
  delay(10);




  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  Wire.begin(11, 12); // SDA on GPIO11, SCL on GPIO12 for ESP32-S3



  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {  
    delay(500);
    Serial.print(".");
    attempts++;
  }


  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


    
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi. Check your settings and try again.");
  }

}




void loop() {
  uint16_t red, green, blue, clear;
  tcs.getRawData(&red, &green, &blue, &clear);




  // Print raw values for debugging purposes
  Serial.print("R: "); Serial.print(red);
  Serial.print(" G: "); Serial.print(green);
  Serial.print(" B: "); Serial.print(blue);
  Serial.print(" Clear: "); Serial.println(clear);




  // Normalize the readings
  float sum = red + green + blue;
  float r = red / sum;
  float g = green / sum;
  float b = blue / sum;




  // Detect red color
  if (r > 0.7) {
    lastRedTime = millis();
    redDetected = true;
  }




  // Detect blue color
  if (blue > 0.6) {
    lastBlueTime = millis();
    blueDetected = true;
  }




  if (!copsDetected) {
    // Check if both red and blue were detected within the timeframe
    if (redDetected && blueDetected &&
      (millis() - lastRedTime <= detectionWindow) &&
      (millis() - lastBlueTime <= detectionWindow)) {
        Serial.println("Cops detected!");
        // Reset the detection
        redDetected = false;
        blueDetected = false;
        copsDetected = true;
        lastCopTime = millis();
        sendStopHttpRequest();
    }
  }
  else {
    if (redDetected && blueDetected &&
      (millis() - lastRedTime <= detectionWindow) &&
      (millis() - lastBlueTime <= detectionWindow)) {
      lastCopTime = millis();
      redDetected = false;
      blueDetected = false;
      }
   
    if (millis() - lastCopTime >= 10000) {
      copsDetected = false;
      sendPlayHttpRequest();
    }
  }




  delay(500); // Delay before the next reading
}




void sendStopHttpRequest() {
  HTTPClient http;
  http.begin("");  // Specify request destination
  http.addHeader("Content-Type", "application/json");  // Specify content-type header




  int httpResponseCode = http.GET();  // Send the request




  if (httpResponseCode > 0) {
    String response = http.getString();  // Get the response
    Serial.println(httpResponseCode);  
    Serial.println(response);          
  } else {
    Serial.print("Error on sending GET Request: ");
    Serial.println(httpResponseCode);
  }




  http.end();
}




void sendPlayHttpRequest() {
  HTTPClient http;
  http.begin("");  // Specify request destination
  http.addHeader("Content-Type", "application/json");  // Specify content-type header




  int httpResponseCode = http.GET();  // Send the request




  if (httpResponseCode > 0) {
    String response = http.getString();  // Get the response
    Serial.println(httpResponseCode);  
    Serial.println(response);          
  } else {
    Serial.print("Error on sending GET Request: ");
    Serial.println(httpResponseCode);
  }




  http.end();
}


