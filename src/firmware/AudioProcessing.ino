#include <Arduino.h>
#include <driver/i2s.h> //I2S Library to read from 

#include "model.h"    //loading of model for ML Classification
Eloquent::ML::Port::SVM classifier;

//code dividinded into header files to structure it
#include "arduinoFFT.h"     //calculate FFT class
#include "variables.h"      //global variables declared
#include "functionsFFT.h"   //functions to calculate FFT
#include "configWiFi.h"     //connect to wifi
#include "configRTOS.h"     //ISR and Task functions
#include "configI2S.h"      //I2S configuration


void setup() {

  // Configure Serial
  Serial.begin(115200);
                      
  setupI2S();   //setup I2S MIC
  connectAWS(); //connect to WiFi and AWS IoT core
  
  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---Real Time Audio Processing Using FFT and ML---");

  // Start a timer to run ISR every 100 ms
  timer = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timer_max_count, true);
  timerAlarmEnable(timer);

  //Task Functions
  xTaskCreatePinnedToCore(vFFTData, "Peform FFT", 10240, NULL, 1, &processing_task, app_cpu); //perform FFT
  xTaskCreatePinnedToCore(vAWSData, "Send Data to AWS", 10240, NULL, 1, NULL, app_cpu); //Send data to AWS
  xTaskCreatePinnedToCore(vNNData, "check Neural Network", 10240, NULL, 1, NULL, app_cpu); //Word detection in Audio using ML
  

}

void loop() {
  delay(2);
}
