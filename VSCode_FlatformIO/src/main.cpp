#include <Arduino.h>
#define BUILTIN_LED 16
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <Ticker.h>
#include <BlynkSimpleEsp8266.h>
Ticker ticker;

/* Private typedef -----------------------------------------------------------*/
#define BLYNK_PRINT Serial
#define light_sensor 4
#define rain_sensor 5
/* Private variables ---------------------------------------------------------*/
char auth[] = "75ca160b7a4b43a8b5e200e556afcad1";
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "StackingBrick";
char pass[] = "1234554320";
unsigned long int time1;
int a, b, c, d;
const int button1 = 2;
const int button2 = 0;

int x, y, z;
/* Variables Use for shift-out74HC595---------------------------------------------------------*/
//Default relay1 and relay2 is TRUE. if true, curtain move to center, if false, curtain move out to 2 direction
bool value1, value2, value3, value4, value5, value6, enable_sensor1, enable_sensor2 ; //6 value to control relay, enable_sensor to consider using sensor to control or not
int value; //Sum value to shift-out
int latchPin = 14;
//chân SH_CP(11) của 74HC595
int clockPin = 12;
//Chân DS (14)của 74HC595
int dataPin = 16;
/* Private function prototypes -----------------------------------------------*/
void xulynutnhan(void);
void shiftout_74hc595(void);
void button_processing();
void sensor_processing();
void set_value_blynk_default();
WidgetLCD lcd(V5);
/* Blynk function prototypes -----------------------------------------------*/
BLYNK_WRITE(V0)
{
  if (param.asInt() == 0) {
    value1 = 0;
  }
  else
    value1 = 1;
  // process received value
}
BLYNK_WRITE(V1)
{
  if (param.asInt() == 0)
    value2 = 0;
  else
    value2 = 1;
  // process received value
}
BLYNK_WRITE(V2)
{
  if (param.asInt() == 0)
    value3 = 0;
  else
    value3 = 1;
  // process received value
}
BLYNK_WRITE(V3)
{
  if (param.asInt() == 0)
    value4 = 0;
  else
    value4 = 1;
  // process received value
}

BLYNK_WRITE(V4)
{
  if (param.asInt() == 0)
    value5 = 0;
  else
    value5 = 1;
  // process received value
}

BLYNK_WRITE(V5)
{
  if (param.asInt() == 0)
    value6 = 0;
  else
    value6 = 1;
  // process received value
}

BLYNK_WRITE(V6)
{
  if (param.asInt() == 0)
    enable_sensor1 = 0;
  else {
    enable_sensor1 = 1;
    Serial.println("Use light sensor");
  }
  // process received value
}

BLYNK_WRITE(V7)
{
  if (param.asInt() == 0)
    enable_sensor2 = 0;
  else {
    enable_sensor2 = 1;
    Serial.println("Use rain sensor");
  }
  // process received value
}

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("RemThongMinh","123456789")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);
  Blynk.config("75ca160b7a4b43a8b5e200e556afcad1", "gith.cf", 8442);
  pinMode(latchPin , OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(light_sensor, INPUT_PULLUP);
  pinMode(rain_sensor, INPUT_PULLUP);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0);
  digitalWrite(latchPin, HIGH);
  delay(1000);
  lcd.clear(); //Use it to clear the LCD Widget
  lcd.print(1, 0, "LamChuCongNghe");
  lcd.print(4, 1, "Welcome !"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  time1 = millis();
  set_value_blynk_default();
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  button_processing();
  sensor_processing();
  shiftout_74hc595();
  //blynk_response();
  if ( (millis() - time1) > 1000 ) {
    Serial.print("Value1:");
    Serial.print(value1);
    Serial.print("   ");
    Serial.print("Value2:");
    Serial.print(value2);
    Serial.print("   ");
    Serial.print("   ");
    Serial.print("Value3:");
    Serial.print(value3);
    Serial.print("   ");
    Serial.print("Value4:");
    Serial.print(value4);
    Serial.print("   ");
    Serial.print("Value5:");
    Serial.print(value5);
    Serial.print("   ");
    Serial.print("Value6:");
    Serial.print(value6);
    Serial.print("   ");
    Serial.print("Sen1:");
    Serial.print(digitalRead(light_sensor));
    Serial.print("   ");
    Serial.print("sen2:");
    Serial.print(digitalRead(rain_sensor));
    Serial.print("   ");
    Serial.print("Value:");
    Serial.print(value);
    Serial.println("   ");

    time1 = millis();

  }
}

void sensor_processing() {
  //  const int light_sensor = 4;
  //  const int rain_sensor = 5;
  if (enable_sensor1 == 1) { // Using sensor to control or not
    if (digitalRead(light_sensor) == 1) {  //Enough light, close curtains
      value1 = 0;
      value2 = 0;
      Blynk.virtualWrite(V0, value1);
      Blynk.virtualWrite(V1, value2);
    }
    else {
      value1 = 1;
      value2 = 1;
      Blynk.virtualWrite(V0, value1);
      Blynk.virtualWrite(V1, value2);
    }
  }

  if (enable_sensor2 == 1) { // Using sensor to control or not
    if (digitalRead(rain_sensor) == 0) {  //If rain, close curtain
      value1 = 1;
      value2 = 1;
      Blynk.virtualWrite(V0, value1);
      Blynk.virtualWrite(V1, value2);
    }
  }
}


void button_processing() {
  /*-----------------*/ // Relay and button 1
  if (digitalRead(button1) == 1)
    a = 0;
  if (digitalRead(button1) == 0)
    a++;
  if (a == 10 ) {
    value1 = 1;
    value2 = 1;
    Blynk.virtualWrite(V0, value1);
    Blynk.virtualWrite(V1, value2);
  }

  //////////

  /*-----------------*/ // Relay and button 2
  if (digitalRead(button2) == 1)
    b = 0;
  if (digitalRead(button2) == 0)
    b++;
  if (b == 10 ) {
    value1 = 0;
    value2 = 0;
    Blynk.virtualWrite(V0, value1);
    Blynk.virtualWrite(V1, value2);
  }
  //////////

  //  /*-----------------*/ // Relay and button 3
  //  if (digitalRead(button3) == 1)
  //    c = 0;
  //  if (digitalRead(button3) == 0)
  //    c++;
  //  if (c == 10 ) {
  //    value3 = !value3;
  //    Blynk.virtualWrite(V3, value3);
  //  }
  //
  //  //////////
  //
  //  /*-----------------*/ // Relay and button 4
  //  if (digitalRead(button4) == 1)
  //    d = 0;
  //  if (digitalRead(button4) == 0)
  //    d++;
  //  if (d == 10 ) {
  //    value4 = !value4;
  //    Blynk.virtualWrite(V4, value4);
  //  }

}

void shiftout_74hc595(void) {
  value = value1 * 2 + value2 * 4 + value3 * 8 + value4 * 16 + value5 * 32 + value6 * 64;
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, value);
  //shiftOut(dataPin, clockPin, MSBFIRST, so1[mode1]);
  digitalWrite(latchPin, HIGH);
}

void blynk_response() {


}

void set_value_blynk_default() { //Sen default data to Blynk
  Blynk.virtualWrite(V0, value1);
  Blynk.virtualWrite(V1, value2);
  Blynk.virtualWrite(V2, value3);
  Blynk.virtualWrite(V3, value4);
  Blynk.virtualWrite(V4, value5);
  Blynk.virtualWrite(V5, value6);
  Blynk.virtualWrite(V6, enable_sensor1);
  Blynk.virtualWrite(V7, enable_sensor2);
}