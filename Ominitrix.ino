#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "images.h"

#define potentiometer A1
#define btnAlienChooser 5
#define buzzer 4
#define btnActivate 6

#define TFT_CS 10
#define TFT_DC 9
Adafruit_GC9A01A tft = Adafruit_GC9A01A(TFT_CS, TFT_DC);

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define IMAGE_WIDTH 100
#define IMAGE_HEIGHT 120

byte isAlienForm = HIGH;
byte isActivate = LOW;
byte lastPotentiometerValue = 0;
boolean hasLastPotentiometer = false;
boolean hasBattery = true;
long elapsedTime = 0L;
long deschargeTime = 5000;
long rechargeTime = 5000;
volatile long loopStart;
byte result = 0;

void setup()
{
  Serial.begin(9600);

  tft.begin();
  tone(buzzer, 800);
  delay(100);
  noTone(buzzer);
  delay(10);
  tone(buzzer, 1600);
  delay(100);
  noTone(buzzer);
  tft.fillScreen(0x0F00);

  pinMode(potentiometer, INPUT);
  pinMode(btnAlienChooser, INPUT);
  pinMode(btnActivate, INPUT);
  pinMode(buzzer, OUTPUT);
}

void loop()
{
  loopStart = millis();

  if (elapsedTime >= rechargeTime && hasBattery == false)
  {
    hasBattery = true;

    tone(buzzer, 800);
    delay(100);
    noTone(buzzer);
    delay(10);
    tone(buzzer, 1600);
    delay(100);
    noTone(buzzer);

    tft.fillScreen(0x0F00);

    elapsedTime = 0L;
    delay(100);
  }

  if (digitalRead(btnActivate) == HIGH)
  {
    // travado enquanto o botão estiver segurado
    while (digitalRead(btnActivate) == HIGH)
      ;

    // se desligado e com bateria
    if (isActivate == LOW && hasBattery == true)
    {

      // liga o omnitrix
      isActivate = HIGH;

      omnitrixStartup();
      alienSelect();
    }
    // se desligado e sem bateria
    else if (isActivate == LOW && hasBattery == false)
    {

      // toca o som
      tone(buzzer, 125);
      delay(250);
      noTone(buzzer);
      delay(100);

      // se ligado
    }
    else if (isActivate == HIGH)
    {
      // desliga o omnitrix
      isActivate = LOW;
      omnitrixShutdown();
      delay(100);
    }
  }

  if (hasBattery == false)
  {
    delay(100);

    elapsedTime += millis() - loopStart;
    Serial.println(elapsedTime);
  }
}

void omnitrixStartup()
{
  // toca o som
  tone(buzzer, 1000);
  delay(100);
  noTone(buzzer);
  delay(10);
  tone(buzzer, 2000);
  delay(100);
  noTone(buzzer);
  delay(100);
}

void alienSelect()
{
  while (isActivate == HIGH)
  {

    if (digitalRead(btnActivate) == HIGH)
    {
      while (digitalRead(btnActivate) == HIGH)
        ;
      isActivate = false;
      omnitrixShutdown();
      return;
    }

    int potVal = analogRead(potentiometer);
    result = map(potVal, 0, 650, 0, 9);

    if (hasLastPotentiometer == false)
    {
      showScreen(result);
      hasLastPotentiometer = true;
      lastPotentiometerValue = result;
    }

    if (result != lastPotentiometerValue)
    {
      lastPotentiometerValue = result;
      tone(buzzer, 125);
      delay(100);
      noTone(buzzer);
      cleanScreen();
      showScreen(result);
    }

    if (digitalRead(btnAlienChooser) == HIGH)
    {
      hasLastPotentiometer = false;
      isActivate = LOW;
      isAlienForm = LOW;
      tone(buzzer, 367);
      delay(100);
      noTone(buzzer);
      tft.fillScreen(0xFFFF);
    }

    if (isAlienForm == LOW)
    {
      delay(deschargeTime);

      omniDescharge();
      hasBattery = false;
      loopStart = millis();
    }
  }
}

void omnitrixShutdown()
{

  // toca o som
  tone(buzzer, 250);
  delay(100);
  noTone(buzzer);
  tft.fillScreen(0x0F00);
  hasLastPotentiometer = false;
}

void omniDescharge()
{
  isAlienForm = HIGH;
  hasLastPotentiometer = false;

  tft.fillScreen(0xF000);
  delay(100);
  tone(buzzer, 125);
  delay(500);
  noTone(buzzer);
  delay(500);

  tft.fillScreen(0xFFFF);
  delay(100);
  tft.fillScreen(0xF000);
  tone(buzzer, 125);
  delay(500);
  noTone(buzzer);
  delay(500);
  tft.fillScreen(0xFFFF);
  delay(100);

  tft.fillScreen(0xFFFF);
  delay(100);
  tft.fillScreen(0xF000);
  tone(buzzer, 125);
  delay(1000);
  noTone(buzzer);
  delay(100);
  tft.fillScreen(0xF000);
}

void cleanScreen()
{
  tft.fillScreen(0x0000);
}

void showScreen(byte result)
{
  cleanScreen();
  drawBitmapMe((DISPLAY_WIDTH - IMAGE_WIDTH) / 2, (DISPLAY_HEIGHT - IMAGE_HEIGHT) / 2, alienList[result], IMAGE_WIDTH, IMAGE_HEIGHT, 0xFFFF);
}

void drawBitmapMe(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{

  int16_t i, j, byteWidth = (w + 7) / 8;
  uint8_t byte;

  for (j = 0; j < h; j++)
  {
    for (i = 0; i < w; i++)
    {
      if (i & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(bitmap + j * byteWidth + i / 8);
      if (byte & 0x80)
        tft.drawPixel(x + i, y + j, color);
    }
  }
}