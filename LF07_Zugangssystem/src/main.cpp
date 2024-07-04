#include <tr064.h>
#include <string.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <driver/ledc.h> // Hinzufügen der LEDC-Bibliothek
#include <list>
#include <NTPClient.h>

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

struct key {
  String name;
  byte rfid[10];
};

list <key> keyList;
list <String> logList;

String currentkeypadString = "";
String correctPassword = "1234A";

WiFiServer server(80);
String header;

uint8_t rowPins[ROWS] = {16, 17, 5, 2};
uint8_t colPins[COLS] = {15, 13, 14, 12};

void ledRedPulse();
void ledGreenPulse();
void ledBlueOff();
void ledBlueOn();
void ledBluePulseShort();

void wifi_connection();
void webserverLoop();
void webserverTask(void *parameter);
void otherTask(void *parameter);
void otherTaskLoop();

void removeKey(byte* rfid);
void enterAddModus(String name);
void enterRemoveModus();
void keypadEvent(KeypadEvent key);
void alarm();
void klingeln();
void shortPositiveTone();
void successTonesLowHigh();

void printControlPage(WiFiClient client);
void printKeys(WiFiClient client);
void printLogList(WiFiClient client);


const char keyMap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

MFRC522 cardReader(SDA, RST);
Keypad keypad = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);

// const char* ssid     = "FRITZ!Box 7490";
// const char* password = "80073437732487114646";

const char  FRITZBOX_IP[]         = "192.168.188.1";
const int   FRITZBOX_PORT         = 49000;
const char  FRITZBOXUSER[]        = "FBuser";
const char  FRITZBOXPASSWORD[]    = "FBpasswort";

// TR064 tr064_connection(FRITZBOX_PORT, FRITZBOX_IP, FRITZBOXUSER, FRITZBOXPASSWORD);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

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

  // Initialisiere Tasks auf den jeweiligen Kernen
  xTaskCreatePinnedToCore(webserverTask, "WebServerTask", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(otherTask, "OtherTask", 10000, NULL, 1, NULL, 1);
}

void loop() {
}


void otherTaskLoop() {

  keypad.getKey();

  int buttonStatus = digitalRead(BUTTON);

  if ( ! cardReader.PICC_IsNewCardPresent() || ! cardReader.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("Current Rfid:");
  for (byte i = 0; i < cardReader.uid.size; i++) {
    Serial.print(cardReader.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  for (list<key>::iterator it = keyList.begin(); it != keyList.end(); it++) {
    bool isEqual = true;
    for (byte i = 0; i < cardReader.uid.size; i++) {
      if (it->rfid[i] != cardReader.uid.uidByte[i]) {
        isEqual = false;
        break;
      }
    }
    if (isEqual) {
      successTonesLowHigh();
      Serial.println("Rfid gefunden");
      ledGreenPulse();
      String currentTime = timeClient.getFormattedTime();
      String newRfidId = "";
      for (byte i = 0; i < cardReader.uid.size; i++) {
        newRfidId.concat(cardReader.uid.uidByte[i] < 0x10 ? " 0" : " ");
        newRfidId.concat(String(cardReader.uid.uidByte[i], HEX));
      }
      newRfidId.toUpperCase();
      String logEntry = "[" + currentTime + "] - " + it->name + " - " + newRfidId;
      logList.push_back(logEntry);
      return;
    }
  }
  Serial.println("Unbekannter Schlüssel");
  String currentTime = timeClient.getFormattedTime();
  String newRfidId = "";
      for (byte i = 0; i < cardReader.uid.size; i++) {
        newRfidId.concat(cardReader.uid.uidByte[i] < 0x10 ? " 0" : " ");
        newRfidId.concat(String(cardReader.uid.uidByte[i], HEX));
      }
  newRfidId.toUpperCase();
  String logEntry = "[" + currentTime + "] - Unbekannter Schlüssel - " + newRfidId;
  logList.push_back(logEntry);
  alarm();
  klingeln();
  ledRedPulse();

  delay(1000);
}

void webserverTask(void *parameter) {
  // print private IP address and start web server
  timeClient.update();
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  while (true) {
    webserverLoop();
    vTaskDelay(10);
  }
}

void otherTask(void *parameter) {
  while (true) {
    otherTaskLoop();
    vTaskDelay(10);
  }
}

void ledRedPulse() {
  digitalWrite(ROT, HIGH);
  delay(1000);
  digitalWrite(ROT, LOW);
}

void ledGreenPulse() {
  digitalWrite(GRUEN, HIGH);
  delay(1000);
  digitalWrite(GRUEN, LOW);
}

void ledBluePulseShort() {
  digitalWrite(BLAU, HIGH);
  delay(200);
  digitalWrite(BLAU, LOW);
}

void ledBlueOn() {
  digitalWrite(BLAU, HIGH);
}

void ledBlueOff() {
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

void webserverLoop() { 
   WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    if (client.connected() && client.available()) {             // if there's bytes to read from the client,
      char c1, c2;
      while (true) {          
        c2 = c1;
        c1 = client.read();                 // lese zeichenweise die Daten der Browser-Anfrage ein
        Serial.write(c1);
        header += c1;

        if(c1=='\r' && c2=='\n') {break;}   // Wenn Ende der Daten erreicht, breche while-Schleife ab
      }
      
      if (header.indexOf("GET / ") != -1 || header.indexOf("GET /index") != -1) {
        printControlPage(client);
      }
      else if (header.indexOf("GET /add?key=") != -1) {
        int start = header.indexOf("GET /add?key=") + 13;
        int end = header.indexOf("HTTP/1.1") - 1;
        String name = header.substring(start, end);;
        enterAddModus(name);
      }
      else if (header.indexOf("GET /removeMode") != -1) {
        enterRemoveModus();
      }
      else if (header.indexOf("GET /newPassword?password=") != -1) {
        int start = header.indexOf("GET /newPassword?password=") + 26;
        int end = header.indexOf("HTTP/1.1") - 1;
        correctPassword = header.substring(start, end);
        successTonesLowHigh();
        Serial.println("Neues Passwort: " + correctPassword);
        ledGreenPulse();
      }
      else {
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-type:text/html");
        client.println();
        client.println("<h1>404 Not Found</h1>");
      }
    }

    header = "";            // Zurücksetzen der Variablen
    client.stop();          // Beende Verbindung
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void wifi_connection() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  timeClient.setTimeOffset(7200);
  timeClient.begin();
}

void keypadEvent(KeypadEvent key) {
  if (keypad.getState() == RELEASED) {
    currentkeypadString += key;
    ledBluePulseShort();
    shortPositiveTone();

    Serial.println(currentkeypadString);
    if (currentkeypadString.length() == 5) {
      if (currentkeypadString == correctPassword) {
        Serial.println("Passwort korrekt");
        successTonesLowHigh();
        ledGreenPulse();
      }
      else {
        Serial.println("Passwort falsch");
        String currentTime = timeClient.getFormattedTime();
        currentkeypadString.toUpperCase();
        String logEntry = "[" + currentTime + "] - Falsches Passwort - " + currentkeypadString;
        logList.push_back(logEntry);
        ledRedPulse();
        alarm();
      }
      currentkeypadString = "";
    }
  }
}

void removeKey(byte* rfid) {
  for (list<key>::iterator it = keyList.begin(); it != keyList.end(); it++) {
    if (it->rfid == rfid) {
      keyList.erase(it);
      break;
    }
  }
}

void enterAddModus(String name) {
  shortPositiveTone();
  shortPositiveTone();
  Serial.println("Enter Add Modus");
  ledBlueOn();

  while (true) {
    if (!cardReader.PICC_IsNewCardPresent() || !cardReader.PICC_ReadCardSerial()) {
      continue;
    }
    key newKey;
    newKey.name = name;
    memcpy(newKey.rfid, cardReader.uid.uidByte, cardReader.uid.size);
    memset(newKey.rfid + cardReader.uid.size, 0, 10 - cardReader.uid.size);
    keyList.push_back(newKey);
    ledBlueOff();
    ledBluePulseShort();
    ledGreenPulse();
    successTonesLowHigh();
    break;
  }
}

void enterRemoveModus() {
  Serial.println("Enter Remove Modus");
  ledBlueOn();

  while (true) {
    if (!cardReader.PICC_IsNewCardPresent() || !cardReader.PICC_ReadCardSerial()) {
      continue;
    }
    removeKey(cardReader.uid.uidByte);
    ledBlueOff();
    ledBluePulseShort();
    ledRedPulse();
    break;
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
  client.println("<section>");
  client.println("<h3>Protokoll</h3>");
  printLogList(client);
  client.println("</section>");
  client.println("<section>");
  client.println("<h3>Verfügbare Schlüssel</h3>");
  printKeys(client);
  client.println("</section>");
  client.println("<section>");
  client.println("<h3>Neues Keypad-Passwort</h3>");
  client.println("<div class=\"input-container\">");
  client.println("<input type=\"text\" id=\"newPassword\" placeholder=\"Passwort eingeben\">");
  client.println("<button onclick=\"changePassword()\">Ändern</button>");
  client.println("</div>");
  client.println("</section>");
  client.println("</section>");
  client.println("</div>");
  client.println("<script>");
  client.println("function addName() {");
  client.println("let nameInput = document.getElementById('nameInput').value.trim();");
  client.println("if (nameInput !== '') {");
  client.println("fetch('http://' + window.location.hostname + ':80/add?key=' + nameInput)");
  client.println(".then(response => response.json());");
  client.println("}");
  client.println("}");
  client.println("function toggleRemoveMode() {");
  client.println("fetch('http://' + window.location.hostname + ':80/removeMode')");
  client.println(".then(response => response.json());");
  client.println("}");
  client.println("function changePassword() {");
  client.println("let newPassword = document.getElementById('newPassword').value.trim();");
  client.println("if (newPassword !== '') {");
  client.println("fetch('http://' + window.location.hostname + ':80/newPassword?password=' + newPassword)");
  client.println(".then(response => response.json());");
  client.println("}");
  client.println("}");
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");
}

void printKeys(WiFiClient client) {
  client.println("<div class=\"keys\" id=\"keys\">");
  for (list<key>::iterator it = keyList.begin(); it != keyList.end(); it++) {
    client.print("<div>");
    client.print(it->name);
    client.print(" - ");
    for (byte i = 0; i < 10; i++) {
      client.print(it->rfid[i], HEX);
      client.print(" ");
    }
    client.println("</div>");
  }
  client.println("</div>");
}

void printLogList(WiFiClient client) {
  client.println("<div class=\"log\" id=\"log\">");
  for (list<String>::iterator it = logList.begin(); it != logList.end(); it++) {
    client.print("<div>");
    client.print(*it);
    client.println("</div>");
  }
  client.println("</div>");
}

void shortPositiveTone() {
  ledcWriteTone(0, 1500);
  delay(100);
  ledcWriteTone(0, 0);
}

void successTonesLowHigh() {
  ledcWriteTone(0, 1000);
  delay(100);
  ledcWriteTone(0, 2000);
  delay(100);
  ledcWriteTone(0, 0);
}