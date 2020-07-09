#include "Tunes.h"

volatile SemaphoreHandle_t Tunes::timerSemaphore;
portMUX_TYPE Tunes::timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t Tunes::isrCounter = 0;
volatile uint32_t Tunes::lastIsrAt = 0;
volatile uint16_t Tunes::osc1 = 0;
volatile uint16_t Tunes::osc2 = 0;
volatile uint16_t Tunes::osc3 = 0;
volatile uint16_t Tunes::osc4 = 0;
volatile uint16_t Tunes::osc5 = 0;
volatile uint16_t Tunes::d[5] = {0,0,0,0,0};
int Tunes::SineValues[256];
int Tunes::SquareValues[256];
hw_timer_t* Tunes::timer;
unsigned int Tunes::tones[] = {
  27,
  28,
  30,
  32,
  34,
  36,
  38,
  40,
  43,
  45,
  48,
  51,
  54,
  57,
  60,
  64,
  68,
  72,
  76,
  80,
  85,
  90,
  95,
  101,
  107,
  114,
  120,
  127,
  135,
  143,
  152,
  161,
  170,
  180,
  191,
  202,
  214,
  227,
  241,
  255,
  270,
  286,
  303,
  321,
  340,
  360,
  382,
  405,
  429,
  454,
  481,
  510,
  540,
  572,
  606,
  642,
  680,
  721,
  764,
  809,
  857,
  908,
  962,
  1020,
  1080,
  1144,
  1212,
  1284,
  1361,
  1442,
  1528,
  1618,
  1715,
  1817,
  1925,
  2039,
  2160,
  2289,
  2425,
  2569,
  2722,
  2884,
  3055,
  3237,
  3429,
  3633,
  3849,
  4078,
  4320,
  4577,
  4850,
  5138,
  5443,
  5767,
  6110,
  6473,
  6858,
  7266,
  7698,
  8156,
  8641,
  9155,
  9699,
  10276,
  10887,
  11534,
  12220,
  12947,
  13717,
  14532,
  15396,
  16312,
  17282,
  18310,
  19398,
  20552,
  21774,
  23069,
  24440,
  25894,
  27433,
  29065,
  30793,
  32624,
  34564,
  36619,
  38797,
  41104

};

void Tunes::onTimer() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&Tunes::timerMux);
  Tunes::isrCounter++;
  Tunes::lastIsrAt = millis();
  Tunes::osc1 += d[0];
  Tunes::osc2 += d[1];
  Tunes::osc3 += d[2];
  Tunes::osc4 += d[3];
  Tunes::osc5 += d[4];
  portEXIT_CRITICAL_ISR(&Tunes::timerMux);
  // Give a semaphore that we can check in the loop

  xSemaphoreGiveFromISR(Tunes::timerSemaphore, NULL);
  int out = 0;
  //out += (Tunes::osc1 >> 8);//((osc1 >> 15)&1 == 1)?0:200;
  //out += (Tunes::osc2 >> 8);//((osc2 >> 15)&1 == 1)?0:200;
  //out += (Tunes::osc3 >> 8);//((osc3 >> 15)&1 == 1)?0:200;
  out += Tunes::SineValues[(osc1 >> 8)];//((osc1 >> 15)&1 == 1)?0:200;
  out += Tunes::SineValues[(osc2 >> 8)];//((osc2 >> 15)&1 == 1)?0:200;
  out += Tunes::SineValues[(osc3 >> 8)];//((osc3 >> 15)&1 == 1)?0:200;
  out += Tunes::SineValues[(osc4 >> 8)];//((osc3 >> 15)&1 == 1)?0:200;
  out += Tunes::SineValues[(osc5 >> 8)];//((osc3 >> 15)&1 == 1)?0:200;
  //out += ((osc1 >> 15)&1 == 1)?0:200;
  //out += ((osc2 >> 15)&1 == 1)?0:200;
  //out += ((osc3 >> 15)&1 == 1)?0:200;
  int DAC_OUT = 26;
  if (d[0] == 0 && d[1] == 0 && d[2] == 0 && d[3] == 0 && d[4] == 0) {
    digitalWrite(DAC_OUT, LOW);
  } else {
    dacWrite(DAC_OUT, (out / 5 + 128));
  }
}

void Tunes::init() {
  float ConversionFactor = (2 * PI) / 256;  // convert my 0-255 bits in a circle to radians
  // there are 2 x PI radians in a circle hence the 2*PI
  // Then divide by 256 to get the value in radians
  // for one of my 0-255 bits.
  float RadAngle;                           // Angle in Radians
  // calculate sine values
  for (int MyAngle = 0; MyAngle < 256; MyAngle++) {
    RadAngle = MyAngle * ConversionFactor;           // 8 bit angle converted to radians
    Tunes::SineValues[MyAngle] = (sin(RadAngle) * 128); // get the sine of this angle and 'shift' up so
    // there are no negative values in the data
    // as the DAC does not understand them and would
    // convert to positive values.
    Tunes::SquareValues[MyAngle] = MyAngle < 128 ? 0 : 128;
  }
  Tunes::timerSemaphore = xSemaphoreCreateBinary();

  Tunes::timer = timerBegin(0, 80, true); // /80 prescale = 1us = 1/1000000s = 1MHz
  timerAttachInterrupt(Tunes::timer, &Tunes::onTimer, true);
  timerAlarmWrite(Tunes::timer, 50, true);
  timerAlarmEnable(Tunes::timer);

}

void Tunes::pause() {
  timerAlarmDisable(Tunes::timer);
}
void Tunes::resume() {
  timerAlarmEnable(Tunes::timer);
}

void Tunes::run() {
  // get semaphore to use GPIO registers (maybe?
  if (xSemaphoreTake(Tunes::timerSemaphore, 0) == pdTRUE) {
    uint32_t isrCount = 0, isrTime = 0;
    portENTER_CRITICAL(&Tunes::timerMux);
    // ==== critical section begin ====
    isrCount = Tunes::isrCounter;
    isrTime = Tunes::lastIsrAt;

    // ==== critical section end ====
    portEXIT_CRITICAL(&Tunes::timerMux);

  }
  // below script is safe?
}
