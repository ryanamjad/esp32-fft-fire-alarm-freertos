# Audio Processing of Fire-Alarm and WiFi Alert Transmission Using ESP32 and FreeRTOS

A real-time embedded system that performs **audio processing** of fire-alarm signals using **FFT** on an **ESP32 microcontroller** with **FreeRTOS** multitasking, and transmits WiFi alerts via **AWS IoT Core (MQTT)**. Additionally integrates an **SVM classifier** for edge-based sound classification of water-related audio events.

---

## Table of Contents

- [Problem Statement](#problem-statement)
- [How It Works](#how-it-works)
- [Tech Stack](#tech-stack)
- [System Architecture](#system-architecture)
- [ML Pipeline](#ml-pipeline)
- [Project Structure](#project-structure)
- [How to Run](#how-to-run)
- [Future Improvements](#future-improvements)
- [License](#license)

---

## Problem Statement

Traditional emergency detection systems rely on cloud-based processing, introducing latency and dependency on network connectivity. This project brings **intelligence to the edge** — performing real-time audio classification directly on a low-power ESP32 microcontroller. The system detects:

- **Fire alarms** via dual-tone frequency detection (1 kHz & 1.5 kHz)
- **Pouring water** and **dripping water** via SVM classification on FFT features
- Reports detections to AWS IoT Core over MQTT for remote monitoring

---

## How It Works

### Signal Processing Pipeline

1. **Audio Capture** — An INMP441 I2S MEMS microphone samples audio at 22,627 Hz with 32-bit resolution
2. **FFT Computation** — A 1024-point FFT with flat-top windowing extracts the frequency spectrum
3. **Feature Extraction** — Energy is computed per octave band (31.5 Hz – 8 kHz), then A-weighted for perceptual loudness:

$$L_A = 10 \cdot \log_{10}\left(\sum_{i=1}^{9} E_i \cdot 10^{W_i/10}\right)$$

where $E_i$ is the energy in octave $i$ and $W_i$ is the A-weighting correction factor.

4. **Classification** — The SVM model classifies the feature vector into one of the target classes
5. **Frequency Detection** — A sliding-window bit-count algorithm detects sustained dual-tone fire alarm signatures

### Real-Time Multitasking

The system leverages **FreeRTOS** to run three concurrent tasks:

| Task | Function | Period |
|:---:|:---:|:---:|
| FFT Processing | Compute FFT & detect fire alarm frequencies | 100 ms |
| ML Classification | SVM prediction on octave-band features | 400 ms |
| AWS Publishing | Send detection results via MQTT | 200 ms |

---

## Tech Stack

| Tool | Purpose |
|:---:|:---:|
| ESP32 | Edge microcontroller for real-time processing |
| INMP441 | I2S MEMS microphone for audio capture |
| Arduino IDE | Firmware development environment |
| FreeRTOS | Real-time multitasking OS |
| ArduinoFFT | Fast Fourier Transform computation |
| Scikit-learn | SVM model training |
| micromlgen | Export trained SVM to embedded C++ |
| AWS IoT Core | Cloud MQTT broker for remote monitoring |
| ArduinoJson | JSON serialization for MQTT payloads |
| Python | Audio preprocessing & ML training scripts |

---

## System Architecture

```
┌─────────────┐     I2S      ┌─────────────┐     FFT      ┌──────────────┐
│  INMP441    │────────────▶│   ESP32     │────────────▶│  Octave-Band │
│  Microphone │              │  (I2S RX)   │              │  Energy Calc │
└─────────────┘              └─────────────┘              └──────┬───────┘
                                                                 │
                                              ┌──────────────────┼──────────────────┐
                                              │                  │                  │
                                              ▼                  ▼                  ▼
                                     ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
                                     │  Fire Alarm  │  │  SVM Model   │  │  A-Weighted  │
                                     │  Freq Detect │  │  Classifier  │  │  Loudness    │
                                     │  (1k + 1.5k) │  │  (4 classes) │  │  Calculation │
                                     └──────┬───────┘  └──────┬───────┘  └──────────────┘
                                             │                 │
                                             ▼                 ▼
                                     ┌─────────────────────────────────┐
                                     │      AWS IoT Core (MQTT)       │
                                     │  Topic: esp32/pub              │
                                     │  {Fire Alarm: T/F, Water: ...} │
                                     └─────────────────────────────────┘
```

---

## ML Pipeline

The classification model is trained offline and deployed to the ESP32:

1. **Data Collection** — Audio samples from the ESC-50 dataset are categorized into pouring water, dripping water, and others
2. **Feature Extraction** — `wav2csv.py` converts `.wav` files into CSV feature vectors
3. **Model Training** — `classifierml.py` trains an SVM with a polynomial kernel (`degree=1`, `gamma=0.001`, `C=1000`)
4. **Code Generation** — `micromlgen` exports the trained SVM as a C++ header file (`model.h`) for direct inclusion in the firmware

---

## Project Structure

```
esp32-fft-fire-alarm-freertos/
├── README.md                          # Project documentation
├── LICENSE                            # MIT License
├── .gitignore                         # Git ignore rules
├── requirements.txt                   # Python dependencies
├── src/
│   ├── firmware/                      # ESP32 Arduino firmware
│   │   ├── AudioProcessing.ino        # Main sketch — setup, WiFi, task creation
│   │   ├── arduinoFFT.h               # FFT library header
│   │   ├── arduinoFFT.cpp             # FFT library implementation
│   │   ├── awsCredentials.h.example   # Template for AWS IoT credentials
│   │   ├── configWiFi.h               # WiFi & MQTT connectivity
│   │   ├── configRTOS.h               # FreeRTOS task definitions (FFT, AWS, ML)
│   │   ├── configI2S.h                # I2S microphone initialization
│   │   ├── functionsFFT.h             # DSP functions (energy, loudness, freq detect)
│   │   ├── model.h                    # SVM classifier (auto-generated from scikit-learn)
│   │   └── variables.h                # Global variables & FFT buffers
│   └── ml/                            # Python ML training scripts
│       ├── classifierml.py            # Train SVM & export to C++ via micromlgen
│       └── wav2csv.py                 # Convert audio files to CSV feature vectors
└── images/                            # Architecture diagrams & results
```

---

## How to Run

### Option 1: Flash ESP32 Firmware

1. Clone this repository:
   ```bash
   git clone https://github.com/ryanamjad/esp32-fft-fire-alarm-freertos.git
   cd esp32-fft-fire-alarm-freertos
   ```

2. Set up AWS IoT credentials:
   ```bash
   cp src/firmware/awsCredentials.h.example src/firmware/awsCredentials.h
   ```
   Edit `awsCredentials.h` with your WiFi credentials, AWS IoT endpoint, device certificate, and private key.

3. Open `src/firmware/AudioProcessing.ino` in **Arduino IDE**.

4. Install required libraries:
   - `ArduinoJson`
   - `PubSubClient`

5. Select **ESP32 Dev Module** as the board and flash.

### Option 2: Retrain the ML Model

1. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Prepare audio data — place `.wav` files in a directory and update the paths in `src/ml/wav2csv.py`.

3. Generate feature CSVs:
   ```bash
   python src/ml/wav2csv.py
   ```

4. Train and export the SVM model:
   ```bash
   python src/ml/classifierml.py
   ```

5. Replace `src/firmware/model.h` with the generated output.

---

## Future Improvements

- Add more sound classes (glass breaking, smoke detector variants, speech)
- Implement on-device model updating via OTA firmware updates
- Add a local display (OLED) for real-time status without cloud dependency
- Explore TensorFlow Lite Micro for deep learning on the edge
- Implement power management for battery-operated deployment
- Add data logging to SD card for offline analysis

---

## License

MIT License — see [LICENSE](LICENSE) for details.

> **Note:** This project demonstrates edge computing principles for real-time audio classification on resource-constrained microcontrollers, combining DSP, embedded ML, and IoT cloud integration.
