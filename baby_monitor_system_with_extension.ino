#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
#include <WiFi.h>
String apiKey = "MFCCWJQ4BZRLMK8B";                  //  Enter your Write API key from ThingSpeak
const char *ssid =  "TelstraDE97B7";                                    // replace with your wifi ssid and wpa2 key
const char *pass =  "s45rcyattt";
const char* server = "api.thingspeak.com";
WiFiClient client;

#include <DHT.h>
#include "SD.h"
#include <Wire.h>




//Variables
float hum;  //Stores humidity value
float temp; //Stores temperature value
float high_temp= 35;
float low_temp= 20;
float high_sound= 150; 



//Constants
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
int LED1=13;
int LED2=2;
int LED3=10;
int MOTION_SENSOR_PIN_1=6;
int MOTION_SENSOR_PIN_2=6;
int SOUND_SENSOR_PIN= 5;


DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino



void setup() 
{
    Serial.begin(115200);
    delay(10);
    Serial.println("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    dht.begin();
  pinMode(MOTION_SENSOR_PIN_1, INPUT);
  pinMode(MOTION_SENSOR_PIN_2, INPUT);
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
 
}
void loop() 
{

  hum = dht.readHumidity();
  temp= dht.readTemperature();
  int motion_sensor_1=digitalRead(MOTION_SENSOR_PIN_1);
  int motion_sensor_2=digitalRead(MOTION_SENSOR_PIN_2);
  int sound_digital = digitalRead(SOUND_SENSOR_PIN);
  //changing temperature parameter to celsius
 if (client.connect(server,80))                                 //   "184.106.153.149" or api.thingspeak.com
    {  
String data_to_send = apiKey;
    data_to_send += "&field1=";
    data_to_send += String(temp);
    data_to_send += "&field2=";
    data_to_send += String(hum);
    data_to_send += "&field3=";
    data_to_send += String(motion_sensor_1);
    data_to_send += "&field4=";
    data_to_send += String(motion_sensor_2);
    data_to_send += "&field5=";
    data_to_send += String(sound_digital);
    data_to_send += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data_to_send.length());
    client.print("\n\n");
    client.print(data_to_send);
    delay(1000);


      //LED STATUS
  if(motion_sensor_2==1 && motion_sensor_1==0)
  {
        digitalWrite(LED1,HIGH);
        delay(1000);
        digitalWrite(LED1,LOW);
  }
  
    if(temp>high_temp && temp<low_temp )
  {
        digitalWrite(LED2,HIGH);
        delay(2000);
        digitalWrite(LED2,LOW);
  }
  
  if(sound_digital == 1)
  {
        digitalWrite(LED3,HIGH);
        delay(2000);
        digitalWrite(LED3,LOW);
  }
    
}
    client.stop();
   
}
