#include <Bridge.h>
#include <HttpClient.h>
#include <FileIO.h>
#include<Process.h>
#include<DHT.h>
#include"Prototype.h"

//**********DHT22 Parameters**********
#define dhtpin 8
#define dhttype DHT22
DHT dht( dhtpin, dhttype );
//************************************
//**********Time Parameters***********
int Date = 0, minutes = 0;
//************************************
//**********File Parameters***********
//const char *proto_file_path = "/mnt/sda1/";
const char  file_path[] = "/mnt/sda1/Datalog.txt";
//************************************
//**********UV sensor Parameters***********
#define UVpin A0
//************************************

void setup() {
  // Initialize Objects
  Bridge.begin();
  Serial.begin(9600);
  FileSystem.begin();
  dht.begin();

  Serial.println("System start\nPlease check WI-FI parameters and if terminal online!");
  /*file_path = proto_file_path;
    strcat(file_path, GetDateFileName());*/
  WriteStartInfo();
  Serial.println(file_path);
  analogReference(INTERNAL);
}


void loop() {
  String dataString = TimeStampAndGetMinutes();
  static int min_rec = minutes;
  static int date_rec = Date;

  //Serial.println(minutes);
  if (minutes == min_rec) //minutes%5 == 0&& seconds==0
  {
    min_rec = (minutes >= 55 ? minutes - 55 : minutes + 5);

    //char file_path[] = "/mnt/sda1/Datalog.txt";
    String Humi, Temp, UVindex;

    dataString += " = ";
    // read three sensors and append to the string:
    Humi = (String)dht.readHumidity();
    Temp = (String)dht.readTemperature();
    UVindex = (String)analogRead(UVpin);

    //Call HTTPGet funtion
    postToThinspeak(Temp, Humi, UVindex);

    dataString += DHTstrForSD(Temp, Humi);
    dataString += UVstrForSD(UVindex);
    WriteData(dataString);
  }

  /*if (Date != date_rec)
    {
    file_path = proto_file_path;
    strcat(file_path, GetDateFileName());
    WriteStartInfo();
    }*/
}
