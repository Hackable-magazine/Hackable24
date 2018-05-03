#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <SPI.h>
#include <RCSwitch.h>

#include "si4021.h"

/* Memo SPI Wemos D1 Mini :
MOSI -> D7
SCK  -> D5
SEL/CS -> D0
FSK -> D4
*/

#define CS D0
#define FSK D4

RCSwitch mySwitch = RCSwitch();

struct cfg {
  float freq;
  unsigned int attenuation;
} maconfig = { 433.92, PS_0DB };

// SSID du point d'accès
const char* ssid = "monSSID";
// mot de passe wifi
const char* password = "---------------";

ESP8266WebServer server(80);

static const char *INDEX_HTML = 
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"<meta charset=\"utf-8\">\n"
"<title>ESP8266 Contrôle éclairage</title>\n"
"<style>\n"
//".hide { width: 0; height: 0; border: 0; border: none; position: absolute; visibility:0 }\n"
"body { font-family: \"helvetica neue\", helvetica, arial, sans-serif; background: #eee; }\n"
".notif { width: 250px; height: 50px; border: 0; border: none; position: absolute; margin-top: 10px; }\n"
".punch {\n"
"  outline: none;\n"
"  background: #4162a8;\n"
"  border-top: 1px solid #38538c;\n"
"  border-right: 1px solid #1f2d4d;\n"
"  border-bottom: 1px solid #151e33;\n"
"  border-left: 1px solid #1f2d4d;\n"
"  border-radius: 4px;\n"
"  -webkit-box-shadow: inset 0 1px 10px 1px #5c8bee, 0 1px 0 #1d2c4d, 0 6px 0 #1f3053, 0 8px 4px 1px #111111;\n"
"  box-shadow: inset 0 1px 10px 1px #5c8bee, 0 1px 0 #1d2c4d, 0 6px 0 #1f3053, 0 8px 4px 1px #111111;\n"
"  color: #fff;\n"
"  font: bold 20px/1 \"helvetica neue\", helvetica, arial, sans-serif;\n"
"  margin-top: 10px;\n"
"  margin-bottom: 10px;\n"
"  padding: 10px 0 12px 0;\n"
"  text-align: center;\n"
"  text-shadow: 0 -1px 1px #1e2d4d;\n"
"  width: 150px;\n"
"  -webkit-background-clip: padding-box; }\n"
".punch:hover {\n"
"  -webkit-box-shadow: inset 0 0 20px 1px #87adff, 0 1px 0 #1d2c4d, 0 6px 0 #1f3053, 0 8px 4px 1px #111111;\n"
"  box-shadow: inset 0 0 20px 1px #87adff, 0 1px 0 #1d2c4d, 0 6px 0 #1f3053, 0 8px 4px 1px #111111;\n"
"  cursor: pointer; }\n"
".punch:active {\n"
"  -webkit-box-shadow: inset 0 1px 10px 1px #5c8bee, 0 1px 0 #1d2c4d, 0 2px 0 #1f3053, 0 4px 3px 0 #111111;\n"
"  box-shadow: inset 0 1px 10px 1px #5c8bee, 0 1px 0 #1d2c4d, 0 2px 0 #1f3053, 0 4px 3px 0 #111111; }\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<h1>Contrôle éclairage</h1>\n"
"<form action=\"/confort\" method=\"get\" target=\"hiddenFrame\">\n"
"  <input value=\"Mode confort\" type=\"submit\" class=\"punch\">\n"
"</form>\n"
"<form action=\"/travail\" method=\"get\" target=\"hiddenFrame\">\n"
"  <input value=\"Mode travail\" type=\"submit\" class=\"punch\">\n"
"</form>\n"
"<iframe name=\"hiddenFrame\" class=\"notif\"></iframe>\n"
"</body>\n"
"</html>\n"
;

// racine du site
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
  Serial.print("web: accès page racine\n");
}

// page /confort
void handleConfort() {
  server.send(200, "text/plain", "mode confort activé");
  Serial.print("web: mode confort\n");
  mySwitch.send("001011111010000011011100");
  //Switch.send("001011111010000011011100");
}

// page /travail
void handleTravail() {
  server.send(200, "text/plain", "mode travail activé");
  Serial.print("web: mode travail\n");
  mySwitch.send("001011111010000011010100");
  //mySwitch.send("001011111010000011010100");
}

// page 404
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println("web: erreur 404");
}

void setup() {
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  uint16_t cbits;
  uint16_t fbits;
  uint16_t pwmanbits;
  uint8_t  pwbits;


  /***** Configuration générale *****/
  Serial.begin(115200);

  // configuration ports en sortie
  pinMode(CS, OUTPUT);
  pinMode(FSK, OUTPUT);
  // CS à l'état haut
  // le module n'est PAS "sélectionné" et
  // ne traite donc PAS les éventuelles données SPI
  digitalWrite(CS, HIGH);  

  // Affichage adresse MAC
  Serial.print("info: MAC = ");
  if(WiFi.macAddress(mac) != 0) {
    for(int i=0; i<WL_MAC_ADDR_LENGTH; i++) {
      if(mac[i]<16) Serial.print ("0");
      Serial.print(mac[i],HEX);
      Serial.print((i < WL_MAC_ADDR_LENGTH-1) ? ":" : "\n\r");
    }
  }

  // Mode client Wifi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  /***** Configuration SPI et si4021 *****/
  // Configuration SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  delay(100);

  // configuration de la fréquence sur 433.92 MHz
  Serial.println("si4021: Configuration Setting");
  if(!(cbits = confset(maconfig.freq))) {
      Serial.println("ERROR : bad config!");
      while(1);
  } else {
    sendToSi4021(CS, cbits);
  }
  Serial.println("si4021: Frequency Setting");
  if(!(fbits = freqset(maconfig.freq))) {
    Serial.println("ERROR : bad frequency!");
  } else {
    sendToSi4021(CS, fbits);
  }

  // Configuration de l'emission
  Serial.println("si4021: Power Setting");
  if(!(pwbits = powerset(maconfig.attenuation))) {
    Serial.println("ERROR : bad power settings!");
  } else {
    sendToSi4021(CS, pwbits);
  }

  // Configuration de l'alimentation
  Serial.println("si4021: Power Management");
  if(!(pwmanbits = powerman())) {
    Serial.println("ERROR : bad power management settings!");
  } else {
    sendToSi4021(CS, pwmanbits);
  }

  /***** Configuration RCSwitch OOK *****/
  // On utilise la sortie FSK (broche 8)
  mySwitch.enableTransmit(FSK);
  // Option : durée de l'impulsion
  // mySwitch.setPulseLength(125);
  // Option : protocole (1 fonctionne la plupart du temps)
  mySwitch.setProtocol(1);
  // Option : nombre de répétition du message
  mySwitch.setRepeatTransmit(5);

  /***** Wifi et serveur HTTP *****/
  Serial.println("net: Connexion AP...");
  WiFi.begin(ssid, password);

  // Connexion Wifi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    // impossible de se connecter au point d'accès
    Serial.println("info: Erreur connexion Wifi !");
    delay(5000);
  }
  Serial.println("net: WiFi connecté");
  Serial.print("net: mon IP = ");
  Serial.println(WiFi.localIP());

  // démarrage mDNS-SD
  if (!MDNS.begin("esp8266si4021")) {
    Serial.println("info: Erreur configuration mDNS!");
  } else {
    Serial.println("net: répondeur mDNS démarré");
    // Ajout service disponible ici
    MDNS.addService("http", "tcp", 80); // Announce esp tcp service on port 23
  }

  // configuration serveur Web
  server.on("/", handleRoot);
  server.on("/confort", handleConfort);
  server.on("/travail", handleTravail);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("web: serveur HTTP démarré");

  Serial.println("info: configuration terminée");
  Serial.println("info: attente des connexions...");
  delay(100);
}

// boucle principale
void loop() {
  server.handleClient();
}
