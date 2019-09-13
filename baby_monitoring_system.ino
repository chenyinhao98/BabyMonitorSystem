/**
   Detects 'Active' and 'Inactive' motion and logs it with timestamps to a .csv file in SDCard.
   Uses  HCSR505 PIR Passive Infra Red Motion Detector.
   Modified code from https://learn.adafruit.com/adafruit-data-logger-shield/using-the-real-time-clock-3
   https://github.com/adafruit/Light-and-Temp-logger
   Author: Niroshinie Fernando 
*/

//Libraries
#include "SD.h"
#include <Wire.h>
#include "RTClib.h"
#include <DHT.h>

#define LOG_INTERVAL  1000 // mills between entries. 
// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()



#define ECHO_TO_SERIAL   1 // echo data to serial port. 


//Variables
char activeMotion [] = "Active";
char inactiveMotion [] = "Inactive";
char* state ;
char activeSound [] = "HIGH";
char inactiveSound [] = "LOW";
char* sound_state ;
float hum;  //Stores humidity value
float temp; //Stores temperature value
float high_temp= 35;
float low_temp= 19;
float high_sound= 150; 



//Constants
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
int LED1=13;
int LED2=2;
int LED3=10;
int MOTION_SENSOR_PIN=6;

RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void setup()
{
  Serial.begin(9600);

  // initialize the SD card
  initSDcard();

  // create a new file
  createFile();


  /**
   * connect to RTC
     Now we kick off the RTC by initializing the Wire library and poking the RTC to see if its alive.
  */
  initRTC();


  /**
     Now we print the header. The header is the first line of the file and helps your spreadsheet or math program identify whats coming up next.
     The data is in CSV (comma separated value) format so the header is too: "millis,stamp, datetime,hum,temp" the first item millis is milliseconds since the Arduino started,
     stamp is the timestamp, datetime is the time and date from the RTC in human readable format, hum is the humidity and temp is the temperature read.
  */
  logfile.println("millis,stamp,datetime,motion,humidity,temperature,sound");
#if ECHO_TO_SERIAL
  Serial.println("millis,stamp,datetime,motion,humidity,temperature,sound");
#endif //ECHO_TO_SERIAL

  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  dht.begin();


  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  
}

void loop()
{
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");
#endif

  // fetch the time
  now = RTC.now();
  // log time
  logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(", ");
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 2000
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
#endif //ECHO_TO_SERIAL

  //Read data and store it to variable
  if (digitalRead(MOTION_SENSOR_PIN) == HIGH)
  {
    //    Serial.println("Active");
    state = activeMotion;
  }
  else 
  {
    //    Serial.println("Inactive");
    state = inactiveMotion;
  }


  

  hum = dht.readHumidity();
  temp= dht.readTemperature(); 
  int sound_analog = analogRead(A0);

  logfile.print(", ");
  logfile.print(state);
  logfile.print(", ");
  logfile.print(hum);
  logfile.print(", ");
  logfile.print(temp);
  logfile.print(", ");
  logfile.println(sound_analog);

#if ECHO_TO_SERIAL
  Serial.print(", ");
  Serial.print(state);
  Serial.print(", ");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(", ");
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.print(" Celsius");
  Serial.print(", ");
  Serial.print("Sound: ");
  Serial.println(sound_analog);




  //LED STATUS
  if(state==activeMotion)
  {
        digitalWrite(LED3,HIGH);
        delay(500);
        digitalWrite(LED3,LOW);
  }
  if(sound_analog>high_sound)
  {
        digitalWrite(LED2,HIGH);
        delay(500);
        digitalWrite(LED2,LOW);
  }
  if(temp>high_temp || temp<low_temp)
  {
        digitalWrite(LED1,HIGH);
        delay(500);
        digitalWrite(LED1,LOW);
  }
  
  

#endif //ECHO_TO_SERIAL

  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();

  logfile.flush();
}


/**
   The error() function, is just a shortcut for us, we use it when something Really Bad happened.
   For example, if we couldn't write to the SD card or open it.
   It prints out the error to the Serial Monitor, and then sits in a while(1); loop forever, also known as a halt
*/
void error(char const *str)
{
  Serial.print("error: ");
  Serial.println(str);

  while (1);
}

void initSDcard()
{
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}

void createFile()
{
  //file name must be in 8.3 format (name length at most 8 characters, follwed by a '.' and then a three character extension.
  char filename[] = "MLOG00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = i / 10 + '0';
    filename[5] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);
}

void initRTC()
{
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL

  }
}
