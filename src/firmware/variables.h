//Variables
#define SAMPLES 1024
const int BLOCK_SIZE = SAMPLES;
const i2s_port_t I2S_PORT = I2S_NUM_0;
boolean alarmStatus = false;
String waterStatus;
static int32_t samples[BLOCK_SIZE];

//Timer Variables
static const uint16_t timer_divider = 8;          // Divide 80 MHz by this
static const uint64_t timer_max_count = 1000000;  // Timer counts to this value
static hw_timer_t *timer = NULL;
static TaskHandle_t processing_task = NULL;

//FFT Variables
#define OCTAVES 9
static float real[SAMPLES];
static float imag[SAMPLES];
static arduinoFFT fft(real, imag, SAMPLES, SAMPLES);
static float energy[OCTAVES];
// A-weighting curve from 31.5 Hz ... 8000 Hz
static const float aweighting[] = {-39.4, -26.2, -16.1, -8.6, -3.2, 0.0, 1.2, 1.0, -1.1};
static unsigned int fireAlarm = 0;
static unsigned int sum = 0;
static unsigned int mn = 9999;
static unsigned int mx = 0;
static unsigned int cnt = 0;
static unsigned long lastTrigger[2] = {0, 0};
