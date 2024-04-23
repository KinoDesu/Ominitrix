#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "aliens.h"
#include "omnitrixAnimation.h"

/*
    Arduino -> UNO
    display -> GC9A01 VER1.0
    _______________________
    | **DISPLAY** | **PIN** |
    |:-----------:|:-------:|
    |     VCC     |  3.3V   |
    |     GND     |  GROUND |
    |     SCL     |  13     |
    |     DC      |  11     |
    |     CS      |  9      |
    |     RST     |  10     |
    |_____________|_________|
*/

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

boolean isAlienForm = false;
boolean isActivate = false;
boolean isSameRecharge = false;
boolean hasLastPotentiometer = false;
byte lastPotentiometerValue = 0;
long elapsedTime = 0L;
long morphedTime = 0L;
long minMorphTime = 5000L;
long deschargeTime = 15000L;
long batteryValue = deschargeTime;
long loopStart;

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

  tft.fillScreen(0x0000);
  fadeInScreen(0x0F00);

  // for (byte i = 0; i < omnitrixAnimationLength; i++) {
  //   tft.drawBitmap((DISPLAY_WIDTH - 240) / 2, (DISPLAY_HEIGHT - 240) / 2, omnitrixAnimation[i], DISPLAY_WIDTH, DISPLAY_HEIGHT, 0xFFFF);
  //   tft.fillScreen(0x0000);
  // }
  pinMode(potentiometer, INPUT);
  pinMode(btnAlienChooser, INPUT);
  pinMode(btnActivate, INPUT);
  pinMode(buzzer, OUTPUT);
}

void loop()
{
  loopStart = millis();

  if (digitalRead(btnActivate) == HIGH)
  {
    // travado enquanto o botão estiver segurado
    while (digitalRead(btnActivate) == HIGH)
      ;
    omnitrixModeSet();
  }

  omnitrixRecharge();
}

void omnitrixModeSet()
{
  // se desligado e com bateria maior q 0
  if (isActivate == false && batteryValue > minMorphTime)
  {
    // liga o omnitrix
    isActivate = true;

    omnitrixStartup();
    alienSelect();
  }
  // se desligado e sem bateria
  else if (isActivate == false && batteryValue < minMorphTime)
  {
    // toca o som
    tone(buzzer, 125);
    delay(250);
    noTone(buzzer);
    delay(100);

    // se ligado
  }
  else if (isActivate == true)
  {
    // desliga o omnitrix
    isActivate = false;
    omnitrixShutdown();
    delay(100);
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
  fadeInScreen(0x0000);
  while (isActivate == true)
  {

    if (digitalRead(btnActivate) == HIGH)
    {
      while (digitalRead(btnActivate) == HIGH)
        ;
      isActivate = false;
      omnitrixShutdown();
      return;
    }

    byte alienNo = map(analogRead(potentiometer), 0, 650, 0, 9);

    if (hasLastPotentiometer == false)
    {
      changeAlien(lastPotentiometerValue, alienNo);
      hasLastPotentiometer = true;
      lastPotentiometerValue = alienNo;
    }

    if (alienNo != lastPotentiometerValue)
    {
      tone(buzzer, 125);
      delay(100);
      noTone(buzzer);
      changeAlien(lastPotentiometerValue, alienNo);
      lastPotentiometerValue = alienNo;
    }

    if (digitalRead(btnAlienChooser) == HIGH)
    {

      hasLastPotentiometer = false;
      isActivate = false;
      isAlienForm = true;
      isSameRecharge = false;
      tone(buzzer, 367);
      delay(100);
      noTone(buzzer);
      fadeInScreen(0xFFFF);

      long morphStart = millis();
      while (isAlienForm)
      {
        morphedTime = millis() - morphStart;
        if (morphedTime >= batteryValue)
        {
          batteryValue -= morphedTime;

          if (batteryValue < 0)
          {
            batteryValue = 0;
          }

          omnitrixDescharge();
          isAlienForm = false;
          loopStart = millis();
        }

        if (digitalRead(btnActivate) == HIGH && morphedTime >= minMorphTime)
        {
          while (digitalRead(btnActivate) == HIGH)
            ;
          batteryValue -= morphedTime;

          if (batteryValue < 0)
          {
            batteryValue = 0;
          }

          omnitrixShutdown();
          isAlienForm = false;
          loopStart = millis();
        }
      }
    }
  }
}

void changeAlien(byte lastAlienNo, byte alienNo)
{
  if (lastAlienNo > alienNo)
  {
    for (int x = (DISPLAY_WIDTH - IMAGE_WIDTH) / 2; x <= (DISPLAY_WIDTH - IMAGE_WIDTH) / 2 + IMAGE_WIDTH; x++)
    {
      tft.drawFastVLine(x, (DISPLAY_HEIGHT - IMAGE_HEIGHT) / 2, IMAGE_HEIGHT, 0x0000);
    }
  }
  else
  {
    for (int x = (DISPLAY_WIDTH - IMAGE_WIDTH) / 2 + IMAGE_WIDTH; x >= (DISPLAY_WIDTH - IMAGE_WIDTH) / 2; x--)
    {
      tft.drawFastVLine(x, (DISPLAY_HEIGHT - IMAGE_HEIGHT) / 2, IMAGE_HEIGHT, 0x0000);
    }
  }

  tft.drawBitmap((DISPLAY_WIDTH - IMAGE_WIDTH) / 2, (DISPLAY_HEIGHT - IMAGE_HEIGHT) / 2, alienList[alienNo], IMAGE_WIDTH, IMAGE_HEIGHT, 0xFFFF);
}

void omnitrixShutdown()
{

  // toca o som
  tone(buzzer, 250);
  delay(100);
  noTone(buzzer);
  fadeInScreen(0x0F00);
  hasLastPotentiometer = false;
}

void omnitrixDescharge()
{
  isAlienForm = false;
  hasLastPotentiometer = false;

  // vermelho
  fadeInScreen(0xF000);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  fadeInScreen(0xFFFF);

  // vermelho
  fadeInScreen(0xF000);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  fadeInScreen(0xFFFF);

  // vermelho
  fadeInScreen(0xF000);
  tone(buzzer, 125);
  delay(500);
  noTone(buzzer);
  fadeInScreen(0xF000);
}

void omnitrixRecharge()
{
  if (batteryValue < deschargeTime)
  {
    delay(100);
    elapsedTime += millis() - loopStart;
    batteryValue += elapsedTime;

    if (batteryValue >= minMorphTime && isSameRecharge == false)
    {
      morphedTime = 0L;
      tone(buzzer, 800);
      delay(100);
      noTone(buzzer);
      delay(10);
      tone(buzzer, 1600);
      delay(100);
      noTone(buzzer);
      isSameRecharge = true;
      fadeInScreen(0x0F00);
    }

    if (batteryValue >= deschargeTime)
    {
      batteryValue = deschargeTime;
      tone(buzzer, 800);
      delay(100);
      noTone(buzzer);
      delay(10);
      tone(buzzer, 1600);
      delay(100);
      noTone(buzzer);
      tone(buzzer, 1600);
      delay(500);
      noTone(buzzer);
      isSameRecharge = false;
    }

    elapsedTime = 0L;
  }
}

void fadeInScreen(uint16_t color)
{
  int i = 1;
  while (i <= DISPLAY_WIDTH / 2 + 1)
  {
    tft.fillCircle(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, i += 10, color);
  }
}