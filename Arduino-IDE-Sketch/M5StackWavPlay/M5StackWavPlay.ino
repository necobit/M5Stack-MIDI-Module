#pragma mark - Depend ESP8266Audio and ESP8266_Spiram libraries
/*
  cd ~/Arduino/libraries
  git clone https://github.com/earlephilhower/ESP8266Audio
  git clone https://github.com/Gianbacchio/ESP8266_Spiram

  Use the "Tools->ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
  Then upload the sketch normally.
  https://github.com/me-no-dev/arduino-esp32fs-plugin
*/
#include <driver/dac.h>
#include <M5Stack.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;

void setup()
{
  M5.begin();
  MIDI.begin();
  MIDI.turnThruOff();

  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("Simple Sampler");

  M5.Lcd.setCursor(10, 40);
  M5.Lcd.print("PUSH BUTTON A");

        M5.Lcd.setCursor(10, 60);
        M5.Lcd.print("Play");
        M5.update();
pinMode(25,INPUT);
//pinMode(26,OUTPUT);
//dac_output_enable(DAC_CHANNEL_2);　//26
dac_output_disable(DAC_CHANNEL_1);  // 25
        file = new AudioFileSourceSD("/cat4410016.wav");
        out = new AudioOutputI2S(0, 1);
        out->SetOutputModeMono(false);
        out->SetGain(0.5);
        wav = new AudioGeneratorWAV();
        wav->begin(file, out);

        while (wav->isRunning()) {
          if (!wav->loop()) wav->stop();
        }

}

void loop()
{

}
