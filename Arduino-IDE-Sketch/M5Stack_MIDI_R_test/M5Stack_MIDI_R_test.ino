#include <M5Stack.h>
#include <driver/dac.h> //Arduino-ESP32 driver
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

void setup() {
  M5.begin();
  dacWrite(25, 0); // Speaker OFF
  dac_output_disable( DAC_CHANNEL_1 );
  MIDI.begin();
  MIDI.turnThruOff();
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("MIDI Monitor");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.print("Control Number:");
  M5.Lcd.setCursor(10, 70);
  M5.Lcd.print("Value:");
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.print("Note Number:");
  M5.Lcd.setCursor(10, 160);
  M5.Lcd.print("Velocity:");
}

void loop() {
  int data1, data2, locateY1, locateY2;
  if (MIDI.read())
  {
    if (MIDI.getType() == midi::ControlChange)
    {
      locateY1 = 40;
      locateY2 = 70;
    }
    else if (MIDI.getType() == midi::NoteOn)
    {
      locateY1 = 130;
      locateY2 = 160;
    }
    else return;

    data1 = MIDI.getData1(); //CC No取得
    data2 = MIDI.getData2(); //CC 値を取得
    char disp1[4] = "";
    char disp2[4] = "";
    sprintf(disp1, "%03d", data1);
    sprintf(disp2, "%03d", data2);
    M5.Lcd.setCursor(200, locateY1);
    M5.Lcd.printf(disp1);
    M5.Lcd.setCursor(200, locateY2);
    M5.Lcd.printf(disp2);

  }
}
