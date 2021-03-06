#include <IotWebConf.h>
#include <PubSubClient.h>
#include <LEDMatrixDriver.hpp>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "iotMatrix";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "iotMatrix8266";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "20200421"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN D2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Callback method declarations.
void configSaved();
boolean formValidator();

DNSServer dnsServer;
WebServer server(80);

char gMqttServer[STRING_LEN];
char gMqttPort[NUMBER_LEN];
char gMqttTopic[STRING_LEN];
char gMqttUser[STRING_LEN];
char gMqttPass[STRING_LEN];

//char intParamValue[NUMBER_LEN];
//char floatParamValue[NUMBER_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

IotWebConfSeparator pseparator1 = IotWebConfSeparator("MQTT Info");
IotWebConfParameter pParm1 = IotWebConfParameter("MQTT Server" , "gMqttServer"  , gMqttServer , STRING_LEN);
IotWebConfParameter pParm2 = IotWebConfParameter("MQTT Port"   , "gMqttPort"    , gMqttPort   , NUMBER_LEN);
IotWebConfParameter pParm3 = IotWebConfParameter("MQTT Topic"  , "gMqttTopic"   , gMqttTopic  , STRING_LEN);
IotWebConfParameter pParm4 = IotWebConfParameter("MQTT User"   , "gMqttUser"    , gMqttUser   , STRING_LEN);
IotWebConfParameter pParm5 = IotWebConfParameter("MQTT Pass"   , "gMqttPass"    , gMqttPass   , STRING_LEN);
 
/*
IotWebConfParameter stringParam = IotWebConfParameter("String param", "stringParam", stringParamValue, STRING_LEN);
IotWebConfSeparator separator1 = IotWebConfSeparator();
IotWebConfParameter intParam = IotWebConfParameter("Int param", "intParam", intParamValue, NUMBER_LEN, "number", "1..100", NULL, "min='1' max='100' step='1'");
// -- We can add a legend to the separator
IotWebConfSeparator separator2 = IotWebConfSeparator("Calibration factor");
IotWebConfParameter floatParam = IotWebConfParameter("Float param", "floatParam", floatParamValue, NUMBER_LEN, "number", "e.g. 23.4", NULL, "step='0.1'");
*/
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  
  iotWebConf.addParameter(&pseparator1);
  iotWebConf.addParameter(&pParm1);
  iotWebConf.addParameter(&pParm2);
  iotWebConf.addParameter(&pParm3);
  iotWebConf.addParameter(&pParm4);
  iotWebConf.addParameter(&pParm5);
  
  //iotWebConf.addParameter(&stringParam);
  //iotWebConf.addParameter(&intParam);
  //iotWebConf.addParameter(&separator2);
  //iotWebConf.addParameter(&floatParam);
  
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.getApTimeoutParameter()->visible = true;

  // -- Initializing the configuration.
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  Serial.println("Ready.");
  
  // Allow the hardware to sort itself out
  delay(1500);
  mqttClient.setServer(gMqttServer, (int)gMqttPort);
  
  if (mqttClient.connect("myClientID")) {
    // connection succeeded
    Serial.println("MQTT Connected");
  } else {
    // connection failed
    // mqttClient.state() will provide more information
    // on why it failed.
    Serial.println("MQTT NOT Connected");
  }
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
  
  mqttClient.loop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 03 Custom Parameters</title></head><body>Hello world!";
  s += "<ul>";
//  s += "<li>String param value: ";
//  s += stringParamValue;
//  s += "<li>Int param value: ";
//  s += atoi(intParamValue);
// s += "<li>Float param value: ";
//  s += atof(floatParamValue);
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
}

boolean formValidator()
{
  Serial.println("Validating form.");
  boolean valid = true;

//  int l = server.arg(stringParam.getId()).length();
//  if (l < 3)
//  {
//    stringParam.errorMessage = "Please provide at least 3 characters for this test!";
//    valid = false;
//  }

  return valid;
}
