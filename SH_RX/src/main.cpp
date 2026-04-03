#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> 
#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
const byte BUTTON_PIN = 0; // D5 is GPIO 14
const byte DNS_PORT = 53;
unsigned long portalTimer = 0;
const unsigned long TIMEOUT_MS = 300000; 

#define LED_PIN 5   // D1

#define SS   15     // D8
#define RST  2      // D4
#define DIO0 16     // D0

volatile bool APon = false;

// const char* ssid = "GalaxyA16";
// const char* password = "12345678";
const char* apiKey = "cd_shu_060326_p1xCZq";


const char* templateID = "103";                  // Template ID //moreinfo: https://circuitdigest.com/article free-sms-api-for-arduino-esp32-esp8266-nodemcu-raspberry-pi
const char* mobileNumber = "+919130788018";       // Mobile number (with country code)
const char* var1 = "Helmet Sensor";            // Variable 1
const char* var2 = "helmet of employee xyz"; 

volatile bool triggerPortal = false;
bool portalActive = false;

// FIX 1: Use commas for IPAddress
IPAddress apIP(192, 168, 4, 1); 

ESP8266WebServer server(80);
DNSServer dnsServer;

// --- FIX 2: Forward Declarations ---
// This tells the compiler these functions exist later in the file
void loadWifiAndConnect();
void startPortal();
void handleSave();
void LoRaTask();
// Interrupt Service Routine
void IRAM_ATTR handleButtonPress() {
  triggerPortal = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

  if(!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
  }
  
  loadWifiAndConnect();

//   WiFi.begin(ssid, password);


  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  SPI.begin();

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);

    
  }

  Serial.println("LoRa Receiver Initialized");

//   Serial.print("Connecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nConnected!");

}

void loop() {
  if (triggerPortal && !portalActive) {
    startPortal();
    triggerPortal = false;
  }

  if (portalActive) {
    dnsServer.processNextRequest();
    server.handleClient();

    // Auto-restart after 5 mins of no save activity
    if (millis() - portalTimer > TIMEOUT_MS) {
        Serial.println("Portal Timeout. Restarting...");
        delay(100);
        ESP.restart(); 
    }
  }

  LoRaTask();
}

// --- Function Definitions ---

void loadWifiAndConnect() {
  if (LittleFS.exists("/config.json")) {
    File file = LittleFS.open("/config.json", "r");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (!error) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(doc["ssid"].as<const char*>(), doc["pass"].as<const char*>());
        Serial.println("Connecting to saved WiFi...");
    }
    file.close();
  }
}

void startPortal() {
  Serial.println("Starting Captive Portal...");
  portalActive = true;
  portalTimer = millis();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Helmet-Config-Setup");
  
  dnsServer.start(DNS_PORT, "*", apIP);

  server.onNotFound([]() {
    if (LittleFS.exists("/index.html")) {
      File file = LittleFS.open("/index.html", "r");
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(404, "text/plain", "File Not Found. Did you upload LittleFS?");
    }
  });

  server.on("/save", HTTP_POST, handleSave);
  server.begin();
}

void handleSave() {
  JsonDocument doc;
  doc["ssid"] = server.arg("ssid");
  doc["pass"] = server.arg("pass");
  
  File file = LittleFS.open("/config.json", "w");
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to file");
  }
  file.close();
  
  server.send(200, "text/html", "Settings Saved! Helmet will now restart and connect.");
  delay(2000);
  ESP.restart();
}


void sendSMS() {
 if (WiFi.status() == WL_CONNECTED) {
   WiFiClientSecure client; // Use WiFiClientSecure for HTTPS connections
   client.setInsecure();    // Skip certificate validation (not secure but works for development)
   HTTPClient http;
   // Build the API URL with the template ID
   String apiUrl = "https://www.circuitdigest.cloud/api/v1/send_sms?ID=" + String(templateID);
   // Start the HTTPS connection with WiFiClientSecure
   http.begin(client, apiUrl);
   http.addHeader("Authorization", apiKey);
   http.addHeader("Content-Type", "application/json");
   // Create the JSON payload with SMS details
   String payload = "{\"mobiles\":\"" + String(mobileNumber) + "\",\"var1\":\"" + String(var1) + "\",\"var2\":\"" + String(var2) + "\"}";
   // Send POST request
   int httpResponseCode = http.POST(payload);
   // Check response
   if (httpResponseCode == 200) {
     Serial.println("SMS sent successfully!");
     Serial.println(http.getString());
   } else {
     Serial.print("Failed to send SMS. Error code: ");
     Serial.println(httpResponseCode);
     Serial.println("Response: " + http.getString());
   }
   http.end(); // End connection
 } else {
   Serial.println("WiFi not connected!");
 }
}void LoRaTask() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    Serial.print("Packet received. Size: ");
    Serial.println(packetSize);

    String receivedData = "";

    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    Serial.print("Message: ");
    Serial.println(receivedData);

    if (receivedData != "0") {
      digitalWrite(LED_PIN, HIGH);
      sendSMS();
      delay(200);
      digitalWrite(LED_PIN, LOW);
    }

   
  }
}