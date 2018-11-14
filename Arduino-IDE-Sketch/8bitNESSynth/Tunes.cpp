#include "Tunes.h"
#include <cmath>

volatile SemaphoreHandle_t Tunes::timerSemaphore;
portMUX_TYPE Tunes::timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t Tunes::isrCounter = 0;
volatile uint32_t Tunes::lastIsrAt = 0;
volatile uint8_t Tunes::outpin = 25;

volatile uint16_t Tunes::osc[4] = {0, 0, 0, 0};
volatile uint16_t Tunes::d[4] = {0, 0, 0, 0};
volatile uint16_t Tunes::voice[4] = {0, 0, 0, 0};
volatile uint16_t Tunes::bnno[4] = {0, 0, 0, 0};
volatile uint8_t Tunes::vol[4] = {100, 100, 100, 100};
volatile uint8_t Tunes::exp[4] = {127, 127, 127, 127};
volatile uint8_t Tunes::velo[4] = {100, 100, 100, 100};
volatile float Tunes::chbend[4] = {1, 1, 1, 1};
volatile uint16_t Tunes::pold[4] = {1, 1, 1, 1};
volatile uint8_t Tunes::pbrange[4] = {2, 2, 2, 2};
volatile uint8_t Tunes::atack[4] = {64, 64, 64, 64};
volatile uint32_t Tunes::atack_counter[4] = {0, 0, 0, 0};
volatile uint8_t Tunes::decay[4] = {64, 64, 64, 64};
volatile int32_t Tunes::decay_counter[4] = {44100, 44100, 44100, 44100};
volatile uint16_t Tunes::counter = 0;
volatile uint8_t Tunes::wave_index[2] = {0, 0};

volatile int Tunes::n_reg = 0x8000;
volatile int Tunes::shortFreq = 0;

volatile uint8_t Tunes::duty_point = 0;
volatile uint8_t Tunes::duty_table [4][8] = {
  {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
  {0, 0, 0, 0, 0, 0, 1, 1}, // 25%
  {0, 0, 0, 0, 1, 1, 1, 1}, // 50%
  {1, 1, 1, 1, 1, 1, 0, 0}, // 25% (inv.)
};

volatile uint8_t Tunes::tri_table[32] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

volatile uint16_t Tunes::noise_table[16] = {
  4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

int Tunes::PulseValues[4][256];
int Tunes::TriValues[256];
int Tunes::NoiseValues[256];

hw_timer_t* Tunes::timer;

void Tunes::noteon(uint8_t mch, float nno, uint8_t vel) {
  voice[mch - 1] = nno;
  float freq = std::exp(nno * 0.05776226f + 2.101178f) * 2.945296f;
  d[mch - 1] = (uint16_t)(freq);
  decay_counter[mch - 1] = 88200;
  velo[mch - 1] = vel;
  /*
      Serial.print("CH");
      Serial.print(mch);
      Serial.print("ON");
      Serial.println(d[mch - 1]);
  */

}

void Tunes::noteoff(uint8_t mch, uint8_t nno) {
  if (voice[mch - 1] == nno) {
    /*
      Serial.print("CH");
      Serial.print(mch);
      Serial.print("OFF");
      Serial.println(voice[mch - 1]);
    */
//    portENTER_CRITICAL_ISR(&Tunes::timerMux);
    voice[mch - 1] = 0;
    d[mch - 1] = 0;
//    portEXIT_CRITICAL_ISR(&Tunes::timerMux);
  }
}

void Tunes::pchange(uint8_t mch, uint8_t patch) {
  if (mch <= 2 && patch <= 3) {
    wave_index[mch - 1] = patch;
  }
  if (mch == 4 && patch <= 2 ) {
    shortFreq = patch;
  }
}

void Tunes::pbend(uint8_t mch, uint8_t data1, uint8_t data2) {
  int fusion = data2 << 7;
  fusion = fusion + data1;
  float pb = (float)(fusion);
  if (fusion != pold[mch - 1]) {
    float freq = std::exp((voice[mch - 1] + ((pb - 8192) / 8192 * pbrange[mch - 1]))  * 0.05776226f + 2.101178f) * 2.945296f;
    boolean onoff = voice[mch - 1];
    d[mch - 1] = (uint16_t)(freq) * onoff;
    pold[mch - 1] = fusion;
  }
}

void Tunes::onTimer() {
  portENTER_CRITICAL_ISR(&Tunes::timerMux);
  Tunes::isrCounter++;
  Tunes::lastIsrAt = millis();
  for (int i = 0 ; i <= 4; i ++) {
    Tunes::osc[i] += d[i];
  }

  //Noise生成
  if (Tunes::d[3] != 0) {
    counter = counter + 1  ;
    if (counter > 127 - (voice[3] * 2)) {
      counter = 0;
      Tunes::n_reg >>= 1;
      Tunes::n_reg |= ((Tunes::n_reg ^ (Tunes::n_reg >> (shortFreq ? 6 : 1))) & 1) << 15;
    }
  }
  boolean nsw = d[3];

  //Decay計算

  for (int i = 0; i < 4; i ++) {
    if (decay[i] < 64 && d[i] != 0 && decay_counter[i] != 0) {
      decay_counter[i] = decay_counter[i] - (64 - decay[i]);
      if ( decay_counter[i] <= 0 ) {
        decay_counter[i] = 0;
      }
    }
  }

  //出力計算
  int out = 0;
  out += Tunes::PulseValues[wave_index[0]][(osc[0] >> 8)] * 0.8 * decay_counter[0] / 88200 * vol[0] / 127 * velo[0] / 127 * exp[0] / 127;
  out += Tunes::PulseValues[wave_index[1]][(osc[1] >> 8)] * 0.8 * decay_counter[1] / 88200 * vol[1] / 127 * velo[1] / 127 * exp[1] / 127;
  out += Tunes::TriValues[(osc[2] >> 8)] * decay_counter[2] / 88200;
  out += Tunes::n_reg & 1 * nsw * 128   * decay_counter[3] / 88200 * vol[3] / 127  * velo[3] / 127  * exp[3] / 127;

  if (d[0] == 0 && d[1] == 0 && d[2] == 0 && d[3] == 0) out = out * 0.9;

  //出力
  dacWrite(Tunes::outpin, (out/16));

  portEXIT_CRITICAL_ISR(&Tunes::timerMux);

  xSemaphoreGiveFromISR(Tunes::timerSemaphore, NULL);
}

void Tunes::init() {

  for (int p_select = 0; p_select < 4; p_select ++) {
    for (int MyAngle = 0; MyAngle < 256; MyAngle++) {
      Tunes::PulseValues[p_select][MyAngle] = duty_table[p_select][MyAngle / 32] < 1 ? 0 : 255;
      Tunes::TriValues[MyAngle] = (tri_table[MyAngle / 8] + 1) * 16 - 1;
      Tunes::NoiseValues[MyAngle] = (noise_table[MyAngle / 16]);
    }
  }
  Tunes::timerSemaphore = xSemaphoreCreateBinary();

  Tunes::timer = timerBegin(0, 80, true); // /80 prescale = 1us = 1/1000000s = 1MHz
  timerAttachInterrupt(Tunes::timer, &Tunes::onTimer, true);
  timerAlarmWrite(Tunes::timer, 45, true);
  timerAlarmEnable(Tunes::timer);
}

void Tunes::pause() {
  timerAlarmDisable(Tunes::timer);
}
void Tunes::resume() {
  timerAlarmEnable(Tunes::timer);
}

void Tunes::run() {
  if (xSemaphoreTake(Tunes::timerSemaphore, 0) == pdTRUE) {
    uint32_t isrCount = 0, isrTime = 0;

    portENTER_CRITICAL(&Tunes::timerMux);
    isrCount = Tunes::isrCounter;
    isrTime = Tunes::lastIsrAt;
    portEXIT_CRITICAL(&Tunes::timerMux);
  }
}
