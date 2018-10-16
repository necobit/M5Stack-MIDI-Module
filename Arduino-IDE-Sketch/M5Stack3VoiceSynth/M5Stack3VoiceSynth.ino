#include <M5Stack.h>
#include <driver/dac.h> //Arduino-ESP32 driver
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

#include "Tunes.h"
Tunes tunes;

void setup() {
  M5.begin();
  //   dacWrite(25, 0); // Speaker OFF
  //  ledcDetachPin(SPEAKER_PIN);
  //  pinMode(SPEAKER_PIN, INPUT);
  MIDI.begin();
  MIDI.turnThruOff();

  tunes.init();
  M5.Speaker.setVolume(1);

  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("ina 3Voice");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 40);

}

int vt[3];
int v;
int vn[3];

void noteon(int data1, int data2)
{
  if (vt[0] <= vt[1] and vt[0] <= vt[2]) {
    noteoff(vn[0]);
    vt[0] = millis();
    v = 0;
    vn[0] = data1;
  }
  else if (vt[1] <= vt[2]) {
    noteoff(vn[1]);
    vt[1] = millis();
    v = 1;
    vn[1] = data1;
  }
  else {
    noteoff(vn[2]);
    vt[2] = millis();
    v = 2;
    vn[2] = data1;
  }

  portENTER_CRITICAL(&Tunes::timerMux);
  Tunes::d[v] = (uint16_t)(tunes.tones[data1]);
  portEXIT_CRITICAL(&Tunes::timerMux);
}

void noteoff(int data1)
{
  if (vn[0] == data1) {
    v = 0;
  }
  else if (vn[1] == data1) {
    v = 1;
  }
  else if (vn[2] == data1) {
    v = 2;
  }
  else return;
  portENTER_CRITICAL(&Tunes::timerMux);
  Tunes::d[v] = 0;
  portEXIT_CRITICAL(&Tunes::timerMux);
  vt[v] = 0;
}

void loop() {
  tunes.resume();
  tunes.run();
  int data1, data2;
  if (MIDI.read())
  {
    if (MIDI.getType() == midi::NoteOn)
    {
      data1 = MIDI.getData1();
      data2 = MIDI.getData2();
      noteon(data1, data2);
    }
  }

  if (MIDI.getType() == midi::NoteOff)
  {
    data1 = MIDI.getData1();
    noteoff(data1);
  }

  M5.update();

}
