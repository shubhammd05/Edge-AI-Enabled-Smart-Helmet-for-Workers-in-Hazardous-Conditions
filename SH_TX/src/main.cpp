#include <SPI.h>
#include <LoRa.h>
#include<freertos/FreeRTOS.h>
#include <auxiliary_process.h>
#define BUTTON_PIN 13
#define SS 5
#define RST 4
#define DIO0 26

// volatile bool buttonpressed = false;
// volatile bool buttonState = HIGH;

// void IRAM_ATTR handleButtonInterrupt() {
//     buttonState = digitalRead(BUTTON_PIN);
//     buttonpressed = true;
// }

// void setup() {

//     Serial.begin(9600);

//     pinMode(BUTTON_PIN, INPUT_PULLUP);

//     // SPI pins (SCK, MISO, MOSI, SS)
//     SPI.begin(18, 19, 23, 5);

//     LoRa.setPins(SS, RST, DIO0);

//     if (!LoRa.begin(433E6)) {
//         Serial.println("LoRa initialization failed!");
//         while (1);
//     }

//     Serial.println("LoRa Transmitter Initialized.");

//     attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
// }

// void loop() {

//         if (buttonpressed) {

//             Serial.println("Button Pressed! Sending LED_ON");

//             LoRa.beginPacket();
//             LoRa.print("LED_ON");
//             LoRa.endPacket();
//             delay(200);
//             LoRa.beginPacket();
//             LoRa.print("LED_OFF");
//             LoRa.endPacket();
//             buttonpressed = false;
//             delay(500);
//         } else {

//             //Serial.println("Button Released! Sending LED_OFF");

//             LoRa.beginPacket();
//             LoRa.print("LED_OFF");
//             LoRa.endPacket();
//         }
//     }
#include <Arduino.h>

// Calibration derived from your provided image
const int xZeroG = 1816; 
const int zOneG  = 2221; 
const int unitsPerG = zOneG - xZeroG; // ~405 units = 9.8 m/s^2

// Hardware Pins (Move Y from 2 to 34!)
const int xPin = 15; 
const int yPin = 34; // CHANGED from 2 to avoid LED interference
const int zPin = 4;  

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db); // Ensures 0-3.3V range
}

void loop() {
  // Read and apply offsets based on your flat-table data
  float accX = ((float)(analogRead(xPin) - xZeroG) / unitsPerG) * 9.80665;
  float accY = ((float)(analogRead(yPin) - xZeroG) / unitsPerG) * 9.80665; // Uses X offset as baseline
  float accZ = ((float)(analogRead(zPin) - xZeroG) / unitsPerG) * 9.80665;

  // Output to match dataset magnitude (Acc_X, Acc_Y, Acc_Z)
  Serial.print(accX, 6); Serial.print("\t");
  Serial.print(accY, 6); Serial.print("\t");
  Serial.println(accZ, 6);
HelloWorld(accX, accY, accZ);

  delay(40); // 50Hz sampling
} 