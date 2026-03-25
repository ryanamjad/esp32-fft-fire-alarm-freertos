#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//Timer Function
// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() 
{
  BaseType_t task_woken = pdFALSE;
  // Read multiple samples at once and calculate the sound pressure
  size_t num_bytes_read;
  
  esp_err_t err = i2s_read(I2S_PORT,
                   (char *)samples,
                    BLOCK_SIZE, // the doc says bytes, but its elements.
                     &num_bytes_read,
                     portMAX_DELAY); // no timeout
                     
  integerToFloat(samples, real, imag, SAMPLES);

  // A task notification works like a binary semaphore but is faster
   vTaskNotifyGiveFromISR(processing_task, &task_woken);  

  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

void vFFTData(void *pvparameters)
{
    while(1){
        Serial.println("Running FFT");
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // apply flat top window, optimal for energy calculations
        fft.Windowing(FFT_WIN_TYP_FLT_TOP, FFT_FORWARD);
        fft.Compute(FFT_FORWARD);    
        
        // calculate energy in each bin
        calculateEnergy(real, imag, SAMPLES);
        // sum up energy in bin for each octave
        sumEnergy(real, energy, 1, OCTAVES);
        // calculate loudness per octave + A weighted loudness
        float loudness = calculateLoudness(energy, aweighting, OCTAVES, 1.0);
        unsigned int peak = (int)floor(fft.MajorPeak());
        // detecting 1kHz and 1.5kHz
        if (detectFrequency(&fireAlarm, 15, peak, 45, 68, true))
        {
            Serial.println("Detected Fire Alarm");
            alarmStatus  = true; 
        }
        else
          alarmStatus  = false; 
        vTaskDelay(100); 
        }
    
}

void vAWSData(void *pvparameters)
{
    while(1){
        Serial.println("Sending Data to AWS");
        publishMessage();
        client.loop();
        vTaskDelay(200); 
        }
    
}

void vNNData(void *pvparameters)
{
    while(1){
      waterStatus = classifier.predictLabel(real);
      Serial.println(classifier.predictLabel(real));
      vTaskDelay(400); 
        }
    
}
