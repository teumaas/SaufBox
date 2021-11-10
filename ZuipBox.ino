// Zuipbox v1.00
// setCursor(Left/Right, Up/Down);

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>    

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>

#include <ArduinoJson.h>
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>

// Application version.
#define VERSION 1.0

#define WAITING_TIME 5
#define TEST_DURATION 4
#define SPEAKER_PIN 15
String ADMIN_UID = "5DF0CA2";

const char* API_URL = "http://192.168.1.20/api/";

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MQ3pin 0
WiFiManager wifiManager;
SoftwareSerial SWSerial( D5, D6 );
PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );

const unsigned char logo [] PROGMEM = {
  0x00, 0xf0, 0x00, 0x03, 0x1e, 0x00, 0x0e, 0x03, 0xc0, 0x10, 0x00, 0x40, 0x20, 0x00, 0x20, 0x20, 
  0x01, 0x40, 0x23, 0xf9, 0xc0, 0x2c, 0x05, 0x40, 0x28, 0x05, 0x4c, 0x28, 0x05, 0x74, 0x2a, 0x26, 
  0x54, 0x2a, 0x22, 0x44, 0x3a, 0x22, 0x44, 0x0a, 0x22, 0x44, 0x0a, 0x22, 0x44, 0x0a, 0x22, 0x44, 
  0x0a, 0x22, 0x44, 0x0a, 0x22, 0x74, 0x0a, 0x22, 0x44, 0x08, 0x00, 0x7c, 0x08, 0x00, 0x40, 0x0f, 
  0xf5, 0xc0, 0x08, 0x00, 0x80, 0x07, 0xff, 0x80
};

const unsigned char gears [] PROGMEM = {
  0x00, 0xe0, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x19, 0xfb, 0x00, 0x00, 0x3f, 0xff, 0x80, 0x00, 
  0x3f, 0xff, 0x80, 0x00, 0x3f, 0xff, 0x80, 0x00, 0x3f, 0x1f, 0x80, 0x00, 0x3e, 0x0f, 0x80, 0x00, 
  0xfc, 0x07, 0xe0, 0x00, 0xfc, 0x07, 0xe0, 0x00, 0xfc, 0x07, 0xe3, 0x80, 0x7e, 0x0f, 0xc3, 0xd8, 
  0x3f, 0x1f, 0xbf, 0xfc, 0x1f, 0xff, 0x3f, 0xfc, 0x3f, 0xff, 0xbf, 0xfc, 0x3f, 0xff, 0xbe, 0x3e, 
  0x1b, 0xff, 0x3c, 0x1f, 0x01, 0xe0, 0x7c, 0x1f, 0x00, 0xe0, 0xfc, 0x1e, 0x00, 0x08, 0x7c, 0x3c, 
  0x00, 0x1c, 0x1f, 0xfc, 0x00, 0xff, 0x9f, 0xfe, 0x00, 0xff, 0x9f, 0xfc, 0x00, 0xf7, 0x9f, 0xe0, 
  0x01, 0xe3, 0xc1, 0xc0, 0x03, 0xc1, 0xe0, 0x00, 0x01, 0xe3, 0xc0, 0x00, 0x00, 0xf7, 0x80, 0x00, 
  0x00, 0xff, 0x80, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00
};

void setup() {
  //Init
  Serial.begin(9600);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("Welkom, dit is ZuipBox v" + String(VERSION));
  Serial.println("----------------------------------");

  //Needed for display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1, 2);
  display.drawBitmap(50, 20, gears, 32, 32, WHITE);
  display.display();
 
  pinMode(SPEAKER_PIN, OUTPUT);

  delay(5000);
  
  wifiManager.setBreakAfterConfig(true);

  if (!wifiManager.autoConnect("ZuipBox", "bier1234")) {
    
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  beginScreen();
}

void loop() {
  //Starting code.
  beginScreen();
}

void adminMode() {
  // This mode is for settings purposes.
  // Work inprogress.

  display.clearDisplay();
  display.setTextSize(1, 2);
  display.drawBitmap(10, 20, gears, 32, 32, WHITE);
  display.setCursor(46, 32);
  display.println("Hard reset..");
  display.display();
 
  //Resets the Wi-Fi
  wifiManager.resetSettings();
  ESP.eraseConfig(); 
  delay(2000);
  ESP.reset(); 
 
  delay(6000);
  beginScreen();
}

// Blowtesting
void blowTest(String UID){
  String cUid = UID;
  String aScore;
  
  Serial.println("Posted UID: " + cUid);
  
  tone(SPEAKER_PIN, 1200, 4000);
  
  display.clearDisplay();
  display.display();
  
  display.setTextSize(1, 2);
  display.setCursor(32, 30);
  display.println("Blazen kut!");
  display.display();

  for (int i = TEST_DURATION; i >= 0; i--) {
    delay(1000);
  }
  
  display.clearDisplay();
  display.display();

  display.setTextSize(1, 1);
  display.setCursor(8, 2);
  display.println("Score is verstuurd!");
  
  display.setTextSize(1, 2);
  display.setCursor(14, 30);

  aScore = "200";

    //  apiRequest(cUid, aScore);
  display.println("Score: " + aScore + "µg/l");
  
  display.display();

  delay(6000);

  beginScreen();
}

// Start screen of Device..
void beginScreen(){
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  display.clearDisplay();

  display.drawBitmap(10, 28, logo, 24, 24, WHITE);

  display.setTextColor(WHITE);
  
  display.setTextSize(1, 1);
  display.setCursor(20, 2);
  display.println("Scan een tag...");

  display.setTextSize(1, 2);
  display.setCursor(38, 35);
  display.println("ZuipBox v" + String(VERSION));
  display.setCursor(38, 18);
  display.println("Value: " + String(analogRead(MQ3pin)));

  display.display();

  nfc.begin();
  nfc.SAMConfig();
  
  if (success) {
    //Captured UID from uid[i]
    String currentUID;
    
    for (uint8_t i = 0; i < uidLength; i++)
    {
      currentUID += String(uid[i], HEX);
    }
    
    currentUID.toUpperCase();
    
    if(currentUID == ADMIN_UID)
    {
      return adminMode();
    }
    
    display.clearDisplay();
    
    digitalWrite(SPEAKER_PIN, HIGH);
    tone(SPEAKER_PIN, 800, 500);
    Serial.println(" ");
    Serial.print("UID: ");
    
    display.setTextSize(1, 2);
    display.setCursor(24, 32);
    display.println("UID: " + currentUID);
    display.display();

    Serial.print(currentUID);
    Serial.println();
   
    display.setTextSize(1, 1);
    display.setCursor(10, 2);
    display.println("Start blaastest:");
    display.setCursor(108, 2);
    display.println(WAITING_TIME);    
    display.display();
    
    delay(3000);

    display.setCursor(108, 2);
    display.fillRect(108, 2, 12, 12, BLACK);
    display.display();
     
    for (int i = WAITING_TIME; i >= 0; i--) {
      display.setCursor(108, 2);
      display.fillRect(108, 2, 12, 12, BLACK);
      display.display();
      
      digitalWrite(SPEAKER_PIN, HIGH);
      tone(SPEAKER_PIN, 1000, 200);
      
      display.setCursor(108, 2);
      display.println(String(i));
      display.display();

      delay(1000);
    }
    
    blowTest(currentUID);
  }
}
