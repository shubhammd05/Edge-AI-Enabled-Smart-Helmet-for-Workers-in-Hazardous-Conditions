#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> 
#include <SPI.h>
#include <LoRa.h>

#define LED_PIN 5   // D1

#define SS   15     // D8
#define RST  2      // D4
#define DIO0 16     // D0

const char* ssid = "GalaxyA16";
const char* password = "12345678";
const char* apiKey = "cd_shu_060326_p1xCZq";


const char* templateID = "103";                  // Template ID //moreinfo: https://circuitdigest.com/article free-sms-api-for-arduino-esp32-esp8266-nodemcu-raspberry-pi
const char* mobileNumber = "9130788018";       // Mobile number (with country code)
const char* var1 = "Helmet Sensor";            // Variable 1
const char* var2 = "helmet of employee xyz"; 

void sendSMS() {

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    String url = "https://www.circuitdigest.cloud/send_sms?";
    url += "ID=" + String(templateID);
    url += "&mobiles=" + String(mobileNumber);
    url += "&var1=" + String(var1);
    url += "&var2=" + String(var2);

    Serial.println("Request URL:");
    Serial.println(url);

    http.begin(client, url);
    http.addHeader("Authorization", "Bearer " + String(apiKey));

    int httpResponseCode = http.GET();

    Serial.print("Response Code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println(response);

    http.end();
  }
  else {
    Serial.println("WiFi not connected!");
  }
}
void setup() {

  Serial.begin(9600);
  WiFi.begin(ssid, password);


  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  SPI.begin();

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);

    
  }

  Serial.println("LoRa Receiver Initialized");

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  
}

void loop() {

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

    if (receivedData == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      sendSMS();
      delay(200);
    }

    if (receivedData == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
    }
  }
}