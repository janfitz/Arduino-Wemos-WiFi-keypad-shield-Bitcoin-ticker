#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>

#define D0 3
#define D1 1
#define D2 16
#define D3 5
#define D4 4
#define D5 14
#define D6 12
#define D7 13
#define D8 0
#define D9 2
#define D10 15

#define DISPLAY_CHARS_IN_LINE 16

LiquidCrystal lcd(D8, D9, D4, D5, D6, D7);

char* ssid[] = {"ssid_01", "ssid_02"};
char* password[] = {"password_01", "password_02"};
int networksLenght = sizeof(ssid)/sizeof(ssid[0]);

String result;
float BTC;
int conectionCounter = 0;
int ssidCounter = 0;
int passwordCounter = 0;
int delayStatus = 0;
int delayCounter = 60;

byte bitcoinSign[8] = { B01010, B11110, B10011, B11110, B10011, B11110, B01010};
byte wifiSign[8] = { B01110, B10001, B00100, B01010, B00000, B00100, B00000};

void setup() {

  // Init special byte characters
  lcd.createChar(0, bitcoinSign);
  lcd.createChar(1, wifiSign);

  // Init display / clear and set dimensions
  lcd.clear();
  lcd.begin(16, 2);

  // Try to connect to some of defined Wifi networks
  while ((wifiConnect(ssid[ssidCounter], password[passwordCounter]) == false) && (ssidCounter <= networksLenght)) {
    ssidCounter++;
    passwordCounter++;
    conectionCounter = 0;
    
    // It was not pissible to connect to any network - delete counters and loop again
    if (ssidCounter == networksLenght) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Can't connect");
      lcd.setCursor(0, 1);
      lcd.print("Trying again...");
      delay(2000);
      ssidCounter = 0;
      passwordCounter = 0;
    }
  }

  // Connection established - print it out
  lcd.clear();
  lcd.print("Connected to");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.SSID());
  delay(2000);

}

void loop() {
  getBitcoinData();
  displayBitcoin(BTC, delayStatus, delayCounter);
}

/*
   Connect to specified wifi network
*/
bool wifiConnect(char* ssid, char* password) {
  int cursorPosition = 0;
  WiFi.begin(ssid, password);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.setCursor(2, 0);
  lcd.print(WiFi.SSID());

  // Test if conection was established
  while (WiFi.status() != WL_CONNECTED) {
    if (conectionCounter < DISPLAY_CHARS_IN_LINE) {
      delay(500);
      lcd.setCursor(cursorPosition, 2);
      lcd.print(".");
      cursorPosition++;
      conectionCounter++;
    }
    // Connection timeout
    else {
      return false;
    }
  }
  // Connected sucessfull
  return true;
}

/*
   Client function to send/receive GET request data.
*/

void getBitcoinData()
{
  // Connect to Coindesk API and store json response
  HTTPClient http;
  http.begin("http://api.coindesk.com/v1/bpi/currentprice/USD.json");
  int statusCode = http.GET();
  result = http.getString();
  http.end();

  char jsonArray [result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  StaticJsonDocument<1024> doc;
  JsonObject root = doc.as<JsonObject>();
  auto error = deserializeJson(doc, jsonArray);
  if (error) {
    Serial.print("deserializeJson() failed with code ");
    Serial.println(error.c_str());
    return;
  }

  float btcRate = doc["bpi"]["USD"]["rate_float"];
  BTC = btcRate;

}

/*
   Print out actual bitcoin price rate and count 60 second to another data fetching
*/
void displayBitcoin(float BTC, int delayStatus, int delayCounter)
{
  delayCounter = 60;
  delayStatus = 0;

  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.write(byte(0));
  lcd.setCursor(2, 0);

  lcd.print("Bitcoin price");
  lcd.setCursor(0, 1);
  lcd.print("$");
  lcd.setCursor(1, 1);
  lcd.print(BTC);

  while (delayStatus != 60000) {
    if (delayCounter >= 10) {
      lcd.setCursor(14, 1);
    }
    else {
      lcd.setCursor(14, 1);
      lcd.print(" ");
      lcd.setCursor(15, 1);
    }

    lcd.print(delayCounter);
    delayStatus = delayStatus + 1000;
    delayCounter--;
    delay(1000);
  }
}
