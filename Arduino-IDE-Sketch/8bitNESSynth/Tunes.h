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
    static volatile uint16_t d[4];
    static volatile uint16_t voice[4];
    static volatile uint16_t bnno[4];
    static volatile uint8_t vol[4];
    static volatile uint8_t decay[4];
    static volatile int32_t decay_counter[4];
    static volatile uint8_t atack[4];
    static volatile uint32_t atack_counter[4];
    static volatile uint8_t duty_table[4][8];
    static volatile uint8_t wave_index[2];

    static volatile int n_reg;
    static volatile int shortFreq;

    static volatile uint8_t tri_table[32];
    static volatile uint16_t noise_table[16];
    static volatile uint8_t duty_point;
    static volatile uint16_t counter;
    static unsigned long tones[128];
    static int TriValues[256];
    static int PulseValues[4][256];
    static int NoiseValues[256];
    
    static hw_timer_t* timer;
    static volatile uint8_t outpin;

    void noteon(uint8_t mch, uint8_t nno, uint8_t vel);
    void noteoff(uint8_t mch, uint8_t nno);
    void pchange(uint8_t mch, uint8_t patch);
    
    void sample_audio();
    void render_audio();

    void init();
    void pinset(uint8_t p1_p = 12, uint8_t p2_p = 14, uint8_t n_p = 26, uint8_t t_p = 27);
    void static IRAM_ATTR onTimer();
    void run();
    void pause();
    void resume();

  private:

    // Pulse 1 Variables

    uint8_t p1_output = 0;
    long p1_timer = 0;

    uint8_t p1_duty_index = 2;
    uint16_t p1_length_counter = 0;
    uint8_t p1_envelope_divider = 0;
    uint8_t p1_decay_counter = 0;
    uint8_t p1_volume = 15;
    uint8_t p1_channel = 0;
    uint8_t p1_pin = 25;

    // Pulse 2 Variables
    uint8_t p2_output = 0;
    int16_t p2_11_bit_timer = 0;
    uint16_t p2_length_counter = 0;
    uint8_t p2_envelope_divider = 0;
    uint8_t p2_decay_counter = 0;
    uint8_t p2_volume = 0;
    uint8_t p2_channel = 1;
    uint8_t p2_pin = 14;

    // Noise Variables
    uint8_t n_output = 0;
    int16_t n_timer = 0;
    uint16_t n_length_counter = 0;
    uint8_t n_envelope_divider = 0;
    uint8_t n_decay_counter = 0;
    uint8_t n_volume = 0;
    uint8_t n_xor = 0;
    uint16_t n_lsfr = 1;
    uint8_t n_channel = 2;
    uint8_t n_pin = 27;

    // Triangle Variables
    uint8_t t_output = 0;
    int16_t t_11_bit_timer = 0;
    uint8_t t_wave_index = 0;
    uint16_t t_length_counter = 0;
    uint16_t t_linear_counter = 0;
    uint8_t t_channel = 3;
    uint8_t t_pin = 26;

};

#endif
