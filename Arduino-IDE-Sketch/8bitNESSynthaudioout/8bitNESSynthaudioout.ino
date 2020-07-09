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
//1,2,4chはCC7,11で音量制御ができます。
//CC6でピッチベンドのレンジを0-24で調整できます。
//CC75 0-64でディケイを調整できます。64は変化なし。
//スウィープはまだ未搭載です。
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
  MIDI.turnThruOn();
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

  Tunes::outpin = 25;
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(20, 200);
  M5.Lcd.print("OUT:25");
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(20, 160);
  M5.Lcd.print("Volume:30");

  dacWrite(25, 0);
  if (Tunes::outpin == 26) {
    ledcDetachPin(SPEAKER_PIN);
    pinMode(SPEAKER_PIN, INPUT);
  }
  tunes.init();
  M5.Speaker.setVolume(255);
  pinMode(5, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
}

int onpu;
boolean onpudraw;
int amp = 1;

void loop() {
  tunes.run();

  int ch, data1, data2;

  /*  if (MIDI.read() && MIDI.getChannel() == 10)
    {
      if (MIDI.getType() == midi::NoteOn)
      {
        data1 = MIDI.getData1();
        if (data1 == 60)
        digitalWrite(5,HIGH);
      }
        else if (MIDI.getType() == midi::NoteOff)
      {
        data1 = MIDI.getData1();
        if (data1 == 60)
        digitalWrite(5,LOW);
      }

    }
  */
/*
if (MIDI.read() && MIDI.getChannel() == 5)
{
  if (MIDI.getType() == midi::NoteOn)
  {
    data1 = MIDI.getData1();
    data2 = MIDI.getData2();
    if (data1 == 60) {
      digitalWrite(5, HIGH);
    }
  }
  else if (MIDI.getType() == midi::NoteOff)
  {
    data1 = MIDI.getData1();
    data2 = MIDI.getData2();
    if (data1 == 60) {
      digitalWrite(5, LOW);
    }
  }
}
 */
if (MIDI.read() && MIDI.getChannel() <= 4)
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
    if (data1 == 7) Tunes::vol[ch - 1] = data2;
    else if (data1 == 11) Tunes::exp[ch - 1] = data2;
    else if (data1 == 75) Tunes::decay[ch - 1] = data2;
    else if (data1 == 6 && data2 <= 24) Tunes::pbrange[ch - 1] = data2;
  }
  else if (MIDI.getType() == midi::PitchBend)
  {
    ch = MIDI.getChannel();
    data1 = MIDI.getData1();
    data2 = MIDI.getData2();
    portENTER_CRITICAL(&Tunes::timerMux);
    tunes.pbend(ch, data1, data2);
    portEXIT_CRITICAL(&Tunes::timerMux);
  }
}
if (M5.BtnA.wasReleased()) {
  if (Tunes::outpin == 26)
  {
    Tunes::outpin = 25;
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(20, 200);
    M5.Lcd.print("OUT:25");
  } else {
    Tunes::outpin = 26;
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(20, 200);
    M5.Lcd.print("OUT:26");
  }
}

if (M5.BtnB.wasReleased() && Tunes::M_vol > 0 ) {
  Tunes::M_vol --;
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(145, 160);
  M5.Lcd.print("   ");
  M5.Lcd.setCursor(145, 160);
  M5.Lcd.print(Tunes::M_vol);
}
if (M5.BtnC.wasReleased() && Tunes::M_vol < 99 ) {
  //amp = !amp;
  //digitalWrite(2,amp);
  Tunes::M_vol ++;
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(145, 160);
  M5.Lcd.print("   ");
  M5.Lcd.setCursor(145, 160);
  M5.Lcd.print(Tunes::M_vol);
}
M5.update();
}
