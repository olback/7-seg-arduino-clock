// Original source: http://moosteria.blogspot.com/2015/01/seven-segment-displays-on-arduino.html

// Digit selectors on 2 - 5 inclusive.
// Segment selectors on 6 - 12 inclusive.
// This is for a common cathode display, so we make a digit selector low
// and a segment selector high to display things.

#include <RtcDS3231.h>
#include <Wire.h>

const int digit_base = 2;
const int segment_base = 6;
const int PIN_COLON = A3;
const bool setTimeOnBoot = false;

RtcDS3231<TwoWire> rtcModule(Wire);

// Segments patterns for the digits.
// Bit 0 - segment A, etc.
// Bit patterns are segments .GFEDBCA
const byte segments[] = {
  B00111111, // 0: ABCDEF
  B00000110, // 1: BC
  B01011011, // 2: ABDEG
  B01001111, // 3: ABCDG
  B01100110, // 4: BCFG
  B01101101, // 5: ACDFG
  B01111101, // 6: ACDEFG
  B00000111, // 7: ABC
  B01111111, // 8: ABCDEFG
  B01101111,  // 9: ABCDFG
  // Error text
  B01111001, // E: ABCDG
  B01010000 // r: ABCDG
};


// Select digit, 0 = leftmost.
void selectDigit(int position) {
  for (int i = 0; i < 4; ++i) {
    digitalWrite(i + digit_base, (i == position) ? LOW : HIGH);
  }
}

void displayDigit(int value) {
  int pattern = segments[value];
  for (int i = 0; i < 8; ++i) {
    boolean on = pattern & 1;
    pattern = pattern >> 1;
    digitalWrite(i + segment_base, on ? HIGH : LOW);
  }
}

void displayNumber(int value) {
  for (int i = 3; i >= 0; --i) {
    selectDigit(i);
    displayDigit(value % 10);
    delay(1);
    value = value / 10;
  }
}

void setup() {

  Wire.begin(10000);
  Serial.begin(115200);

  // Don't reset the time on every boot
  if (setTimeOnBoot) {
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    rtcModule.SetDateTime(compiled);
  }

  for (int i = 2; i <= 13; ++i) {
    pinMode(i, OUTPUT);
  }

  for (int i = 2; i <= 5; ++i) {
    digitalWrite(i, HIGH);
  }

  for (int j = 6; j <= 12; ++j) {
    digitalWrite(j, LOW);
  }

  pinMode(PIN_COLON, OUTPUT);

}

void showError() {

    digitalWrite(PIN_COLON, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    selectDigit(0);
    displayDigit(10);
    delay(1);
    selectDigit(1);
    displayDigit(11);
    delay(1);
    selectDigit(2);
    displayDigit(11);
    delay(1);

}

void loop() {

  RtcDateTime now = rtcModule.GetDateTime();
  byte hours = now.Hour();
  byte minutes = now.Minute();
  byte seconds = now.Second();

  char ts[10] = "\0";
  sprintf(ts, "%02d:%02d:%02d\n", hours, minutes, seconds);
  Serial.write(ts, strlen(ts));

  if (setTimeOnBoot) {
    delay(1000);
    return;
  }

  if (hours < 24) {

    if (seconds % 2 == 0) {
      digitalWrite(PIN_COLON, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(PIN_COLON, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }

    char t[5];
    sprintf(t, "%02d%02d", hours, minutes);
    displayNumber(atoi(t));

  } else { // Rtc error

    showError();

  }

  // delay(0);
}

