//#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include "RTClib.h"
extern "C" {
#include "user_interface.h"
}

const int RTCPIN = 0;
const int LEDPIN = 12;
const int SPARKPIN = 5;
const int RELAY_PIN = 13;

const int NUMPIXELS = 5;
const int SPARK_LIGHT_NUM = 7;

const int BOOTTIME = 22;//起動時間
const int SLEEPTIME = 2;//スリープ時間

const int DATASIZE = 1683;//EEPROMデータ数


DateTime rnow;
DateTime anow;
RTC_DS1307 rtc;

bool bLux = true;//連続起動制御用bool



void setup() {
  Serial.begin(115200);//===削除===
  wifi_stop();
  EEPROM.begin(DATASIZE + 7);
  rtcSetup();
  if (hour() >= SLEEPTIME/*2*/ && hour() < BOOTTIME/*22*/) { //スリープさせるかどうか判定
    Serial.println("setupでスリープ");
    //    setAlarm();
  }

  randomSeed(analogRead(5));
  ledSetup();
}

void loop() {
<<<<<<< HEAD
  //====================================================================================

  if (hour() == SLEEPTIME) {
    Serial.println("loopでスリープ");
    //    setAlarm();
  }

  //====================================================================================

  int Lux = system_adc_read();
  Serial.println(Lux);

  if (Lux > 600 && bLux) {//暗くなったら
    digitalWrite(RELAY_PIN, HIGH);//リードリレーON
    delay(5);
    for (int i = 0; i < NUMPIXELS; i++) {//全LEDを落とす
      pixels.setPixelColor(i, 0, 0, 0);
    }
    for (int i = 0; i < SPARK_LIGHT_NUM; i++) {//全LEDを落とす
      sparks.setPixelColor(i, 0, 0, 0);
    }
    pixels.show();
    sparks.show();

    //---------------------
    int first_wait_rand = random(4, 9);
    delay(first_wait_rand * 1000);//暗くなってから光り始めるまでの時間
    int bright_time_rand1 = random(100);//インターバル時間決定用乱数1
    int bright_time_rand2 = random(20) * 1000;//インターバル時間決定用乱数2
    //---------------------


    for (int i = 0; i < DATASIZE - (NUMPIXELS - 1) * 3; i += 3) { //サブのLED用のデータ以外ループ
      lightLED(bright_time_rand1, bright_time_rand2, i);
    }
    fallLight();

    bLux = false;
    digitalWrite(RELAY_PIN, LOW);//リードリレーON
  } else if (Lux < 600 && !bLux) {
    bLux = true;
  }
  delay(100);//照度センサタイミング
  showDate();

}


//========================================================
void ledSetup() {//リードリレーON&&LED消灯
  pinMode(RELAY_PIN, OUTPUT);
  
}


void rtcSetup() {
  pinMode(RTCPIN, OUTPUT);
  rtc.begin();
  //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//初回のみ
  //  EEPROM.write(1683, 1);
  //  EEPROM.commit();
  if (EEPROM.read(1683) == 1) {
    digitalWrite(RTCPIN, HIGH);//RTC電源ON
    delay(100);
    rnow = rtc.now();
  } else {
    rnow = DateTime(EEPROM.read(1684), EEPROM.read(1685), EEPROM.read(1686), EEPROM.read(1687), EEPROM.read(1688), EEPROM.read(1689)) + TimeSpan(0, 1, 0, 0);
  }
  setTime(rnow.hour(), rnow.minute(), rnow.second(), rnow.day(), rnow.month(), rnow.year());
  showDate();
  rtc.writeSqwPinMode(OFF);
  digitalWrite(RTCPIN, LOW);//RTC電源OFF

}


void setAlarm() {//setup→時間未定,loop→SLEEPTIME(2時)
  rnow = DateTime(year(), month(), day(), hour(), minute(), second());//スリープ時の時刻を記録しておく
  anow = rnow;
  //スリープまでの時間を探る
  while (anow.hour() != BOOTTIME) {//22時
    anow = anow + TimeSpan(0, 1, 0, 0);
  }

  DateTime Alarm(anow.year(), anow.month(), anow.day(), anow.hour(), 00, 00);
  Serial.print(Alarm.year());
  Serial.print(Alarm.month());
  Serial.print(Alarm.day());
  Serial.print(Alarm.hour());
  Serial.print(Alarm.minute());
  Serial.println(Alarm.second());
  TimeSpan diff = Alarm - rnow;
  Serial.print(diff.hours());
  Serial.print("/");
  Serial.print(diff.minutes());
  Serial.print("/");
  Serial.println(diff.seconds());

  if (diff.hours() == 0) {
    //            unsigned int diffTS = diff.totalseconds() * 1000 * 1000 / 24 - 1 * 1000 * 1000;//待ちの一秒を引く//動かない======================
    pinMode(0, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(12, INPUT_PULLUP);
    pinMode(13, INPUT_PULLUP);
    Serial.print(" - LAST_POWER_DOWN -");
    delay(1000);
    ESP.deepSleep(3600 * 1000 * 1000 , WAKE_RF_DISABLED);//===調査===
  } else {
    pinMode(12, INPUT_PULLUP);//使わないピンは省エネ化
    //        if (EEPROM.read(1683) != 0) {
    //          EEPROM.write(1683, 0 );
    //        }

    EEPROM.write(1684, year() % 100 );
    EEPROM.write(1685, month() );
    EEPROM.write(1686, day() );
    EEPROM.write(1687, hour() );
    EEPROM.write(1688, minute() );
    EEPROM.write(1689, second() );
    EEPROM.commit();
    Serial.print(" - POWER_DOWN -");
    delay(1000);
    ESP.deepSleep(3600 * 1000 * 1000 , WAKE_RF_DISABLED);//====調査===
  }

}

void Interval(int r1, int r2) {
  if (r1 < 15) {
    delay((50000 + r2) / (DATASIZE / 3 - (NUMPIXELS - 1)) / 2);
  } else if (r1 >= 15 && r1 < 50) {
    delay((60000 + r2) / (DATASIZE / 3 - (NUMPIXELS - 1)) / 2);
  } else if (r1 >= 50 && r1 < 70) {
    delay((70000 + r2) / (DATASIZE / 3 - (NUMPIXELS - 1)) / 2);
  } else if (r1 >= 70 && r1 < 85) {
    delay((80000 + r2) / (DATASIZE / 3 - (NUMPIXELS - 1)) / 2);
  } else {
    delay((90000 + r2) / (DATASIZE / 3 - (NUMPIXELS - 1)) / 2);
  }
}

void lightLED(int int_rand, int sec_rand, int i) {
  int bright =  (EEPROM.read(i) + EEPROM.read(i + 1) + EEPROM.read(i + 2) * 10) / 2 - 10;
  if (bright > 255) {
    bright = 255;
  }
  if (bright < 10) {
    bright += 5;
  }
  pixels.setBrightness(bright);
  pixels.setPixelColor(0, EEPROM.read(i), EEPROM.read(i + 1), EEPROM.read(i + 2));

  if (bright > 90) { //spark処理
    sparks.setBrightness(60);
    int r[SPARK_LIGHT_NUM];
    for (int j = 0; j < sizeof(r); j++) {
      r[j] = random(0, 5);
      if (r[j] == 3) {
        sparks.setPixelColor(j, EEPROM.read(i), EEPROM.read(i + 1), EEPROM.read(i + 2));
      }
    }
  }
  pixels.show();
  sparks.show();
  Interval(int_rand, sec_rand);
  for (int j = 0; j < SPARK_LIGHT_NUM; j ++) {
    sparks.setPixelColor(j, 0, 0, 0);
  }
  sparks.show();
  Interval(int_rand, sec_rand);
}

void fallLight() {
  pixels.setPixelColor(0, 0, 0, 0);//mainLEDを消灯
  pixels.show();
  for (int i = 1; i < NUMPIXELS; i++) {
    int x = DATASIZE - (NUMPIXELS - i) * 3;
    int bright = (EEPROM.read(x) + EEPROM.read(x + 1) + EEPROM.read(x + 2)) / 3;
    if (bright > 255) {
      bright = 255;
    }
    if (bright < 10) {
      bright += 10;
    }
    pixels.setBrightness(bright);
    pixels.setPixelColor(i , EEPROM.read(x), EEPROM.read(x + 1), EEPROM.read(x + 2));
    pixels.show();
    delay(25);
    pixels.setPixelColor(i, 0, 0, 0);
    pixels.show();
  }
}


void showDate() {
  Serial.print(year());
  Serial.print("/");
  Serial.print(month());
  Serial.print("/");
  Serial.print(day());
  Serial.print("/");
  Serial.print(hour());
  Serial.print("/");
  Serial.print(minute());
  Serial.print("/");
  Serial.println(second());
}

void wifi_stop() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
}
