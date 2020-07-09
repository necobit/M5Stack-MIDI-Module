#include <Arduino.h>

#ifndef TUNES_H
#define TUNES_H

class Tunes
{
  public:
    static volatile SemaphoreHandle_t timerSemaphore;
    static portMUX_TYPE timerMux;

    static volatile uint32_t isrCounter;
    static volatile uint32_t lastIsrAt;
    static volatile uint16_t osc1;
    static volatile uint16_t osc2;
    static volatile uint16_t osc3;
    static volatile uint16_t osc4;
    static volatile uint16_t osc5;
    static volatile uint16_t d[5];
    static  unsigned int tones[128];
    static int SineValues[256];
    static int SquareValues[256];
    static hw_timer_t* timer;

    void init();
    void static IRAM_ATTR onTimer();
    void run();
    void pause();
    void resume();

};
#endif
