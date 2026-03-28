#define BUZZER_PIN 27
#define xPin 15 
#define yPin 34 // CHANGED from 2 to avoid LED interference
#define zPin 4
#define BUTTON_PIN 13
#define SS 5
#define RST RX2
#define DIO0 26
#include <MQUnifiedsensor.h>

// Sensor Setup
#define BOARD "ESP32"
#define PIN 35 
#define TYPE "MQ-135"
#define VOLTAGE_RESOLUTION 3.3
#define ADC_RESOLUTION 12

#define TEMP_PIN 32
#define TEMP_THRESHOLD 60.0    // Danger threshold in Celsius
#define ALERT_TEMP 5