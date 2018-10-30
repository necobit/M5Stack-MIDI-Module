//NES Like Synth ver. 0.2
//M5Stack用MIDIモジュールを利用したファミコン風音源です。
//https://necobit.base.shop/items/14047506
//MIDIモジュールはこちらで販売しています。
//
//シンセエンジンは@ina_aniさんのエンジンを参考に、
//https://github.com/inajob/o-bako-esp32
//NESの音源の仕組みはConnor Nishijimaさんのプログラムを参考にしました。
//https://github.com/connornishijima/Cartridge
//Lixie LabsのソースはGPLv3なので継承するか迷いましたが、
//変数の定義を除くとソースはほぼ残っていないのでMITライセンスとします。

#include <M5Stack.h>
#include <driver/dac.h>
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

#include "Tunes.h"
Tunes tunes;

void setup() {
  M5.begin();
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("8bit PSG SOUND");
  M5.Lcd.setCursor(160, 30);
  M5.Lcd.print("Synth");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(180, 60);
  M5.Lcd.print("by necobit");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(160, 70);
  M5.Lcd.print("Special Thanks");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(180, 80);
  M5.Lcd.print("@ina_ani");
  M5.Lcd.setCursor(180, 90);
  M5.Lcd.print("Lixie Labs");

  Tunes::outpin = 26;  //50kΩ程度の可変抵抗を持っている人はここを26に変更して↓
  dacWrite(Tunes::outpin, 0);
  ledcDetachPin(SPEAKER_PIN);   //ここのコメントアウトを↓
  pinMode(SPEAKER_PIN, INPUT);   //外して下さい。音量が下げられます。
  tunes.init();
  M5.Speaker.setVolume(1);

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
