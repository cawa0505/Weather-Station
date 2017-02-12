//氣象站控制程式-原型v4.1
//加入二次時間判斷
//須搭載LM35DZ溫度感應器、DHT11溫濕度感應器、I2C 1602LCD顯示器、DS1302時間模組及MTARDSD記憶卡模組
#include <SPI.h>
#include <SD.h>
#include <DS1302.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <stdio.h>
//#include <Enerlib.h>

//SD卡pin腳
const int chipSelect = 4;

//氣象資訊pin腳
int sensor = 0;
int sw = 3;
bool chk = false;

//檔案檢查LEDpin腳
int fileOK = 10;
int fileNOT = 9;

//LCD資訊
LiquidCrystal_I2C lcd(0x27, 16, 2);

//**********時間模組所需設定****************************
const int kCePin   = 5;  //RST
const int kIoPin   = 6;  //DAT
const int kSclkPin = 7;  //CLK
DS1302 rtc(kCePin, kIoPin, kSclkPin);

//*************************溫濕度模組設定值**************
int DHpin = 8;
byte dat[5];

byte read_data()
{
  byte data;
  for (int i = 0; i < 8; i++)
  {
    if (digitalRead(DHpin) == LOW)
    {
      while (digitalRead(DHpin) == LOW);
      delayMicroseconds(30);
      if (digitalRead(DHpin) == HIGH) data |= (1 << (7 - i));
      while (digitalRead(DHpin) == HIGH);
    }
  }
  return data;
}

void start_test()
{
  digitalWrite(DHpin, LOW);
  delay(30);
  digitalWrite(DHpin, HIGH);
  delayMicroseconds(40);
  pinMode(DHpin, INPUT);
  while (digitalRead(DHpin) == HIGH);
  delayMicroseconds(80);
  if (digitalRead(DHpin) == LOW);
  delayMicroseconds(80);
  for (int i = 0; i < 4; i++) dat[i] = read_data();
  pinMode(DHpin, OUTPUT);
  digitalWrite(DHpin, HIGH);
}
//**************END**************

//**************時間模組所需副程式********
bool getTime() {//系統日期時間的函式
  Time t = rtc.time();
  if (t.min % 5 == 0 && t.sec == 0) return true;
  //if (t.sec == 0) return true;
  return false;
}

void printTime() {//顯示日期時間的函式
  Time t = rtc.time();
  char buf[50], buf2[10];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);
  snprintf(buf2, sizeof(buf2), ":%02d", t.sec);


  Serial.println(buf);
  lcd.print(buf);
  lcd. setCursor ( 0, 1) ;//LCD換行!!!!
  lcd.print(buf2);
}

String TakeTime() {//顯示日期時間的函式
  Time t = rtc.time();
  char buf[21];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d;%02d:%02d:%02d;",
           t.yr, t.mon, t.date, t.hr, t.min, t.sec);
  return buf;
}
//***************END*******************
//***************喚醒副程式************
/*void Sleepwake()
  {
  if(energy.WasSleeping())
  {
  }
  else
  {

  }
  }*/
void setup() {
  // 定義LCD
  lcd.begin();
  lcd.print("Hello!");

  //定義pin腳
  pinMode(sw, INPUT);
  pinMode(DHpin, OUTPUT);
  pinMode(fileOK, OUTPUT);
  pinMode(fileNOT, OUTPUT);

  //定義終端機連線
  Serial.begin(9600);
  while (!Serial) {
    ; // 等候
  }

  //************SD卡檢查*******************
  Serial.print("Initializing SD card...");

  // 檢查SD卡是否可被偵測
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}
//************END************************

/*rtc.writeProtect(false);// 是否防止寫入 (日期時間設定成功後即可改成true)
  rtc.halt(false);// 是否停止計時
  Time t(2016, 8, 18, 20, 35, 00, Time::kWednesday); //年 月 日 時 分 秒 星期幾 (日期時間設定成功後即可註解掉)
  rtc.time(t);//設定日期時間 (日期時間設定成功後即可註解掉)*/


//***********主迴圈**********************
void loop() {
  //宣告寫入檔案的字串，同時作為初始化動作
  //String dataString="";
  //digitalWrite(fileOK, LOW);
  bool timechk;  //二次時間檢查用
  timechk = getTime();
  int val, temp;
  lcd.clear();
  //Serial.print(dataString);
  if (digitalRead(sw) == HIGH && getTime() == true) //偵測開啟
  {
    //******取溫度**********************
    val = analogRead(sensor);
    temp = (125 * val) >> 8;
    //************取濕度***************
    start_test();

    lcd.print("Humidity= ");
    lcd.print(dat[0], DEC);
    lcd.print('%');

    Serial.print(dat[0], DEC);
    lcd. setCursor ( 0, 1) ;//LCD換行!!!!
    //*************END***********
    lcd.print("Temprature= ");
    lcd.print(temp);
    lcd.print("C");

    Serial.print(";");
    Serial.println(temp);
  }
  else//偵測關閉
  {
    lcd.clear();
    //energy.Standby(); //進入待機
  }

  //********************檔案處理************************
  //***********檔案輸出字串整理*************************
  String dataString = TakeTime();
  dataString += String(dat[0]);//把濕度放進輸出字串
  dataString += String(';');
  dataString += String(temp);//把溫度放進輸出字串
  /*開檔，同時只可開啟一個檔案
    開啟別的當案前此檔必須關閉*/

  //檔案開啟成功，寫入檔案
  if (digitalRead(sw) == HIGH)
    if  (getTime() == true || timechk == true) {
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      if (dataFile)
      {
        digitalWrite(fileNOT, LOW);
        digitalWrite(fileOK, HIGH);
        /*delay(1000);
          digitalWrite(fileOK, LOW);*/

        dataFile.println(dataString);
        delay(1000);
        dataFile.close();
        // print to the serial port too:
        Serial.println(dataString);
      }
      else if (!dataFile)
      {
        digitalWrite(fileOK, LOW);
        digitalWrite(fileNOT, HIGH);

        Serial.println("error opening datalog.txt");
        dataFile.close();
      }

    }

    else if (getTime() == false  || digitalRead(sw) == LOW)
    {
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      // lcd.clear();
      if (dataFile)
      {
        digitalWrite(fileNOT, LOW);//不可同時亮
        digitalWrite(fileOK, LOW);
        Serial.println("FILE TEST PASS");
      }
      else if (!dataFile)
      {
        digitalWrite(fileOK, LOW); //不可同時亮
        digitalWrite(fileNOT, HIGH);
        Serial.println("FILE TEST FAIL");
      }
      lcd.clear();
      printTime();
      delay(1000);
      dataFile.close();
    }
  //***********************END**************************
}
