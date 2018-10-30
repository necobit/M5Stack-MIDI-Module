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
    static volatile uint16_t d[3];
    static volatile uint16_t voice[3];
    static volatile uint16_t bnno[3];
    static volatile uint8_t duty_table[4][8];
    static volatile uint8_t p1_wave_index;
    static volatile uint8_t duty_point;
    static volatile uint16_t counter;
    static unsigned long tones[128];
    static int SineValues[256];
    static int PulseValues[4][256];
    static hw_timer_t* timer;

    void noteon(uint8_t mch, uint8_t nno, uint8_t vel);
    void noteoff(uint8_t mch, uint8_t nno, uint8_t vel);
    void sample_audio();
    void render_audio();

    void init();
    void pinset(uint8_t p1_p = 12, uint8_t p2_p = 14, uint8_t n_p = 26, uint8_t t_p = 27);
    void static IRAM_ATTR onTimer();
    void run();
    void pause();
    void resume();

    const uint32_t NES_APU_FREQ =  1789773 / 2 / 2; // APU is half speed of NES CPU, and we are running half the resolution of that to stay light.
    const uint32_t cycle_period = F_CPU / NES_APU_FREQ;

    const uint16_t audio_rate = 44100;
    const uint32_t audio_period = 80000000 / audio_rate;
    uint32_t next_audio = 0;

    uint32_t next_cycle = 0;
    uint32_t cpu_cycles = 0;
    uint32_t apu_cycles = 0;

    uint32_t t_last = 0;
    uint32_t cycles_delta = 0;
    uint32_t cycles_so_far = 0;

    const uint8_t audio_divisor = 2;
    uint8_t audio_counter = 0;




    const uint16_t noise_table[16] = {
      4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
    };

    const uint8_t tri_table[32] = {
      15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };

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
    uint8_t p2_wave_index = 1;
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

    uint32_t vgm_index = 0;
    uint16_t vgm_wait = 0;
    bool NES_PLAYING = false;

    uint32_t VGM_EOF_OFFSET = 0;
    uint32_t VGM_TOTAL_NUM_SAMPLES = 0;
    uint32_t VGM_RATE = 0;
    uint32_t VGM_DATA_OFFSET = 0;
    uint32_t VGM_NES_APU_CLOCK = 0;

};

#endif
