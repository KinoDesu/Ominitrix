#include "Omnitrix_Configs.h"

void setup()
{
  Serial.begin(115200);

  pinMode(potentiometer, INPUT);
  pinMode(btnAlienChooser, INPUT_PULLUP);
  pinMode(btnActivate, INPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(ROTARY_PINCLK, INPUT);
  pinMode(ROTARY_PINDT, INPUT);

  attachInterrupt(digitalPinToInterrupt(ROTARY_PINCLK), rotary, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PINDT), rotary, CHANGE);

  tft.begin();

  tone(buzzer, 800);
  delay(100);
  noTone(buzzer);
  delay(10);
  tone(buzzer, 1600);
  delay(100);
  noTone(buzzer);

  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);
  if (rc == PNG_SUCCESS)
  {
    tft.fillScreen(OMNITRIX_GREEN);
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void loop()
{
  loopStart = millis();

  if (digitalRead(btnActivate) == HIGH)
  {
    // travado enquanto o botão estiver segurado
    while (digitalRead(btnActivate) == HIGH)
    {
      delay(100);
      holdCount += millis() - loopStart;

      if (holdCount >= 5000)
      {
        holdCount = 0;
        isClockMode = true;
        clockMode();
      }
    }
    omnitrixModeSet();
  }

  omnitrixRecharge();
}

// modo omnitrix
#pragma region Omnitrix

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

  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);

  if (rc == PNG_SUCCESS)
  {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();
  }
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

void getAlienNo()
{
  if (rotaryEncoder)
  {
    // Get the movement (if valid)
    int8_t rotationValue = checkRotaryEncoder();

    // If valid movement, do something
    if (rotationValue != 0)
    {
      alienNo += rotationValue;
      if (alienNo < 0)
      {
        alienNo = alienCount - 1;
      }
      if (alienNo >= alienCount)
      {
        alienNo = 0;
      }
    }
  }
}

void alienSelect()
{

  tft.fillScreen(OMNITRIX_GREEN);

  int frame = 0;
  int fps = 10;

  for (; frame <= omnitrix_anim_N; frame++)
  {

    int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[frame], sizeof(omnitrix_anim[frame]), pngDraw);

    if (rc == PNG_SUCCESS)
    {
      tft.startWrite();
      rc = png.decode(NULL, 0);
      tft.endWrite();
      tft.endWrite();
    }
    delay(fps);
  }
  delay(100);

  while (isActivate == true)
  {

    if (digitalRead(btnActivate) == HIGH)
    {
      while (digitalRead(btnActivate) == HIGH)
        ;
      isActivate = false;
      hasLastAlien = false;
      omnitrixShutdown();
      return;
    }

    if (hasLastAlien == false)
    {
      changeAlien(alienNo);
      hasLastAlien = true;
      lastAlienValue = alienNo;
    }

    getAlienNo();

    if (alienNo != lastAlienValue)
    {
      lastAlienValue = alienNo;
      tone(buzzer, 125);
      delay(100);
      noTone(buzzer);
      changeAlien(alienNo);
    }

    if (digitalRead(btnAlienChooser) == LOW)
    {
      while (digitalRead(btnAlienChooser) == LOW)
        ;

      hasLastAlien = false;
      isActivate = false;
      isAlienForm = true;
      isSameRecharge = false;
      tone(buzzer, 367);
      delay(100);
      noTone(buzzer);
      changeScreen(OMNITRIX_WHITE);

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
          hasLastAlien = false;

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

void changeAlien(int alienNo)
{

  int16_t rc = png.openFLASH((uint8_t *)omnitrix_alien_backround, sizeof(omnitrix_alien_backround), pngDraw);

  if (rc == PNG_SUCCESS)
  {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();
  }

  delay(100);

  rc = png.openFLASH((uint8_t *)omnitrix_aliens[alienNo], sizeof(omnitrix_aliens[alienNo]), pngDraw);

  if (rc == PNG_SUCCESS)
  {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }
}

void omnitrixShutdown()
{

  // toca o som
  tone(buzzer, 250);
  delay(100);
  noTone(buzzer);
  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);

  if (rc == PNG_SUCCESS)
  {
    tft.fillScreen(OMNITRIX_GREEN);
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();
  }
}

void omnitrixDescharge()
{
  isAlienForm = false;
  hasLastAlien = false;

  // vermelho
  changeScreen(OMNITRIX_RED);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  changeScreen(OMNITRIX_WHITE);

  delay(500);

  // vermelho
  changeScreen(OMNITRIX_RED);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  changeScreen(OMNITRIX_WHITE);

  delay(500);

  // vermelho
  changeScreen(OMNITRIX_RED);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  changeScreen(OMNITRIX_WHITE);

  delay(500);

  // vermelho
  changeScreen(OMNITRIX_RED);
  // toca
  tone(buzzer, 125);
  delay(250);
  noTone(buzzer);
  // branco
  changeScreen(OMNITRIX_WHITE);

  delay(500);

  // vermelho
  changeScreen(OMNITRIX_RED);
  tone(buzzer, 125);
  delay(500);
  noTone(buzzer);
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
      changeScreen(OMNITRIX_GREEN);
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

void changeScreen(uint16_t color)
{
  int16_t rc = png.openFLASH((uint8_t *)omnitrix_anim[0], sizeof(omnitrix_anim[0]), pngDraw);

  if (rc == PNG_SUCCESS)
  {
    tft.fillScreen(color);
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    tft.endWrite();

    // png.close(); // Required for files, not needed for FLASH arrays
  }
}

#pragma endregion

void clockMode()
{
  long time = 0;

  tft.fillScreen(OMNITRIX_WHITE);
  tft.setCursor(100, 100, 2);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(5);

  while (isClockMode)
  {
    tft.fillScreen(OMNITRIX_WHITE);
    loopStart = millis();
    tft.println(time);
    time++;
    delay(1000);

    while (digitalRead(btnActivate) == HIGH)
    {
      delay(100);
      holdCount += millis() - loopStart;

      if (holdCount >= 5000)
      {
        holdCount = 0;
        isClockMode = false;
      }
    }
  }
}