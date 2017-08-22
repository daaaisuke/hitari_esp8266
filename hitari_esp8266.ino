#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "RTClib.h"
extern "C" {
#include "user_interface.h"
}


const int LEDPIN = 12;//LEDデジタルピン
const int SWPIN = 13;//SWITCHデジタルピン
const int NUMPIXELS = 4;//LED個数
const int BOOTTIME = 21;
const int SLEEPTIME = 2;
const int DATASIZE = 1683;



DateTime now;
RTC_DS1307 rtc;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_RGB + NEO_KHZ800);

uint ADC_Value = 0;
bool bLux = true;

void setup() {
  Serial.begin(115200);//シリアル通信スタート
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  randomSeed(analogRead(0));//いるのかどうか要検証
  ledSetup(255);
  rtcSetup();
  showDate();
  EEPROM.begin(DATASIZE);
  //  if (now.hour() >= SLEEPTIME/*2*/ && now.hour() < BOOTTIME/*21*/) { //スリープさせるかどうか判定
  ////    スリープ時間だったら
  //    setAlarm();
  //  }

}


void loop() {
  showDate();
  int bright;
  int Lux = system_adc_read();
  Serial.println(Lux);
  if (Lux > 600 && bLux) { //部屋が暗くなったら
    int strand = random(5);
    setFirstDelay(strand);//暗くなってから光り始めるまでの時間
    setFirstDelay(2);
    int int_rand = random(100);
    int sec_rand = random(20) * 1000;
    for (int i = 0; i < DATASIZE - (NUMPIXELS - 1) * 3; i += 3) {//サブのLED用のデータ以外ループ
      //      lightLED(int_rand, sec_rand, i);
      lightLED(35, 0, i);
    }
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    fallLight();

    bLux = false;
  } else if (Lux < 600 && !bLux) {
    bLux = true;
  }
  delay(100);//照度センサタイミング
  //  if (now.hour() == SLEEPTIME) {//スリープ時間になったら
  //    setAlarm();
  //  }
  pixels.setBrightness(0);
  for (int i = 0; i < NUMPIXELS; i++) {//全LEDを落とす
    pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
}


//========================================================
void ledSetup(int bright) {
  pinMode(SWPIN, OUTPUT);
  digitalWrite(SWPIN, HIGH);
  pixels.begin();
  pixels.setBrightness(bright);
  for (int i = 0; i < NUMPIXELS; i++) {//全LEDを落とす
    pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
}


void rtcSetup() {
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}





void setAlarm() {
  now = rtc.now();
  DateTime Cnow(now);//現在時刻を加工用にコピー
  if (Cnow.hour() == BOOTTIME) {
    Cnow = Cnow + TimeSpan(1, 0, 0, 0);
  } else {
    while (Cnow.hour() != BOOTTIME) {
      Cnow = Cnow + TimeSpan(0, 1, 0, 0);
    }
  }
  digitalWrite(SWPIN, LOW);
  DateTime Alarm(Cnow.year(), Cnow.month(), Cnow.day(), Cnow.hour(), 00, 00);
  TimeSpan diff = Alarm - now;
  if (diff.days() > 0 || diff.hours() > 0) {
    pinMode(12, INPUT_PULLUP);
    Serial.print(" - POWER_DOWN -");
    delay(1000);
    ESP.deepSleep(3600 * 1000 * 1000 , RF_DISABLED);
  } else {
    unsigned int diffTS = diff.totalseconds() * 1000 * 1000 / 24 - 1 * 1000 * 1000;//起きる時刻までの秒数を指定
    Serial.println(diffTS);
    pinMode(12, INPUT_PULLUP);
    Serial.print(" - LAST_POWER_DOWN -");
    delay(1000);
    ESP.deepSleep(diffTS , WAKE_RF_DEFAULT);
    delay(1000);
  }

}

void setFirstDelay(int r) {
  switch (r) {
    case 0:
      delay(8000);
      break;
    case 1:
      delay(9000);
      break;
    case 2:
      delay(10000);
      break;
    case 3:
      delay(11000);
      break;
    case 4:
      delay(12000);
      break;
  }
}

void Interval(int r1, int r2) {
  if (r1 < 15) {
    delay((30000 + r2) / (DATASIZE - (NUMPIXELS - 1) * 3) * 3);
  } else if (r1 >= 15 && r1 < 50) {
    delay((50000 + r2) / (DATASIZE - (NUMPIXELS - 1) * 3) * 3);
  } else if (r1 >= 50 && r1 < 70) {
    delay((70000 + r2) / (DATASIZE - (NUMPIXELS - 1) * 3) * 3);
  } else if (r1 >= 70 && r1 < 85) {
    delay((90000 + r2) / (DATASIZE - (NUMPIXELS - 1) * 3) * 3);
  } else if (r1 >= 85 && r1 < 95) {
    delay((110000 + r2) / (DATASIZE - (NUMPIXELS - 1) * 3) * 3);
  } else {
    delay((130000 + r2) / (DATASIZE - (NUMPIXELS - 1) * 3) * 3);
  }
}
void lightLED(int int_rand, int sec_rand, int i) {
  int bright =  (EEPROM.read(i) + EEPROM.read(i + 1) + EEPROM.read(i + 2) * 10) / 2 + 30;
  if (bright > 255) {
    bright = 255;
  }
  pixels.setBrightness(bright);
  pixels.setPixelColor(0, EEPROM.read(i), EEPROM.read(i + 1), EEPROM.read(i + 2));
  pixels.show();
  Interval(int_rand, sec_rand);
}

void fallLight() {
  for (int i = 1; i < NUMPIXELS; i++) {//1,2,3,4
    int x = (-NUMPIXELS + i) * 3 + 1 + DATASIZE; //-12,-9,-6,-3
    int bright = (EEPROM.read(x) + EEPROM.read(x + 1) + EEPROM.read(x + 2) * 10) / 2 + 30;
    if (bright > 255) {
      bright = 255;
    }
    pixels.setBrightness(bright);
    pixels.setPixelColor(i , EEPROM.read(x - 1), EEPROM.read(x), EEPROM.read(x + 1));
    pixels.show();
    delay(25);
    pixels.setPixelColor(i, 0, 0, 0);
    pixels.show();
  }
}


void showDate() {
  now = rtc.now();
  Serial.print(now.year());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.hour());
  Serial.print("/");
  Serial.print(now.minute());
  Serial.print("/");
  Serial.println(now.second());
}
