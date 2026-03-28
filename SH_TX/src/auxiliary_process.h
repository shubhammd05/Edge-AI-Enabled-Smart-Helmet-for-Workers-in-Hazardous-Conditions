#include <preprocessing_rf.cpp>
#include <Arduino.h>
#include <RandomForestModel.h>
#include <definitions.h>
// Instantiate the objects
FallPreprocessor preprocessor;
Eloquent::ML::Port::RandomForest svm; // Your ported model header

// Counter for the 50-observation "Delta"
int observationDelta = 0;
const int TRIGGER_THRESHOLD = 50; 

// Timing variable for 25Hz (40ms)
unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL = 40;

void HelloWorld(float x, float y,float z) {
    // unsigned long currentTime = millis();

    // 1. Maintain 25Hz Sampling Frequency
    // if (currentTime - lastSampleTime >= SAMPLE_INTERVAL) {
    //     lastSampleTime = currentTime;

        // Fetch your raw accelerometer data (Example values)
        // float x = readRawX(); 
        // float y = readRawY();
        // float z = readRawZ();

        // 2. Add sample to the Preprocessor's circular buffer
        preprocessor.addSample(z,x, y);
        
        // Increment our sliding window delta counter
        observationDelta++;

        // 3. The 50-observation Sliding Window Trigger
        if (observationDelta >= TRIGGER_THRESHOLD) {
            observationDelta = 0; // Reset delta counter

            // Only proceed if the 500-sample window is completely full
            if (preprocessor.isReady()) {
                float standardizedFeatures[5];

                // 4. Calculate and Standardize features
                preprocessor.getFeatures(standardizedFeatures);

                // 5. Predict using the SVM model
                int result = svm.predict(standardizedFeatures);
                Serial.printf("Result=%d",result);
                // Handle the output
                if (result == 1) {
                    // Trigger Buzzer/Override sequence
                    Serial.println("FALL_____________________________");
                    digitalWrite(BUZZER_PIN, HIGH);
                    delay(500);
                    digitalWrite(BUZZER_PIN, LOW);
                }
            }
        }
    }
