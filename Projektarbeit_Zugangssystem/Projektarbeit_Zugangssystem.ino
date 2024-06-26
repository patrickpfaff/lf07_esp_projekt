#include <tr064.h>
#include <string.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#define BUTTON 32
#define SCK 18
#define SDA 21
#define MOSI 23
#define MISO 19
#define RST 0
 
#define GRUEN 27
#define ROT 26

String chip = "36 12 6F 90";
String karte = "8C 3F 5F DB";

byte test = 0;


const char* ssid     = "FRITZ!Box 7490";
const char* password = "80073437732487114646";

const char  FRITZBOX_IP[]         = "192.168.188.1";
const int   FRITZBOX_PORT         = 49000;
const char  FRITZBOXUSER[]        = "FBuser";
const char  FRITZBOXPASSWORD[]    = "FBpasswort";

MFRC522 cardReader(SDA, RST);
TR064 tr064_connection(FRITZBOX_PORT, FRITZBOX_IP, FRITZBOXUSER, FRITZBOXPASSWORD);

String lastRfid = ""; 
 
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
  // Serial.println(buttonStatus);

  if ( ! cardReader.PICC_IsNewCardPresent() || ! cardReader.PICC_ReadCardSerial()) {
    return;
  }
 
  String newRfidId = "";
  for (byte i = 0; i < cardReader.uid.size; i++) {
    // !! Achtung es wird ein Leerzeichen vor der ID gesetzt !!
    newRfidId.concat(cardReader.uid.uidByte[i] < 0x10 ? " 0" : " ");
    newRfidId.concat(String(cardReader.uid.uidByte[i], HEX));

    test = cardReader.uid.uidByte[i];
  }
 
  //alle Buchstaben in Großbuchstaben umwandeln
  newRfidId.toUpperCase();
 
  //Wenn die neue gelesene RFID-ID ungleich der bereits zuvor gelesenen ist,
  //dann soll diese auf der seriellen Schnittstelle ausgegeben werden.
  if (!newRfidId.equals(lastRfid)) {
    //überschreiben der alten ID mit der neuen
    lastRfid = newRfidId;
    Serial.print(" gelesene RFID-ID :");
    Serial.println(newRfidId);
    Serial.println();
    Serial.println(test);
    if (test == 0xdb) {
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
  tr064_connection.init();
  String call_params[][2] = {{"NewX_AVM-DE_PhoneNumber", "**9"}};                                          // Die Telefonnummer **9 ist der Fritzbox-Rundruf.
  tr064_connection.action("urn:dslforum-org:service:X_VoIP:1", "X_AVM-DE_DialNumber", call_params, 1);    // Action: Rundruf
  delay(2000);                                                                                            // Warte 2 Sekunden bis zum Auflegen
  tr064_connection.action("urn:dslforum-org:service:X_VoIP:1", "X_AVM-DE_DialHangup");
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