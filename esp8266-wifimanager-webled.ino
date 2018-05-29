#include <WiFiManager.h> // from library manager
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>

#include <FS.h>
//#include "SPIFFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>  //https://github.com/bblanchon/ArduinoJson
#include <ESP8266HTTPClient.h>

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_D1_PIN_ORDER
#include <FastLED.h>
#define NUM_LEDS 16
#define DATA_PIN 14
CRGB leds[NUM_LEDS];


//define your default values here, if there are different values in config.json, they are overwritten.
char apikey[40];
char redC[4] = {'0'};
char greenC[4] = {'0'};
char blueC[4] = {'0'};

//flag for saving data
bool shouldSaveConfig = true;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();



  //read configuration from FS json
  Serial.println("mounting FS...");
//  SPIFFS.begin (true);

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(apikey, json["apikey"]);
          strcpy(redC, json["redC"]);
          strcpy(greenC, json["greenC"]);
          strcpy(blueC, json["blueC"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_apikey("Api Key", "Api Key", apikey, 40);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Set to continue on failure so I can keep the parameters
  wifiManager.setBreakAfterConfig(true);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  wifiManager.addParameter(&custom_apikey);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(40);
  // DAN: Ok for some reason this does not like to connect the first time, so this
  // timeout forces a reset which then lets it connect to the network and start
  // doing stuff
  // someone has to finish setup before the timeout ends or it reboots
  // and it could reboot in middle of setup
  // UPDATE FIXED IT!!! by adding the lines in WiFiManeger.cpp (216) to reset
  // instead of waiting for this reset lol

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Dan Test // ESP8266")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
//    ESP.restart();
//    delay(5000);
  }

  //read updated parameters
  strcpy(apikey, custom_apikey.getValue());

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["apikey"] = apikey;
    json["redC"] = redC;
    json["greenC"] = greenC;
    json["blueC"] = blueC;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println();
// for debugging:
  Serial.println("Api Key: ");
  Serial.println(apikey);
  Serial.println("Red Color: ");
  Serial.println(redC);
  Serial.println("Green Color: ");
  Serial.println(greenC);
  Serial.println("Blue Color: ");
  Serial.println(blueC);
  Serial.println("%");

//  WiFi.disconnect(true); //erases store credentially
//  SPIFFS.format();  //erases stored values
  Serial.println("Done");
  
  // check if need initial restart
  // This will be determined why whether our localip is 0.0.0.0
  // Which means it was the first boot and it did its stupid fail crap
  if(WiFi.localIP().toString() == "0.0.0.0"){
    delay(2000);
    ESP.restart();
  }

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  // initial LED display from the json and update leds
  Serial.println("Displaying LEDS");
  Serial.println(atoi(redC));
  Serial.println(atoi(greenC));
  Serial.println(atoi(blueC));
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i].r = atoi(redC);
    leds[i].g = atoi(greenC);
    leds[i].b = atoi(blueC);
    FastLED.show(); 
    delay(30);
  };
  
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin("http://104.236.103.190/getcolors"); //Specify the URL
    int httpCode = http.GET();                                        //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);

        // parse the payload
        StaticJsonBuffer<200> jsonBuffers;
        JsonObject& colorData = jsonBuffers.parseObject(payload);
        Serial.println("HERE IS THE JSON DATATA");
        colorData.printTo(Serial);
//        const char* newRedC = colorData["red"];
//        Serial.print("HERE IS RED: ");
//        Serial.println(newRedC);
        strcpy(redC, colorData["red"]);
        strcpy(greenC, colorData["green"]);
        strcpy(blueC, colorData["blue"]);
        Serial.println("PARSED THE NEW JSON");

        // update the stored json
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
        json["apikey"] = apikey;
        json["redC"] = redC;
        json["greenC"] = greenC;
        json["blueC"] = blueC;
        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile) {
          Serial.println("failed to open config file for writing");
        }
        Serial.println("Printing new JSON:::::::");
        json.printTo(Serial);
        json.printTo(configFile);
        configFile.close();
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }
  Serial.println(apikey);
  delay(4000);

///////////////////////////////////////////////////////////////////////

//Uncomment these lines of code if you want to reset the device 
//It could be linked to a physical reset button on the device and set
//to trigger the next 3 lines of code.
  //WiFi.disconnect(true); //erases store credentially
  //SPIFFS.format();  //erases stored values
  //ESP.restart();
///////////////////////////////////////////////////////////////////////


}
