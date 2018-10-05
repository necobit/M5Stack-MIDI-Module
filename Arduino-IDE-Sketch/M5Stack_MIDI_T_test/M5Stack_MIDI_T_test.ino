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
  M5.Lcd.print("MIDI Tx TEST");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.print("Note Number:");
}

void loop() {
  MIDI.sendNoteOn(0, 127, 1);    // Send a Note (pitch 0, velo 127 on channel 1)
  M5.Lcd.setCursor(200, 130);
  M5.Lcd.printf("0");
  delay(1000);                // Wait for a second
  MIDI.sendNoteOff(0, 0, 1);
  M5.Lcd.setCursor(200, 130);
  M5.Lcd.printf(" ");
  delay(1000);
  MIDI.sendNoteOn(1, 127, 1);    // Send a Note (pitch 1, velo 127 on channel 1)
  M5.Lcd.setCursor(200, 130);
  M5.Lcd.printf("1");
  delay(1000);                // Wait for a second
  MIDI.sendNoteOff(1, 0, 1);
  M5.Lcd.setCursor(200, 130);
  M5.Lcd.printf(" ");
  delay(1000);
}
