#include <math.h>

class FallPreprocessor {
private:
    static const int WINDOW_SIZE = 500;
    float bufX[WINDOW_SIZE], bufY[WINDOW_SIZE], bufZ[WINDOW_SIZE], bufSMV[WINDOW_SIZE];
    int head = 0;
    bool bufferFull = false;

    // IMPORTANT: Replace these with your NEW 13-feature scaling constants from Python
    // order: [SMV_min, SMV_max, SMV_mean, SMV_peak, Acc_X_std, Acc_Y_std, Acc_Z_std, 
    //         Acc_X_max, Acc_Y_max, Acc_Z_max, Acc_X_min, Acc_Y_min, Acc_Z_min]
   float means[13] = {5.140881f, 21.414075f, 9.987059f, 16.273194f, 3.219494f, 2.970961f, 2.316501f, 15.334318f, 13.028109f, 6.253001f, -4.716356f, -7.794830f, -7.409010f};
    float scales[13] = {2.720651f, 8.170833f, 0.228201f, 10.390165f, 1.818895f, 1.440472f, 1.554036f, 5.147305f, 9.291681f, 5.446575f, 9.941159f, 6.440987f, 5.457575f};
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
        // Statistical Accumulators
        float sumX = 0, sumY = 0, sumZ = 0, sumSMV = 0;
        float sqSumX = 0, sqSumY = 0, sqSumZ = 0;
        
        // Min/Max Initializers
        float minX = 100, minY = 100, minZ = 100, minSMV = 100;
        float maxX = -100, maxY = -100, maxZ = -100, maxSMV = -100;

        for (int i = 0; i < WINDOW_SIZE; i++) {
            float x = bufX[i]; float y = bufY[i]; float z = bufZ[i]; float smv = bufSMV[i];

            // Sums for Means and Standard Deviations
            sumX += x; sqSumX += x * x;
            sumY += y; sqSumY += y * y;
            sumZ += z; sqSumZ += z * z;
            sumSMV += smv;

            // Global Min/Max hunts
            if (x < minX) minX = x; if (x > maxX) maxX = x;
            if (y < minY) minY = y; if (y > maxY) maxY = y;
            if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
            if (smv < minSMV) minSMV = smv; if (smv > maxSMV) maxSMV = smv;
        }

        // 1. Calculate Raw Features (Order must match Python training exactly!)
        float raw[13];
        raw[0]  = minSMV;                                           // SMV_min
        raw[1]  = maxSMV;                                           // SMV_max
        raw[2]  = sumSMV / WINDOW_SIZE;                             // SMV_mean
        raw[3]  = maxSMV - minSMV;                                  // SMV_peak
        raw[4]  = sqrt((sqSumX / WINDOW_SIZE) - pow(sumX / WINDOW_SIZE, 2)); // Acc_X_std
        raw[5]  = sqrt((sqSumY / WINDOW_SIZE) - pow(sumY / WINDOW_SIZE, 2)); // Acc_Y_std
        raw[6]  = sqrt((sqSumZ / WINDOW_SIZE) - pow(sumZ / WINDOW_SIZE, 2)); // Acc_Z_std
        raw[7]  = maxX;                                             // Acc_X_max
        raw[8]  = maxY;                                             // Acc_Y_max
        raw[9]  = maxZ;                                             // Acc_Z_max
        raw[10] = minX;                                             // Acc_X_min
        raw[11] = minY;                                             // Acc_Y_min
        raw[12] = minZ;                                             // Acc_Z_min

        // 2. Standardize
        for (int i = 0; i < 13; i++) {
            featureArray[i] = (raw[i] - means[i]) / scales[i];
        }
    }
};