/*
  PubNub over WiFi Example using Adafruit Feather M0 WINC1500
*/
  
#include <SPI.h>
#include "Adafruit_CCS811.h"

Adafruit_CCS811 ccs;

#include <WiFi101.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS; // the Wifi radio's status

const static char pubkey[] = "pub-c-c7e392ef-57af-41df-bf5b-18e583d14c19";
const static char subkey[] = "sub-c-d6f95e84-c881-11e7-9178-bafd478c18bc";
const static char channel[] = "CCS811_Test";
static char temp;


void setup() {
  /* This is the only line of code that is Feather M0 WINC1500
  specific, the rest is the same as for the WiFi101 */
  WiFi.setPins(8,7,4,2);

  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Serial set up");

  // attempt to connect using WPA2 encryption:
  Serial.println("Attempting to connect to WPA network...");
  status = WiFi.begin(ssid, pass);

  // if you're not connected, stop here:
  if ( status != WL_CONNECTED) {
    Serial.println("Couldn't get a wifi connection");
    while (true);
  } else {
    Serial.print("WiFi connecting to SSID: ");
    Serial.println(ssid);

    PubNub.begin(pubkey, subkey);
    Serial.println("PubNub set up");
  }
  Serial.begin(9600);
  
  Serial.println("CCS811 test");
  
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
}


void loop() {
  if(ccs.available()){

    if(!ccs.readData()){
      Serial.print("CO2: ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm, TVOC: ");
      Serial.print(ccs.getTVOC());
      Serial.print("ppb   Temp:");
      Serial.println(temp);
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
    temp = ccs.calculateTemperature();
  }

  /* Publish */
  
  char msg[] = "\"Why Hello There\"";
  WiFiClient *client = PubNub.publish(channel, msg);

  if (!client) {
    Serial.println("publishing error");
    delay(1000);
    return;
  }
  client->stop();

  /* Subscribe */

  PubSubClient *sclient  = PubNub.subscribe(channel);
  if (0 == sclient) {
    Serial.println("subscribe error");
    delay(1000);
    return;
  }

  while (sclient->wait_for_data()) {
    Serial.write(sclient->read());
  }
  sclient->stop();

  /* Wait a little before we go again */
  delay(1000);
}
