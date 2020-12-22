
#include "ThingSpeak.h"   //thingspeak library
#include <WiFi.h>    //esp library

#define DEBUG 0   //preprocessor macro

char ssid[] = "amma trophy";   // your network SSID (name) 
char pass[] = "76858086";   // your network password
WiFiClient  client;

unsigned long myChannelNumber = 1245532;
const char * myWriteAPIKey = "ULB0449CAQY046M4";


//variable to store data
int person = -1;
int hightemp = -1;
String data;
int check = 0;
long oldmillis ;

void setup() {
  Serial.begin(9600);  //Initialize serial

  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  oldmillis = millis();
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  if(Serial.available())   //check for serial data coming from bbb
  {
    data = Serial.readString();    //read the data
    if(DEBUG) Serial.print("Data : ");
    if(DEBUG) Serial.println(data);
    
    String temp = data.substring(0,data.indexOf(','));    //get substring of a string
    if(DEBUG)  Serial.print("Person: ");
    if(DEBUG) Serial.println(temp);
    person = temp.toInt();

    String temp2 = data.substring(data.indexOf(',')+1);
    if(DEBUG) Serial.print("High Temp Persons: ");
    if(DEBUG) Serial.println(temp2);
    hightemp = temp2.toInt();

    if(data.indexOf(',') != -1)
    {
      check = 1;
    }
    else
    {
      check = 0;
    }
    
  }

if(person != -1 and check > 0 and (millis() - oldmillis)>16000 )
{
  if(DEBUG) Serial.print("Uploading Data: ");
  if(DEBUG) Serial.print(person);
  if(DEBUG) Serial.print(",");
  if(DEBUG) Serial.println(hightemp);

  ThingSpeak.setField(1, person);   //set the fields for uploading
  ThingSpeak.setField(2, hightemp);
  
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);  //write data into fields
  
  if(x == 200){
    if(DEBUG) Serial.println("Channel update successful.");
  }
  else{
    if(DEBUG) Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  oldmillis = millis();
  person = -1;
  check = 0;
  hightemp = -1;
}
delay(100);

}
