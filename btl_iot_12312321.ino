#include <WiFi.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Firebase configuration
#include "addons/TokenHelper.h" // Include Firebase token generation header
#include "addons/RTDBHelper.h"  // Include Firebase RTDB helper functions

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// PIR sensor pin
#define PIR_PIN 18
// LED pin
#define LED_PIN 21

// Firebase project credentials
#define FIREBASE_HOST "https://iot-24-f6d01-default-rtdb.asia-southeast1.firebasedatabase.app/" // Replace with your Realtime Database URL
#define FIREBASE_API_KEY "AIzaSyCQfO92aw6ReefMqZ7SSOgm24zSXHucbOQ" // Replace with your Firebase API key

FirebaseData fbdo;           // Firebase Data object
FirebaseAuth auth;           // Firebase authentication object
FirebaseConfig config;       // Firebase configuration object

// NTP Client configuration
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // UTC+7, refresh every 60s

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Init PIR sensor and LED pin
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Connect to WiFi
  WiFi.begin("android2887435", "39104185"); // Replace with your WiFi credentials
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  // Start NTP Client
  timeClient.begin();

  // Firebase initialization
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_HOST;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase authentication succeeded");
    config.token_status_callback = tokenStatusCallback; // Token generation callback
  } else {
    Serial.printf("Firebase authentication failed: %s\n", config.signer.signupError.message.c_str());
    return;
  }

  Firebase.begin(&config, &auth);
}

void loop() {
  // Check for motion
  int motion = digitalRead(PIR_PIN);

  // Turn on/off LED based on motion detection
  digitalWrite(LED_PIN, motion);

  // Update the display and send data to Firebase
  updateDisplay(motion);
  sendDataToRealtimeDatabase(motion);

  delay(1000); // Delay for stability
}

// Generate random ID
String generateRandomId(int length) {
  String id = "";
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  int charsetSize = sizeof(charset) - 1;

  for (int i = 0; i < length; i++) {
    id += charset[random(0, charsetSize)];
  }
  return id;
}

// Get the current time as a formatted string (yyyy-MM-dd HH:mm:ss)
String getCurrentTime() {
  timeClient.update(); // Refresh time from NTP server
  unsigned long epochTime = timeClient.getEpochTime();

  // Convert epoch time to yyyy-MM-dd HH:mm:ss
  char buffer[20];
  struct tm *timeinfo = gmtime((time_t *)&epochTime);/-strong/-heart:>:o:-((:-h snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

  return String(buffer);
}

// Send motion data to Firebase Realtime Database
void sendDataToRealtimeDatabase(int motion) {
  String path = "/motions/" + String(millis()); // Unique path for each entry
  FirebaseJson json;
  String randomId = generateRandomId(16); // Tạo ID ngẫu nhiên dài 16 ký tự
  json.set("id", randomId);               // Thêm ID vào dữ liệu JSON
  json.set("motion", motion == HIGH);     // Store motion status as boolean
  json.set("timestamp", "2024-11-21 10:30:00");        // Store timestamp

  // Push data to Firebase
  if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
    Serial.println("Data sent to Realtime Database");
  } else {
    Serial.printf("Error sending data: %s\n", fbdo.errorReason().c_str());
  }
}

// Update display with motion status (or log it to Serial)
void updateDisplay(int motion) {
  Serial.print("Motion: ");
  Serial.println(motion == HIGH ? "Detected" : "Not Detected");
}