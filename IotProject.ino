#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define PIR_PIN D5              // PIR motion sensor connected to D5
#define LED_PIN D1              // LED connected to D1

const char* ssid = "Khilji Villa's";         // Replace with your Wi-Fi name
const char* password = "KhiljiVillas";       // Replace with your Wi-Fi password

ESP8266WebServer server(80);

bool lightState = false;           // Stores current LED state
bool manualMode = false;           // True if manual mode, false if motion mode
unsigned long lastMotionTime = 0;
const unsigned long autoOffDelay = 5000; // 5 seconds

// Display the control page
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Light Control</title>";
  html += "<style>";
  html += "body{font-family:sans-serif;text-align:center;background:#f0f0f0;padding:20px;}";
  html += "h2{color:#333;} .btn{padding:15px 30px;font-size:20px;margin:10px;border:none;cursor:pointer;border-radius:10px;}";
  html += ".on{background:green;color:white;} .off{background:red;color:white;} .motion{background:orange;color:white;}";
  html += "</style></head><body>";

  html += "<h2>Wi-Fi + Motion Light Control</h2>";
  html += "<p>Motion Status: <b>" + String(digitalRead(PIR_PIN) ? "Motion Detected" : "No Motion") + "</b></p>";
  html += "<p>Mode: <b>" + String(manualMode ? "Manual" : "Motion Sensor") + "</b></p>";
  html += "<p>Light is <b style='color:" + String(lightState ? "green" : "red") + "'>" + (lightState ? "ON" : "OFF") + "</b></p>";

  html += "<form action='/on' method='GET'><button class='btn on'>Turn ON</button></form>";
  html += "<form action='/off' method='GET'><button class='btn off'>Turn OFF</button></form>";
  html += "<form action='/motion' method='GET'><button class='btn motion'>Enable Motion Mode</button></form>";

  html += "<p style='margin-top:30px;'>IP: " + WiFi.localIP().toString() + "</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Handle ON button
void handleOn() {
  manualMode = true;
  lightState = true;
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Manual Mode: LED turned ON");
  handleRoot();
}

// Handle OFF button
void handleOff() {
  manualMode = true;
  lightState = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("Manual Mode: LED turned OFF");
  handleRoot();
}

// Handle Motion Mode button
void handleMotionMode() {
  manualMode = false;
  lightState = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("Switched to Motion Sensor Mode");
  handleRoot();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Define server routes
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/motion", handleMotionMode);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();

  // Only run motion logic if in motion mode
  if (!manualMode) {
    if (digitalRead(PIR_PIN) == HIGH) {
      if (!lightState) {
        Serial.println("Motion Detected: LED turned ON");
      }
      lightState = true;
      digitalWrite(LED_PIN, HIGH);
      lastMotionTime = millis();
    }

    // Turn off light after 5 seconds of no motion
    if (lightState && (millis() - lastMotionTime > autoOffDelay)) {
      lightState = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("No Motion: LED turned OFF (timeout)");
    }
  }
}
