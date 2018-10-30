#include <M5Stack.h>
#include <driver/dac.h>
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

#include "Tunes.h"
Tunes tunes;

const uint8_t PULSE_1_PIN  = 26;
const uint8_t PULSE_2_PIN  = 26;
const uint8_t NOISE_PIN    = 26;
const uint8_t TRIANGLE_PIN = 26;

void setup() {
  M5.begin();
  MIDI.begin();
  MIDI.turnThruOff();
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("8bit PSG SOUND");
  M5.Lcd.setCursor(160, 30);
  M5.Lcd.print("Synth");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(180, 60);
  M5.Lcd.print("by necobit");

//  dacWrite(25, 0); // Speaker OFF
//  ledcDetachPin(SPEAKER_PIN);
//  pinMode(SPEAKER_PIN, INPUT);
  tunes.init();
  M5.Speaker.setVolume(1);
  //  tunes.pinset(PULSE_1_PIN, PULSE_2_PIN, NOISE_PIN, TRIANGLE_PIN);

}

int p_output;

void loop() {
  tunes.resume();
  tunes.run();

  int ch, data1, data2;
  if (MIDI.read())
  {
    if (MIDI.getType() == midi::NoteOn)
    {
      ch = MIDI.getChannel();
      data1 = MIDI.getData1();
      data2 = MIDI.getData2();
      portENTER_CRITICAL(&Tunes::timerMux);
      tunes.noteon(ch, data1, data2);
      portEXIT_CRITICAL(&Tunes::timerMux);
    }
  }
  else if (MIDI.getType() == midi::NoteOff)
  {
    ch = MIDI.getChannel();
    data1 = MIDI.getData1();
    data2 = MIDI.getData2();
    portENTER_CRITICAL(&Tunes::timerMux);
    tunes.noteoff(ch, data1, data2);
    portEXIT_CRITICAL(&Tunes::timerMux);
  }
  M5.update();
}
