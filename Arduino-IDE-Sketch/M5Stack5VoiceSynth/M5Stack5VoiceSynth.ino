//認知のバグ
//手弾きで和音を鳴らすと一つ音が鳴らないことがあります。

//setup()内のコメントアウトを外して、Tunes.hの
//  int DAC_OUT = 25;
//をint DAC_OUT = 26;に変更して
//Bottomモジュールに出ているGPIO25と26の間に50k程度の
//可変抵抗を挟むとある程度音量制御ができます。

#include <M5Stack.h>
#include <driver/dac.h> //Arduino-ESP32 driver
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

#include "Tunes.h"
Tunes tunes;

void setup() {
  M5.begin();
  dacWrite(25, 0); // Speaker OFF
  ledcDetachPin(SPEAKER_PIN);
  pinMode(SPEAKER_PIN, INPUT);
  MIDI.begin();
  MIDI.turnThruOff();

  tunes.init();
  M5.Speaker.setVolume(1);

  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("Simple 3Voice");
  M5.Lcd.setCursor(160, 30);
  M5.Lcd.print("Synth");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(180, 60);
  M5.Lcd.print("Powered by @ina_ani");
}

int vt[5];
int v;
int vn[5];

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
  else if (vn[3] == data1) {
    v = 3;
  }
  else if (vn[4] == data1) {
    v = 4;
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
