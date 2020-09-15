/// At max sample rate of 952 Hz
/// Delay sould be 1.050 ms
/// Capturing 952 in 1019 ms ~ 934 Hz
/// For live demo, capture 100 samples, repeatedly
/// For logging, capture 1000 samples, once
///
/// Pin connections:
/// CS_M    --> Pin 5
/// CS_AG   --> Pin 6
/// SDO_M   --> Pin 12 (SPI DIN/MISO)
/// SDO_AG  --> Pin 12 (SPI DIN/MISO)
/// SCL     --> Pin 13 (SPI SCK)
/// SDA     --> Pin 11 (SPI DOUT/MOSI)
/// VDD     --> 3.3V DO NOT EXCEED 3.3V!!
/// GND     --> GND
///
///
///
///
/// Created 7 Jan 2019
/// Modified 24 Jul 2019 (V03)
/// -Added realtime processing (freq, pk-pk)
/// 
///
/// @author Mustafa Ghazi



/*****************************************************************
LSM9DS1_Basic_SPI.ino
SFE_LSM9DS1 Library Simple Example Code - SPI Interface
Jim Lindblom @ SparkFun Electronics
Original Creation Date: April 29, 2015
https://github.com/sparkfun/LSM9DS1_Breakout

The LSM9DS1 is a versatile 9DOF sensor. It has a built-in
accelerometer, gyroscope, and magnetometer. Very cool! Plus it
functions over either SPI or I2C.

This Arduino sketch is a demo of the simple side of the
SFE_LSM9DS1 library. It'll demo the following:
* How to create a LSM9DS1 object, using a constructor (global
  variables section).
* How to use the begin() function of the LSM9DS1 class.
* How to read the gyroscope, accelerometer, and magnetometer
  using the readGryo(), readAccel(), readMag() functions and 
  the gx, gy, gz, ax, ay, az, mx, my, and mz variables.
* How to calculate actual acceleration, rotation speed, 
  magnetic field strength using the calcAccel(), calcGyro() 
  and calcMag() functions.
* How to use the data from the LSM9DS1 to calculate 
  orientation and heading.

Hardware setup: This example demonstrates how to use the
LSM9DS1 with an SPI interface. The pin-out is as follows:
  LSM9DS1 --------- Arduino
          CS_AG ------------- 9
          CS_M ------------- 10
          SDO_AG ----------- 12
          SDO_M ------------ 12 (tied to SDO_AG)
          SCL -------------- 13
          SDA -------------- 11
          VDD -------------- 3.3V
          GND -------------- GND

The LSM9DS1 has a maximum voltage of 3.6V. Make sure you power it
off the 3.3V rail! Signals going into the LSM9DS1, at least,
should be level shifted down to 3.3V - that's CSG, CSXM,
SCL, and SDA.

Better yet, use a 3.3V Arduino (e.g. the Pro or Pro Mini)!

Development environment specifics:
  IDE: Arduino 1.6.3
  Hardware Platform: Arduino Pro 3.3V
  LSM9DS1 Breakout Version: 1.0

This code is beerware. If you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, 
please buy us a round!

Distributed as-is; no warranty is given.
*****************************************************************/
// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////
// Use the LSM9DS1 class to create an object. [imu] can be
// named anything, we'll refer to that throught the sketch.
LSM9DS1 imu;

///////////////////////
// Example SPI Setup //
///////////////////////
// Define the pins used for our SPI chip selects. We're
// using hardware SPI, so other signal pins are set in stone.
#define LSM9DS1_M_CS  5 // Can be any digital pin
#define LSM9DS1_AG_CS 6  // Can be any other digital pin

////////////////////////////
// Sketch Output Settings //
////////////////////////////

#define MY_PI 3.14159
// for error checking, max no. of pts in pk-pk estimation array
#define N_PTS_MAX 200
// for error checking max no. of pts in zcr estimation array
#define N_ZCR_PTS_MAX 100
// if pk-pk is less than this threshold then there is no 
// vibration...we are just getting noise
#define PK_PK_NOISE_THRESHOLD 0.1

#define PRINT_SPEED 250 // 250 ms between prints

#define N_SAMPLES 200*1
float tVec[N_SAMPLES];
float yVec[N_SAMPLES];
volatile int n = 0; // to 

// On startup, blink pin 13 LED 'a' times
void blinkStartupSequence(int a);

// Configure accel for 16g and 952 samples/sec
void configureLSM9DS1();

/// @brief Compute the mean peak value, given a time vector and a y vector
int meanPeaks(float timeVector[], float yVector[], int numDataPts, float *meanVal);

/// @brief Compute the mean valley value, given a time vector and a y vector
int meanValleys(float timeVector[], float yVector[], int numDataPts, float *meanVal);

/// @brief Find positive zero crossing points given a time vector and a y vector
int pZeroCrossings (float timeVector[], float yVector[], int numDataPts, float threshold, float zcrTimesVector[]);

/// @brief Interpolate x coordinate given y coordinate and 2 points
float xInterpolate(float x1, float y1, float x2, float y2, float y);

/// @brief Compute average period from a vector of zero crossing time instances
float zcrToPeriod(float zcrVec[], int numDataPts);

/// @brief Generate test data points based on a sine wave
void generateSinWavePtsMillis(float peak, float valley, float freqHz, float dtMillis, int numDataPts, float timeVector[], float yVector[]);


void setup() 
{
  
  Serial.begin(115200);
  blinkStartupSequence(10); // blink LED 13 5 times
  configureLSM9DS1();
  
  
  // The above lines will only take effect AFTER calling
  // imu.begin(), which verifies communication with the IMU
  // and turns it on.
  if (!imu.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                  "work for an out of the box LSM9DS1 " \
                  "Breakout, but may need to be modified " \
                  "if the board jumpers are.");
    while (1)
      ;
  }
  delay(1000); // let the sensor readings settle down (>25 ms)
}

void loop()
{
  int i = 0;
  unsigned long startTime, endTime;
  float calcPeak, calcValley, calcPeriod, calcFreq;
    float zcr[N_SAMPLES/2];
    int nZCR = 0;
  // note down start time and log data
  startTime = millis();
  for(i=0; i<N_SAMPLES; i++) {
    imu.readAccel();    
    yVec[i] = imu.calcAccel(imu.az); 
    tVec[i] = (float)(millis() - startTime); // make sure never goes over 65000 ms
    delayMicroseconds(1031); // for 952 samples/sec (980us used previously)
    
  }
  // note down the end time
  endTime = millis();
  //Serial.print("time taken in ms: ");
  //Serial.println(endTime-startTime);

  // now compute and compare with test data
  meanPeaks(tVec, yVec, N_SAMPLES, &calcPeak);
  meanValleys(tVec, yVec, N_SAMPLES, &calcValley);
  nZCR = pZeroCrossings(tVec, yVec, N_SAMPLES, (calcPeak + calcValley)/2.0, zcr);

  if ( (nZCR > 0) && ((calcPeak-calcValley) > PK_PK_NOISE_THRESHOLD) ) {
    calcPeriod = zcrToPeriod(zcr, nZCR); // [ms]
    calcFreq = 1000.0/calcPeriod; // Hz
    //Serial.print("pk-pk (g): ");
    //Serial.print(n);
    Serial.print((float)endTime);
    Serial.print("\t");
    Serial.print(calcPeak-calcValley);
    //Serial.print(" Freq (Hz): ");
    Serial.print("\t");
    Serial.print(calcFreq);
    Serial.print("\n");
  } else {
    // Serial.print("no vibration!\n");
    Serial.print((float)endTime);
    Serial.print("\t");
    Serial.print("0.00");
    //Serial.print(" Freq (Hz): ");
    Serial.print("\t");
    Serial.print(0.00);
    Serial.print("\n");
  }
  calcPeak = 0; calcValley = 0; calcPeriod = 0; calcFreq = 0;

  
  //while(1); // comment out for live demo
  n++; // use for counting in the if conditional
  //delay(1000);
}

// On startup, blink pin 13 LED 'a' times
void blinkStartupSequence(int a) {
  int j;
  pinMode(13, OUTPUT);
  
  for(j=0; j<a; j++){
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
  }
  
}

// Configure accel for 16g and 952 samples/sec

void configureLSM9DS1(){

  // Before initializing the IMU, there are a few settings
  // we may need to adjust. Use the settings struct to set
  // the device's communication mode and addresses:
  imu.settings.device.commInterface = IMU_MODE_SPI;
  imu.settings.device.mAddress = LSM9DS1_M_CS;
  imu.settings.device.agAddress = LSM9DS1_AG_CS;

  // accel scale can be 2, 4, 8, or 16
  imu.settings.accel.scale = 16; // Set accel range to +/-16g
  imu.settings.gyro.scale = 2000; // Set gyro range to +/-2000dps
  imu.settings.gyro.sampleRate = 6; // set sample rate to 952 Hz (accel + gyro)
  imu.settings.accel.sampleRate = 6;
  
}


/// @brief Compute the mean peak value, given a time vector and a y vector
///
/// TODO: right side edge case?
///
/// @param timeVector array of time (t) elements [ms]
/// @param yVector array of corresponding y elements [g]
/// @param numDataPts number of elements in timeVector/yVector array
/// @param *meanVal the computed mean peak [g]
///
/// @return number of peaks (0 if none found or error)
///
int meanPeaks(float timeVector[], float yVector[], int numDataPts, float *meanVal) {

    if (numDataPts > N_PTS_MAX) return 0;
    float runningSum= 0;

    int i = 1; //overall counter
    int j = 0; // peak counter
    while (i < (numDataPts - 2)) {
        // get slope before i and after i
        //                        _
        // could be shaped /\ or / \

        float slope1 = (yVector[i] - yVector[i-1])/(timeVector[i] - timeVector[i-1]);
        float slope2 = (yVector[i] - yVector[i+1])/(timeVector[i] - timeVector[i+1]);
        float slope3 = (yVector[i+1] - yVector[i+2])/(timeVector[i+1] - timeVector[i+2]);
        // if slope changes from +ive to -ive, or from +ive to 0 to -ive
        if (
            ( (slope1 > 0.0) && (slope2 < 0.0) ) ||
            ( (slope1 > 0.0) && (slope2 == 0.0) && (slope3 < 0.0) )
            ){
            runningSum += yVector[i];
            j += 1;
        }
        i += 1;
    }
    // calculate mean peak if finite number of peaks
    if (j > 0) {
        *meanVal = runningSum/((float)j);
    } else {
        *meanVal = 0.0;
    }

    return j;
}


/// @brief Compute the mean valley value, given a time vector and a y vector
///
/// TODO: right side edge case?
///
/// @param timeVector array of time (t) elements [ms]
/// @param yVector array of corresponding y elements [g]
/// @param numDataPts number of elements in timeVector/yVector array
/// @param *meanVal the computed mean valley [g]
///
/// @return number of valleys (0 if none found or error)
///
int meanValleys(float timeVector[], float yVector[], int numDataPts, float *meanVal) {

    if (numDataPts > N_PTS_MAX) return 0;
    float runningSum = 0;

    int i = 1; //overall counter
    int j = 0; // valley counter
    while (i < (numDataPts - 2)) {
        // get slope before i and after i
        //
        // could be shaped \/ or \_/

        float slope1 = (yVector[i] - yVector[i-1])/(timeVector[i] - timeVector[i-1]);
        float slope2 = (yVector[i] - yVector[i+1])/(timeVector[i] - timeVector[i+1]);
        float slope3 = (yVector[i+1] - yVector[i+2])/(timeVector[i+1] - timeVector[i+2]);
        // if slope changes from -ive to +ive, or from -ive to 0 to +ive
        if (
            ( (slope1 < 0.0) && (slope2 > 0.0) ) ||
            ( (slope1 < 0.0) && (slope2 == 0.0) && (slope3 > 0.0) )
            ){
            runningSum += yVector[i];
            j += 1;
        }
        i += 1;
    }
    // calculate mean valley if finite number of valleys
    if (j > 0) {
        *meanVal = runningSum/((float)j);
    } else {
        *meanVal = 0.0;
    }

    return j;
}


/// @brief Find positive zero crossing points given a time vector and a y vector
///
/// TODO: right side edge case?
///
/// @param timeVector array of time (t) elements [ms]
/// @param yVector array of corresponding y elements [g]
/// @param numDataPts number of elements in timeVector/yVector array
/// @param threshold zero cross threshold for the yVector [g]
/// @param zcrTimesVector computed array of positive zero crossing points [ms]
///
/// @return number of elements in zcrTimesVector (0 if none found or error)
///
int pZeroCrossings (float timeVector[], float yVector[], int numDataPts, float threshold, float zcrTimesVector[]) {

    if (numDataPts > N_PTS_MAX) return 0;
    int i = 0; // overall counter
    int j = 0; // zcr counter
    while (i < (numDataPts - 2)) {
        // if value changes from below threshold to above threshold
        //  /         _/
        // /    or   /
        if ((yVector[i] <= threshold) && (yVector[i+1] > threshold) ){
            zcrTimesVector[j] = xInterpolate(timeVector[i], yVector[i], timeVector[i+1], yVector[i+1], threshold);
            j += 1;
        } else if ((yVector[i] < threshold) && (yVector[i+1] == threshold) && (yVector[i+2] > threshold)) {
            zcrTimesVector[j] = xInterpolate(timeVector[i], yVector[i], timeVector[i+1], yVector[i+1], threshold);
            j += 1;
        }
        i += 1;
    }
    return j;
}


/// @brief Interpolate x coordinate given y coordinate and 2 points
///
/// TODO: checks for infinity and so on
///
/// @param x1
/// @param y1
/// @param x2
/// @param y2
/// @param y
///
/// @return x coordinate
///
float xInterpolate(float x1, float y1, float x2, float y2, float y) {
    float x = 0.0;
    // if vertical line, x = x1 = x2
    if( (x2 - x1) == 0){
        return x1;
    }
    // otherwise interpolate as usual
    x = (y - y1)*(x2 - x1)/(y2 - y1) + x1;
    return x;
}


/// @brief Compute average period from a vector of zero crossing time instances
///
/// @param zcrVec array of zero crossing points [ms]
/// @param numDataPts number of elements in array
///
/// @return mean period over numDataPts [ms]
///
float zcrToPeriod(float zcrVec[], int numDataPts) {

    if (numDataPts > N_ZCR_PTS_MAX) return 0;
    float runningSum = 0.0;
    int i = 1; // starts at the second array element
    while (i < numDataPts){
        // at t_i, period is t_i - t_(i-1)
        runningSum += zcrVec[i] - zcrVec[i-1];
        i += 1;
    }
    // numDataPts elements in the time vector
    // but (numDataPts - 1) different periods obtained
    // so the the total is averaged over (numDataPts - 1)
    return (runningSum/(numDataPts - 1.0));
}


/// @brief Generate test data points based on a sine wave
///
/// @param peak desired peak/max value of the sine wave
/// @param valley desired valley/min value of the sine wave
/// @param freqHz desired frequency [Hz]
/// @param dtMillis desired time step [ms]
/// @param numDataPts desired number of data points to generate
/// @param timeVector computed array of time (t) elements [ms]
/// @param yVector computed array of corresponding amplitude
///
/// @return void
///
void generateSinWavePtsMillis(float peak, float valley, float freqHz, float dtMillis, int numDataPts, float timeVector[], float yVector[]) {

    int i;
    float freqRad = freqHz*MY_PI*2.0; // [rad/s]
    float amplitude = (peak - valley)/2.0;
    float offset = (peak + valley)/2.0;

    for(i=0; i<numDataPts; i++){
        timeVector[i] = dtMillis*(float)i; // [ms]
        // need to use seconds, not milliseconds here
        yVector[i] = amplitude*sin(freqRad*dtMillis/1000.0*(float)i) + offset; // A*sin(wt) + offset

    }

}
