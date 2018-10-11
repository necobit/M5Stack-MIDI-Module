#include <M5Stack.h>
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

void setup() {
  M5.begin();
  dacWrite(25, 0); // Speaker OFF
  MIDI.begin();
  MIDI.turnThruOff();
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("MIDI Rnd Seq.");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.print("Note Number:");
}

void loop() {
int RN;
RN=random(46,120);
  
  MIDI.sendNoteOn(RN, 127, 1);    // Send a Note (pitch 0, velo 127 on channel 1)
  M5.Lcd.setCursor(200, 130);
  M5.Lcd.print(RN);
  delay(100);                // Wait for a second
  MIDI.sendNoteOff(RN, 0, 1);
  M5.Lcd.setCursor(200, 130);
  M5.Lcd.printf("   ");

}
