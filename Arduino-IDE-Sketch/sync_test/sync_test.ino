/*
    Name:       button.ino
    Created:    2018/9/21 14:06:15
    Author:     sakabin
*/

#include <M5Stack.h>
// The setup() function runs once each time the micro-controller starts
void setup() {
  // init lcd, serial, but don't init sd card
  M5.begin(true, false, true);
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(65, 10);
  M5.Lcd.println("MIDI Module2");
  M5.Lcd.setCursor(3, 35);
  M5.Lcd.println("Sync Test");
  M5.Lcd.println("A:Slow B:Fast");
  M5.Lcd.setTextColor(RED);
  pinMode(5, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

int count;
int tempo = 80000;
int tempoo = tempo;

void loop() {
  M5.update();
  if (count > tempo) {
    digitalWrite(5, HIGH);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setCursor(150, 120);
    M5.Lcd.println("O");
    M5.update();

    delay(50);
    digitalWrite(5, LOW);
    M5.Lcd.setTextColor(BLACK);

    M5.Lcd.setCursor(150, 120);
    M5.Lcd.println("O");
    M5.update();

    count = 0;
  }
  else count ++;

  if (M5.BtnA.wasReleased()) {
    tempo = tempo + 10000;
  }
  else if (M5.BtnB.wasReleased()) {
    tempo = tempo - 10000;
  }
}
