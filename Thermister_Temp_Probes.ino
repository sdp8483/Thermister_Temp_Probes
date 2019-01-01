#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// which analog pin to connect
#define PROBE0 A0
#define PROBE1 A1
// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 16
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 20000

int p0 = 0;
int p0_last = 0;
unsigned long p0_count = 0;
int p1 = 0;
int p1_last = 0;
unsigned long p1_count = 0;

uint16_t samples[NUMSAMPLES];

void setup(void) {
  analogReference(EXTERNAL);

  lcd.begin(16, 2);
  lcd.clear();
}

void loop(void) {
  // Probe0
  p0 = readTempProbe(PROBE0);

  if (p0_last == p0) {
    p0_count++;
  } else {
    p0_count = 0;
    lcd.setCursor(5, 0);
    lcd.print("           ");
  }

  p0_last = p0;

  // Probe1
  p1 = readTempProbe(PROBE0);

  if (p1_last == p0) {
    p1_count++;
  } else {
    p1_count = 0;
    lcd.setCursor(5, 1);
    lcd.print("           ");
  }

  p1_last = p1;

  // display

  lcd.setCursor(0, 0);
  lcd.print(p0);
  lcd.print((char)223); // degree symbol
  lcd.print('C');
  lcd.print(' ');
  lcd.print(p0_count);

  lcd.setCursor(0, 1);
  lcd.print(p1);
  lcd.print((char)223); // degree symbol
  lcd.print('C');
  lcd.print(' ');
  lcd.print(p1_count);

  delay(1000);
}

int readTempProbe(int pin) {
  uint8_t i;
  float average;

  // take N samples in a row, with a slight delay
  for (i = 0; i < NUMSAMPLES; i++) {
    samples[i] = analogRead(pin);
    delay(10);
  }

  // average all the samples out
  average = 0;
  for (i = 0; i < NUMSAMPLES; i++) {
    average += samples[i];
  }
  average /= NUMSAMPLES;

  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  return steinhart;
}
