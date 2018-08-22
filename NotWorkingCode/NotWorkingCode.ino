/***************************************************************************
  This is a library for the CCS811 air 

  This sketch reads the sensor

  Designed specifically to work with the Adafruit CCS811 breakout
  ----> http://www.adafruit.com/products/3566

  These sensors use I2C to communicate. The device's I2C address is 0x5A

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include "Adafruit_CCS811.h"
#include <ArduinoJson.h>

Adafruit_CCS811 ccs;

#include <SPI.h>
#include <PubNub.h>


static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS; // the Wifi radio's status

const static char pubkey[] = "pub-c-c7e392ef-57af-41df-bf5b-18e583d14c19";
const static char subkey[] = "sub-c-d6f95e84-c881-11e7-9178-bafd478c18bc";
const static char channel[] = "CCS811_Test";

Adafruit_CCS811 ccs;*/

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
    while (!Serial) {
    }
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
    float temp = ccs.calculateTemperature();
    if(!ccs.readData()){
     Serial.print("CO2: ");
     Serial.print(ccs.geteCO2());
     Serial.print("ppm, TVOC: ");
     Serial.print(ccs.getTVOC());
     Serial.print("ppb   Temp:");
     Serial.println(temp);

     //Serial.print("TESTING");
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
     StaticJsonBuffer<200> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();

   JsonArray& data = root.createNestedArray("data");
  data.add("CO2:");
  data.add((ccs.geteCO2()));
  data.add("TVOC:");
  data.add((ccs.getTVOC()));
  data.add("Temp:");
  data.add((temp));

  root.printTo(Serial);
  // Thi
  Serial.println();
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

  root.prettyPrintTo(Serial);
  }
  delay(500);

  WiFiClient *client = PubNub.publish(JsonObject);
  


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
}
