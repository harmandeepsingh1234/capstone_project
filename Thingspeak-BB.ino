
#include "ThingSpeak.h"    //library for thingspeak
#include <WiFi.h>         //library for wifi

char ssid[] = "amma trophy";   // your network SSID (name) 
char pass[] = "76858086";   // your network password
WiFiClient  client;

unsigned long myChannelNumber = 1245532;
const char * myWriteAPIKey = "ULB0449CAQY046M4";

int person = -1;

void setup() {
  Serial.begin(9600);  //Initialize serial

  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
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

  if(Serial.available())   //checking if serial data is availalble
  {
    person = Serial.parseInt();        //storing the recieved data in person
  }

if(person != -1)
{
  Serial.print("Uploading Data: ");
  Serial.println(person);
  
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeField(myChannelNumber, 1, person, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  

  
  delay(20000); // Wait 15 seconds to update the channel again
}
delay(100);
  person = -1;
}
