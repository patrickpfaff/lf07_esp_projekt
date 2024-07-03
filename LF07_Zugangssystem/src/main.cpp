#include <tr064.h>
#include <string.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <driver/ledc.h> // Hinzufügen der LEDC-Bibliothek

using namespace std;

#define BUTTON 32
#define BUZZER 25

#define SCK 18
#define SDA 21
#define MOSI 23
#define MISO 19
#define RST 0
 
#define GRUEN 27
#define ROT 26
#define BLAU 22

#define ROWS 4
#define COLS 4

String currentkeypadString = "";
String correctPassword = "1234A";

uint8_t rowPins[ROWS] = {16, 17, 5, 2};
uint8_t colPins[COLS] = {15, 13, 14, 12};

void wifi_connection();
void buttonRot();
void buttonGruen();
void klingeln();
void alarm();
void keypadEvent(KeypadEvent key);
void printControlPage(WiFiClient client);

const char keyMap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

MFRC522 cardReader(SDA, RST);
Keypad keypad = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);
// TR064 tr064_connection(FRITZBOX_PORT, FRITZBOX_IP, FRITZBOXUSER, FRITZBOXPASSWORD);

byte karte[] = {0xA3, 0xBA, 0x8F, 0x94};
byte chip[] = {0x23, 0x22, 0xD7, 0x91};

const char* ssid     = "anouri SNS";
const char* password = "flume budge prolate crypt gave berkeley judaism comely";

void setup() {
  Serial.begin(9600);
  
  wifi_connection();

  pinMode(GPIO_NUM_32, INPUT_PULLUP);
  pinMode(ROT, OUTPUT);
  pinMode(GRUEN, OUTPUT);
  pinMode(BLAU, OUTPUT);
  pinMode(BUZZER, OUTPUT);

// Initialisiere das LEDC-Modul
  ledcSetup(0, 5000, 8);  // Kanal 0, 5 kHz, 8-bit Auflösung
  ledcAttachPin(BUZZER, 0);
  delay(50);

  SPI.begin();
  cardReader.PCD_Init();

  keypad.setDebounceTime(10);
  keypad.addEventListener(keypadEvent);
}
 
void loop() {

  keypad.getKey();

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
    alarm();
    buttonRot();
    klingeln();
  }

  delay(1000);
}


void buttonRot() {
  digitalWrite(ROT, HIGH);
  delay(1000);
  digitalWrite(ROT, LOW);
}

void buttonGruen() {
  digitalWrite(GRUEN, HIGH);
  delay(1000);
  digitalWrite(GRUEN, LOW);
}

void buttonBlauKurz() {
  digitalWrite(BLAU, HIGH);
  delay(200);
  digitalWrite(BLAU, LOW);
}

void klingeln() {
  Serial.println("Klingeling Klingeling");
  // tr064_connection.init();
  // String call_params[][2] = {{"NewX_AVM-DE_PhoneNumber", "**9"}};                                          // Die Telefonnummer **9 ist der Fritzbox-Rundruf.
  // tr064_connection.action("urn:dslforum-org:service:X_VoIP:1", "X_AVM-DE_DialNumber", call_params, 1);    // Action: Rundruf
  // delay(2000);                                                                                            // Warte 2 Sekunden bis zum Auflegen
  // tr064_connection.action("urn:dslforum-org:service:X_VoIP:1", "X_AVM-DE_DialHangup");
}

void alarm() {
  Serial.println("Alarm Alarm");
  // Ein Alarm Ton wird abgespielt mithilfe der tone - Funktion
  tone(BUZZER, 500, 250);
  delay(250);
  tone(BUZZER, 500, 250);
  delay(250);
  tone(BUZZER, 500, 250);
}

void wifi_connection() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void keypadEvent(KeypadEvent key) {
  if (keypad.getState() == RELEASED) {
    currentkeypadString += key;
    buttonBlauKurz();

    Serial.println(currentkeypadString);
    if (currentkeypadString.length() == 5) {
      if (currentkeypadString == correctPassword) {
        Serial.println("Passwort korrekt");
        buttonGruen();
      }
      else {
        Serial.println("Passwort falsch");
        buttonRot();
        alarm();
      }
      currentkeypadString = "";
    }
  }
}

void printControlPage(WiFiClient client) {
  client.println("<!DOCTYPE html>");
  client.println("<html lang=\"de\">");
  client.println("<head>");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<title>Zugangskontrollsystem</title>");
  client.println("<style>");
  client.println("body {");
  client.println("font-family: Arial, sans-serif;");
  client.println("margin: 20px;");
  client.println("background-color: #f0f0f0;");
  client.println("}");
  client.println(".container {");
  client.println("max-width: 800px;");
  client.println("margin: auto;");
  client.println("background-color: #fff;");
  client.println("padding: 20px;");
  client.println("border-radius: 8px;");
  client.println("box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);");
  client.println("}");
  client.println("header {");
  client.println("text-align: center;");
  client.println("margin-bottom: 20px;");
  client.println("}");
  client.println("header h1 {");
  client.println("color: #007bff;");
  client.println("margin: 0;");
  client.println("font-size: 2em;");
  client.println("}");
  client.println("section {");
  client.println("margin-bottom: 20px;");
  client.println("}");
  client.println("section h2 {");
  client.println("color: #007bff;");
  client.println("margin-top: 0;");
  client.println("font-size: 1.5em;");
  client.println("}");
  client.println(".input-container {");
  client.println("display: flex;");
  client.println("align-items: center;");
  client.println("margin-bottom: 10px;");
  client.println("}");
  client.println(".input-container input {");
  client.println("padding: 10px;");
  client.println("margin-right: 10px;");
  client.println("flex: 1;");
  client.println("border: 1px solid #ccc;");
  client.println("border-radius: 4px;");
  client.println("font-size: 1em;");
  client.println("}");
  client.println(".input-container button {");
  client.println("padding: 10px 20px;");
  client.println("background-color: #007bff;");
  client.println("color: white;");
  client.println("border: none;");
  client.println("border-radius: 4px;");
  client.println("cursor: pointer;");
  client.println("font-size: 1em;");
  client.println("}");
  client.println(".btn-remove {");
  client.println("background-color: #dc3545;");
  client.println("padding: 10px 20px;");
  client.println("color: white;");
  client.println("border: none;");
  client.println("border-radius: 4px;");
  client.println("cursor: pointer;");
  client.println("font-size: 1em;");
  client.println("}");
  client.println(".log, .keys {");
  client.println("border: 1px solid #ccc;");
  client.println("padding: 10px;");
  client.println("border-radius: 4px;");
  client.println("max-height: 200px;");
  client.println("overflow-y: auto;");
  client.println("background-color: #f8f9fa;");
  client.println("}");
  client.println(".log {");
  client.println("margin-top: 10px;");
  client.println("min-height: 80px;");
  client.println("}");
  client.println(".keys {");
  client.println("min-height: 40px;");
  client.println("margin-top: 10px;");
  client.println("}");
  client.println(".log div, .keys div {");
  client.println("margin-bottom: 5px;");
  client.println("padding: 5px 10px;");
  client.println("background-color: #fff;");
  client.println("border-radius: 4px;");
  client.println("word-wrap: break-word;");
  client.println("}");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<header>");
  client.println("<h1>Zugangskontrollsystem</h1>");
  client.println("</header>");
  client.println("<div class=\"container\">");
  client.println("<section>");
  client.println("<h2>Neuen Schlüssel hinzufügen</h2>");
  client.println("<div class=\"input-container\">");
  client.println("<input type=\"text\" id=\"nameInput\" placeholder=\"Name eingeben\">");
  client.println("<button onclick=\"addName()\">Hinzufügen</button>");
  client.println("</div>");
  client.println("</section>");
  client.println("<section>");
  client.println("<h2>Schlüsselverwaltung</h2>");
  client.println("<button onclick=\"toggleRemoveMode()\" class=\"btn-remove\">Entfernmodus</button>");
  client.println("<div class=\"log\" id=\"log\">");
  client.println("<!-- Hier wird das Protokoll angezeigt -->");
  client.println("</div>");
  client.println("<div class=\"keys\" id=\"keys\">");
  client.println("<!-- Hier werden die aktuellen Schlüssel angezeigt -->");
  client.println("</div>");
  client.println("</section>");
  client.println("</div>");
  client.println("<script>");
  client.println("// JavaScript für die Funktionen");
  client.println("function addName() {");
  client.println("let nameInput = document.getElementById('nameInput').value.trim();");
  client.println("if (nameInput !== '') {");
  client.println("// get request to the server");
  client.println("fetch('http://localhost:8080/add?key=' + nameInput)");
  client.println(".then(response => response.json());");
  client.println("}");
  client.println("}");
  client.println("function toggleRemoveMode() {");
  client.println("// get request to the server");
  client.println("fetch('http://localhost:8080/removeMode')");
  client.println(".then(response => response.json());");
  client.println("}");
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");
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