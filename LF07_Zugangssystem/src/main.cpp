#include <tr064.h>
#include <string.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
using namespace std;

#define BUTTON 32
#define SCK 18
#define SDA 21
#define MOSI 23
#define MISO 19
#define RST 0
 
#define GRUEN 27
#define ROT 26

void wifi_connection();
void buttonRot();
void buttonGruen();
void klingeln();

MFRC522 cardReader(SDA, RST);

byte karte[] = {0xA3, 0xBA, 0x8F, 0x94};
byte chip[] = {0x23, 0x22, 0xD7, 0x91};

String lastRfid = "";

const char* ssid     = "anouri SNS";
const char* password = "flume budge prolate crypt gave berkeley judaism comely";

void setup() {
  Serial.begin(9600);
  
  wifi_connection();

  pinMode(GPIO_NUM_32, INPUT_PULLUP);
  pinMode(ROT, OUTPUT);
  pinMode(GRUEN, OUTPUT);

  delay(50);

  SPI.begin();
  cardReader.PCD_Init();
}
 
void loop() {
  int buttonStatus = digitalRead(BUTTON);

  if ( ! cardReader.PICC_IsNewCardPresent() || ! cardReader.PICC_ReadCardSerial()) {
    return;
  }
 
  String newRfidId = "";
  for (byte i = 0; i < cardReader.uid.size; i++) {
    newRfidId.concat(cardReader.uid.uidByte[i] < 0x10 ? " 0" : " ");
    newRfidId.concat(String(cardReader.uid.uidByte[i], HEX));
  }
 
  newRfidId.toUpperCase();
 
  if (!newRfidId.equals(lastRfid)) {
    lastRfid = newRfidId;
    Serial.print(" gelesene RFID-ID :");
    Serial.println(newRfidId);

    int result = memcmp(cardReader.uid.uidByte, chip, 4);
    Serial.println(result);

    Serial.println("Chip:");
    for (byte i = 0; i < 4; i++) {
      Serial.print(chip[i], HEX);
      Serial.print(" ");
    }

    Serial.println("Current Rfid:");
    for (byte i = 0; i < cardReader.uid.size; i++) {
      Serial.print(cardReader.uid.uidByte[i], HEX);
      Serial.print(" ");
    }

    if (result == 0) {
      buttonGruen();

    }
    else {
      buttonRot();
      klingeln();
    }
  }
}


void buttonRot() {
  digitalWrite(GRUEN, LOW);

  digitalWrite(ROT, HIGH);
}

void buttonGruen() {
  digitalWrite(ROT, LOW);

  digitalWrite(GRUEN, HIGH);
}

void klingeln() {
  Serial.println("Klingeling Klingeling");
}
 
void wifi_connection() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

// byte* readRfid() {
//   if ( ! cardReader.PICC_IsNewCardPresent()) {
//     // Serial.println("Keine neue Karte!");
//     return NULL;
//   }

//   if ( ! cardReader.PICC_ReadCardSerial()) {
//     Serial.println("Karte kann nicht gelesen werden!");
//     return NULL;
//   }

//   return cardReader.uid.uidByte;
// }

// String transformRfid(byte* rfidBytes, int length) {
//   String newRfidId = "";
//   for (byte i = 0; i < length; i++) {
//     // !! Achtung es wird ein Leerzeichen vor der ID gesetzt !!
//     newRfidId.concat(rfidBytes[i] < 0x10 ? " 0" : " ");
//     newRfidId.concat(String(rfidBytes[i], HEX));
//   }
 
//   //alle Buchstaben in Großbuchstaben umwandeln
//   newRfidId.toUpperCase();

//   return newRfidId;
// }