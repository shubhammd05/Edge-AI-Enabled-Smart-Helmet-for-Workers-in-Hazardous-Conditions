#include <SPI.h>
#include <LoRa.h>
#include<freertos/FreeRTOS.h>
#include <auxiliary_process.h>
#include <Arduino.h>
#include <functions.h>

#define BUTTON_PIN 13
#define SS 5
#define RST 4
#define DIO0 26



// Calibration derived from your provided image
const int xZeroG = 1816; 
const int zOneG  = 2221; 
const int unitsPerG = zOneG - xZeroG; // ~405 units = 9.8 m/s^2 



void IRAM_ATTR handleButtonInterrupt() {
  delay(500);
   if(danger == 0){
    danger = 4;
   }
   else{
    danger = 0;
    LoRa.beginPacket();
    LoRa.print(String(danger));
    LoRa.endPacket();
   }
}



TaskHandle_t LoRaTaskHandle = NULL;
TaskHandle_t AccelTaskHandle = NULL;
TaskHandle_t TempTaskHandle = NULL;


void vGasMonitoringTask(void *pvParameters) {
  MQ135.init();
  // Note: Calibration logic should be here (MQ135.calibrate)
  // Inside your Gas Task or Setup
MQ135.setRegressionMethod(1); // _PPM_ =  a*ratio^b
MQ135.init(); 

Serial.print("Calibrating MQ-135, please wait...");
float calcR0 = 0;
for(int i = 1; i<=10; i ++) {
  MQ135.update(); 
  calcR0 += MQ135.calibrate(3.6); // 3.6 is the ratio for MQ135 in clean air
}
MQ135.setR0(calcR0/10);
Serial.println("Done!");
  while (danger == 0) {
    MQ135.update(); // Update ADC reading once for all functions
    
    

    // Call the three functions sequentially
    checkCO();
    checkCO2();
    checkNH3();
    
    // 500ms delay as requested
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
void vLoRaCommunicationTask(void *pvParameters) {
  for (;;) {
    if (danger != 0) {
      Serial.println("Sending distress signal");
      
      LoRa.beginPacket();
      LoRa.print(String(danger));
      LoRa.endPacket();

      vTaskDelay(pdMS_TO_TICKS(2000)); // Non-blocking delay

       
    } 
    
    }

    // Small delay to prevent this task from hogging the CPU (Watchdog protection)
    vTaskDelay(pdMS_TO_TICKS(50)); 
  }

void vTemperatureTask(void *pvParameters) {
    // Configure ADC for Pin 13
    analogSetPinAttenuation(TEMP_PIN, ADC_0db);  
    int temp_count = 0;
    float tempC;
    while (danger == 0) {
        // 1. Read Analog Value
        uint32_t mv = analogReadMilliVolts(TEMP_PIN);
  
  // Convert to Celsius (10mV = 1 degree)
      tempC = mv / 10.0;
Serial.println(tempC);
        // 4. Check Safety Threshold
        if (tempC > TEMP_THRESHOLD) {
            temp_count++;
            Serial.printf("!!! OVERHEAT WARNING: %.2f C !!!\n", tempC);
        } 

        if(temp_count >=5){
          danger = ALERT_TEMP;
        }

        // 5. Non-blocking delay (Check every 1 second)
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


void vAccelerometerTask(void *pvParameters) {
    // Ensure ADC is configured for the 3.3V range
    analogSetAttenuation(ADC_11db); 

    while(danger == 0) {
        // 1. Read Raw ADC Data
        int rawX = analogRead(xPin);
        int rawY = analogRead(yPin);
        int rawZ = analogRead(zPin);

        // 2. Convert to m/s^2 to match Xsens dataset magnitude 
        float accX = ((float)(rawX - xZeroG) / unitsPerG) * 9.80665;
        float accY = ((float)(rawY - xZeroG) / unitsPerG) * 9.80665;
        float accZ = ((float)(rawZ - xZeroG) / unitsPerG) * 9.80665;

        // 3. Debug Output (Optional: remove for production to save CPU)
        Serial.printf("%.6f\t%.6f\t%.6f\n", accX, accY, accZ);

        // 4. Run Fall Detection Logic (TinyML/SVM)
        HelloWorld(accX, accY, accZ);

        // 5. Precise Timing: 40ms delay = 25Hz
        // If your model needs the full 50Hz/100Hz from the Xsens kit, 
        // change this to pdMS_TO_TICKS(20)[cite: 94].
        vTaskDelay(pdMS_TO_TICKS(40)); 
    }
}

 void setup() {
      Serial.begin(115200);
  analogSetAttenuation(ADC_11db); // Ensures 0-3.3V range


    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // SPI pins (SCK, MISO, MOSI, SS)
    SPI.begin(18, 19, 23, 5);

    LoRa.setPins(SS, RST, DIO0);

    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa initialization failed!");
        while (1);
    }

    Serial.println("LoRa Transmitter Initialized.");

    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
    xTaskCreatePinnedToCore(
    vGasMonitoringTask,   // Function to run
    "GasTask",            // Name
    4096,                 // Stack size (Increased to prevent crash)
    NULL,                 // Parameter
    1,                    // Priority
    NULL,                 // Task handle
    1                     // Core 1
  );

  xTaskCreatePinnedToCore(
    vLoRaCommunicationTask, // Task function
    "LoRaTask",             // Name
    4096,                   // Stack size (LoRa needs a bit more than 2k)
    NULL,                   // Parameter
    2,                      // Priority (Higher than Gas monitor if alerts are critical)
    &LoRaTaskHandle,        // Handle
    1                       // Pin to Core 0 (Radio tasks usually sit on Core 0)
  );

  xTaskCreatePinnedToCore(
        vAccelerometerTask,   // Task function
        "FallDetection",      // Task Name
        8192,                 // Stack Size in Bytes (8KB)
        NULL,                 // Task input parameter
        3,                    // Priority (Higher priority for safety events)
        &AccelTaskHandle,     // Task handle
        0                     // Pin to Core 1 (Application Core)
    );

    xTaskCreatePinnedToCore(
        vTemperatureTask,
        "TempMonitor",
        4096,
        NULL,
        2,             // Priority 2 (Mid-level priority)
        &TempTaskHandle,
        1              // Core 1 (Sensor Core)
    );

}
  

void loop(){}