//#include <Arduino.h>
#include <math.h>

class FallPreprocessor {
private:
    static const int WINDOW_SIZE = 500;
    float bufX[WINDOW_SIZE], bufY[WINDOW_SIZE], bufZ[WINDOW_SIZE], bufSMV[WINDOW_SIZE];
    int head = 0;
    bool bufferFull = false;

    // YOUR PYTHON SCALING CONSTANTS (Replace these with your actual results!)
    float means[5] = {0.443477, 0.126955, 0.194573, 0.119403, 0.116190}; 
    float scales[5] = {0.298364, 0.115516, 0.149218, 0.065824, 0.098027};

public:
    void addSample(float x, float y, float z) {
        float smv = sqrt(x*x + y*y + z*z);
        
        bufX[head] = x;
        bufY[head] = y;
        bufZ[head] = z;
        bufSMV[head] = smv;

        head = (head + 1) % WINDOW_SIZE;
        if (head == 0) bufferFull = true;
    }

    bool isReady() { return bufferFull; }

    void getFeatures(float* featureArray) {
        float sumX = 0, sumY = 0, sumZ = 0, sumSMV = 0;
        float sqSumX = 0, sqSumY = 0, sqSumZ = 0;
        float minSMV = 100.0; // High initial value

        for (int i = 0; i < WINDOW_SIZE; i++) {
            // Stats for Standard Deviation
            sumX += bufX[i]; sqSumX += bufX[i] * bufX[i];
            sumY += bufY[i]; sqSumY += bufY[i] * bufY[i];
            sumZ += bufZ[i]; sqSumZ += bufZ[i] * bufZ[i];
            
            // Stats for SMV Mean/Min
            sumSMV += bufSMV[i];
            if (bufSMV[i] < minSMV) minSMV = bufSMV[i];
        }

        // 1. Calculate Raw Features
        float raw[5];
        raw[0] = minSMV;
        raw[1] = sqrt((sqSumZ / WINDOW_SIZE) - pow(sumZ / WINDOW_SIZE, 2)); // Std Z
        raw[2] = sqrt((sqSumY / WINDOW_SIZE) - pow(sumY / WINDOW_SIZE, 2)); // Std Y
        raw[3] = sqrt((sqSumX / WINDOW_SIZE) - pow(sumX / WINDOW_SIZE, 2)); // Std X
        raw[4] = sumSMV / WINDOW_SIZE; // Mean SMV

        // 2. Standardize (The crucial step!)
        for (int i = 0; i < 5; i++) {
            featureArray[i] = (raw[i] - means[i]) / scales[i];
        }
    }
};