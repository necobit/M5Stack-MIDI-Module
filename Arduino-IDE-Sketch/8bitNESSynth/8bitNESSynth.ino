//NES Like Synth ver. 0.35
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
//
//1,2ch パルス系 プログラムチェンジ1-4で 12.5%,25%,50%,75%を切り替えられます
//3ch 疑似三角波
//4ch ノイズ　プログラムチェンジ1 高周期ノイズ 2 低周期ノイズ
//CC75 0-64でディケイを調整できます。64は変化なし。
//ボリューム、ピッチベンド、スウィープ等はまだ未搭載です。
//
//認知の不具合
//音に周期的な変化がある
//減衰の消え際が急

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
  M5.Lcd.setCursor(47, 0);
  M5.Lcd.print("8bit NES Like");
  M5.Lcd.setCursor(200, 30);
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


//Catbot
  M5.Lcd.fillRect(230, 150, 89, 70 , 0xFF80);
  M5.Lcd.fillTriangle(240, 150, 255, 130, 270, 150, 0xFF80);
  M5.Lcd.fillTriangle(280, 150, 295, 130, 310, 150, 0xFF80);
  M5.Lcd.drawLine(250, 210, 295, 210, 0x0000);
  M5.Lcd.drawLine(250, 205, 250, 210, 0x0000);
  M5.Lcd.drawLine(295, 205, 295, 210, 0x0000);
  M5.Lcd.fillCircle(250, 180, 8 , 0xE8E4);
  M5.Lcd.fillCircle(290, 180, 20 , 0xE8E4);

  Tunes::outpin = 25;  //50kΩ程度の可変抵抗を持っている人はここを26に変更して↓
  dacWrite(25, 0);
//  ledcDetachPin(SPEAKER_PIN);   //ここのコメントアウトを↓
//  pinMode(SPEAKER_PIN, INPUT);   //外して下さい。音量が下げられます。
  tunes.init();
  M5.Speaker.setVolume(1);

}

int p_output;

void loop() {
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
    portENTER_CRITICAL(&Tunes::timerMux);
    tunes.noteoff(ch, data1);
    portEXIT_CRITICAL(&Tunes::timerMux);
  }
  else if (MIDI.getType() == midi::ProgramChange)
  {
    ch = MIDI.getChannel();
    data1 = MIDI.getData1();
    portENTER_CRITICAL(&Tunes::timerMux);
    tunes.pchange(ch, data1);
    portEXIT_CRITICAL(&Tunes::timerMux);
  }
  else if (MIDI.getType() == midi::ControlChange)
  {
    ch = MIDI.getChannel();
    data1 = MIDI.getData1();
    data2 = MIDI.getData2();
    if (data1 == 75) Tunes::decay[ch - 1] = data2;
  }

  M5.update();
}
