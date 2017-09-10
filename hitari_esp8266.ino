#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "RTClib.h"
extern "C" {
#include "user_interface.h"
}


const int LEDPIN = 12;//LEDデジタルピン
const int RELAY_PIN = 13;///リードリレー制御ピン

const int NUMPIXELS = 7;//LED個数
const int FALL_LIGHT_NUM = 3;
const int SPARK_LIGHT_NUM = 3;

const int BOOTTIME = 21;//起動時間
const int SLEEPTIME = 2;//スリープ時間

const int DATASIZE = 1683;//EEPROMデータ数



DateTime now;
RTC_DS1307 rtc;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_RGB + NEO_KHZ800);

//uint ADC_Value = 0;
bool bLux = true;//連続起動制御用bool

void setup() {
  Serial.begin(115200);//シリアル通信スタート
  wifi_stop();//wifi機能stop
  randomSeed(analogRead(5));
  ledSetup(0);
  rtcSetup();
  showDate();
  EEPROM.begin(DATASIZE);
//    if (now.hour() >= SLEEPTIME/*2*/ && now.hour() < BOOTTIME/*21*/) { //スリープさせるかどうか判定
  //    スリープ時間だったら
//      setAlarm();
//    }

}


void loop() {
  showDate();
  int bright;
  int Lux = system_adc_read();
  Serial.println(Lux);
  if (Lux > 600 && bLux) { //部屋が暗くなったら
    int strand = random(5);
    setFirstDelay(strand);//暗くなってから光り始めるまでの時間
//    setFirstDelay(2);
    int int_rand = random(100);//インターバル時間決定用乱数1
    int sec_rand = random(20) * 1000;//インターバル時間決定用乱数2
    for (int i = 0; i < DATASIZE - FALL_LIGHT_NUM * 3; i += 3) {//サブのLED用のデータ以外ループ
      //      lightLED(int_rand, sec_rand, i);
      lightLED(35, 0, i);
    }
    fallLight();

    bLux = false;
  } else if (Lux < 600 && !bLux) {
    bLux = true;
  }
  delay(100);//照度センサタイミング
//    if (now.hour() == SLEEPTIME) {//スリープ時間になったら
//      setAlarm();
//    }
  pixels.setBrightness(0);
  for (int i = 0; i < NUMPIXELS; i++) {//全LEDを落とす
    pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
}


//========================================================
void ledSetup(int bright) {//リードリレーON&&LED消灯
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);//リードリレーON
  pixels.begin();
  pixels.setBrightness(bright);
  for (int i = 0; i < NUMPIXELS; i++) {//全LEDを落とす
    pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
}


void rtcSetup() {
  rtc.begin();
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}





void setAlarm() {
  now = rtc.now();
  Serial.println(now.day());
  Serial.println(now.hour());
  Serial.println(now.minute());
  Serial.println(now.second());
  while (now.hour() != BOOTTIME) {//bootタイムまでの時間を計算
    now = now + TimeSpan(0, 1, 0, 0);
  }
  
  digitalWrite(RELAY_PIN, LOW);//リードリレーOFF
  DateTime Alarm(now.year(), now.month(), now.day(), now.hour(), 00, 00);
  TimeSpan diff = Alarm - rtc.now();
  if (diff.hours() == 0) {
    unsigned int diffTS = diff.totalseconds() * 1000 * 1000 / 24 - 1 * 1000 * 1000;//起きる時刻までの秒数を指定
    Serial.println(diffTS);
    pinMode(12, INPUT_PULLUP);
    Serial.print(" - LAST_POWER_DOWN -");
    delay(1000);
    ESP.deepSleep(diffTS , WAKE_RF_DEFAULT);
  } else {
    pinMode(12, INPUT_PULLUP);//使わないピンは省エネ化
    Serial.print(" - POWER_DOWN -");
    delay(1000);
    ESP.deepSleep(3600 * 1000 * 1000 , RF_DISABLED);
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
    delay((30000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3);
  } else if (r1 >= 15 && r1 < 50) {
    delay((50000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3);
  } else if (r1 >= 50 && r1 < 70) {
    delay((70000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3);
  } else if (r1 >= 70 && r1 < 85) {
    delay((90000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3);
  } else if (r1 >= 85 && r1 < 95) {
    delay((110000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3);
  } else {
    delay((130000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3);
  }
}

void HarfInterval(int r1, int r2) {
  if (r1 < 15) {
    delay((30000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3/2);
  } else if (r1 >= 15 && r1 < 50) {
    delay((50000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3/2);
  } else if (r1 >= 50 && r1 < 70) {
    delay((70000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3/2);
  } else if (r1 >= 70 && r1 < 85) {
    delay((90000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3/2);
  } else if (r1 >= 85 && r1 < 95) {
    delay((110000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3/2);
  } else {
    delay((130000 + r2) / (DATASIZE - (NUMPIXELS - 4) * 3) * 3/2);
  }
}
void lightLED(int int_rand, int sec_rand, int i) {
  int bright =  (EEPROM.read(i) + EEPROM.read(i + 1) + EEPROM.read(i + 2) * 10) / 2 + 30;
  if (bright > 255) {
    bright = 255;
  }
  pixels.setBrightness(bright);
  pixels.setPixelColor(0, EEPROM.read(i), EEPROM.read(i + 1), EEPROM.read(i + 2));
  if(bright > 130){//spark処理
    int r = random(NUMPIXELS - SPARK_LIGHT_NUM, NUMPIXELS);
    switch(r){
      case 4:
        pixels.setPixelColor(r, EEPROM.read(i), EEPROM.read(i + 1), EEPROM.read(i + 2));
      case 5:
        pixels.setPixelColor(r, EEPROM.read(i), EEPROM.read(i + 1), EEPROM.read(i + 2));
      case 6:
        pixels.setPixelColor(r, EEPROM.read(i), EEPROM.read(i + 1), EEPROM.read(i + 2));
    }
  }
  pixels.show();
  HarfInterval(int_rand, sec_rand);
  for(int i = 4; i < 7; i ++){
    pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
  HarfInterval(int_rand, sec_rand);
}

void fallLight() {
  pixels.setPixelColor(0, 0, 0, 0);//mainLEDを消灯
  pixels.show();
  for (int i = 1; i < NUMPIXELS - 3; i++) {//1,2,3,4
    int x = (-(NUMPIXELS - 3) + i) * 3 + 1 + DATASIZE; //-12,-9,-6,-3
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

void wifi_stop(){
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();  
}
