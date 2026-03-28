#include <MQUnifiedsensor.h>

#define BOARD "ESP32"
#define PIN 35 
#define TYPE "MQ-135"
#define VOLTAGE_RESOLUTION 3.3
#define ADC_RESOLUTION 12

// Thresholds based on OSHA PEL (Permissible Exposure Limits)
const float THRESHOLD_CO   = 50.0;   // 50 ppm
const float THRESHOLD_CO2  = 5000.0; // 5000 ppm
const float THRESHOLD_NH3  = 50.0;   // 50 ppm

MQUnifiedsensor MQ135(BOARD, VOLTAGE_RESOLUTION, ADC_RESOLUTION, PIN, TYPE);

volatile int danger = 0;

void checkCO() {
  MQ135.setA(605.18); MQ135.setB(-3.937); // CO Coefficients
  float coPPM = MQ135.readSensor();
  Serial.println(coPPM);
  if (coPPM > THRESHOLD_CO) danger = 1;
}

void checkCO2() {
  MQ135.setA(110.47); MQ135.setB(-2.862); // CO2 Coefficients
  float co2PPM = MQ135.readSensor() + 400; // Add atmospheric baseline
  Serial.println(co2PPM);
  if (co2PPM > THRESHOLD_CO2) danger = 2;
}

void checkNH3() {
  MQ135.setA(102.2); MQ135.setB(-2.473); // NH3 Coefficients
  float nh3PPM = MQ135.readSensor();
  Serial.println(nh3PPM);
  if (nh3PPM > THRESHOLD_NH3) danger = 3;
}