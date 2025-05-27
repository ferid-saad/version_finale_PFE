#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <LoRa.h>

#define SerialMon Serial
#define MODEM_RST 5
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define BLUE_LED 13
#define MOTOPOMPE_PIN 32
#define Vanne1_PIN 25
#define Vanne2_PIN 14
#define SerialAT Serial1


#define SCK 18
#define MISO 19
#define MOSI 21
#define SS 15
#define RST 22
#define DIO0 2

#define TINY_GSM_RX_BUFFER 1024

const char apn[] = "internet.tn";
const char gprsUser[] = "";
const char gprsPass[] = "";


const char server[] = "testlilygo-default-rtdb.firebaseio.com";
const char resource[] = "/data.json";
const int port = 443;
const String FIREBASE_AUTH = "BDWr2Ie3r6dBvKFIG1tT8vzlaQgE20fHHw9B33MQ";

float temperature, humidite, humiditeSol;
unsigned long lastFirebaseCheck = 0;
const unsigned long FIREBASE_CHECK_INTERVAL = 5000;  // 5 secondes mais sa sera 30s en production
bool motopompeState = false;
bool vanne1State = false;
bool vanne2State = false;

TinyGsm modem(Serial1);
TinyGsmClientSecure client(modem, 0);
HttpClient http_client(client, server, port);

DynamicJsonDocument firebaseData(512);
DynamicJsonDocument doc(256);

void setupModem() {
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);

  digitalWrite(MODEM_POWER_ON, HIGH);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(100);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, HIGH);
  digitalWrite(BLUE_LED, LOW);
  pinMode(MOTOPOMPE_PIN, OUTPUT);
  pinMode(Vanne1_PIN, OUTPUT);
  pinMode(Vanne2_PIN, OUTPUT);
}

void setup() {
  SerialMon.begin(115200);
  delay(10);

  setupModem();
  Serial1.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  SerialMon.println("Initializing modem...");
  modem.restart();

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    while (true)
      ;
  }
  SerialMon.println(" success");

  if (!modem.gprsConnect(apn)) {
    SerialMon.println("GPRS connection failed");
    while (true)
      ;
  }
  SerialMon.println("GPRS connected");

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    SerialMon.println("LoRa init failed.");
    while (true)
      ;
  }
  SerialMon.println("LoRa ready");
}


void sendToFirebase(float temperature, float humidite, float humiditeSol) {
  if (!modem.isGprsConnected()) {
    SerialMon.println("Connexion GPRS...");
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println("Échec connexion GPRS");
      return;
    }
  }

  if (!client.connect(server, port)) {
    SerialMon.println("Échec connexion serveur Firebase");
    return;
  }

  DynamicJsonDocument docPost(256);
  docPost["temperature"] = temperature;
  docPost["humidite"] = humidite;
  docPost["humiditeSol"] = humiditeSol;

  String postData;
  serializeJson(docPost, postData);

  http_client.beginRequest();
  http_client.patch(resource);  // /data.json
  http_client.sendHeader("Content-Type", "application/json");
  http_client.sendHeader("Content-Length", postData.length());
  http_client.sendHeader("auth", FIREBASE_AUTH);  // clé d'auth Firebase
  http_client.beginBody();
  http_client.print(postData);
  http_client.endRequest();

  int statusCode = http_client.responseStatusCode();
  SerialMon.print("Code retour Firebase: ");
  SerialMon.println(statusCode);

  if (statusCode == 200) {
    SerialMon.println("Données mises à jour sur Firebase !");
  } else {
    SerialMon.println("Échec mise à jour Firebase");
  }

  http_client.stop();
}

/*void sendLoRaCommand(String command) {
  LoRa.beginPacket();
  LoRa.print(command);
  if (!LoRa.endPacket(100)) { // Timeout 100ms
    SerialMon.println("Erreur envoi LoRa");
  }
}*/

void processFirebaseCommands() {
  if (!modem.isGprsConnected() && !modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println("Échec reconnexion GPRS");
    return;
  }
  if (LoRa.parsePacket() > 0) {  // Si données reçues pendant traitement
    delay(50);                   // Laissez finir la réception
    return;                      // Reporte le traitement
  }
  if (!client.connect(server, port)) {
    SerialMon.println("Échec connexion Firebase");
    return;
  }

  http_client.connectionKeepAlive();
  int err = http_client.get(resource);
  if (err != 0) {
    SerialMon.println("Échec requête GET");
    return;
  }

  int status = http_client.responseStatusCode();
  String response = http_client.responseBody();

  if (status == 200) {
    DynamicJsonDocument doc(256);
    deserializeJson(doc, response);

    bool newMotopompe = doc["motopompe"];
    bool newVanne1 = doc["vanne1"];
    bool newVanne2 = doc["vanne2"];

    // Contrôle des changements d'état
    if (newMotopompe != motopompeState) {
      digitalWrite(MOTOPOMPE_PIN, newMotopompe ? HIGH : LOW);
      motopompeState = newMotopompe;
      SerialMon.println("Motopompe: " + String(newMotopompe ? "ON" : "OFF"));
    }

    if (newVanne1 != vanne1State) {
      digitalWrite(Vanne1_PIN, newVanne1 ?  HIGH : LOW);
      vanne1State = newVanne1;
      Serial.println("Vanne 1 désactivée - Niveau broche: " + String(newVanne1 ?  HIGH : LOW));

      delay(100);  // Pause entre commandes
    }

    if (newVanne2 != vanne2State) {
      digitalWrite(Vanne2_PIN, newVanne2 ?  HIGH : LOW);
      vanne2State = newVanne2;
      Serial.println("Vanne 2 activée - Niveau broche: " +  String(newVanne2 ?  HIGH : LOW));
      delay(100);  // Pause entre commandes
    }
  }

  http_client.stop();
}

void loop() {
  // Partie 1: Réception LoRa et envoi à Firebase
  if (LoRa.parsePacket()) {
    String message;
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    if (sscanf(message.c_str(), "T = %f,H = %f,H.Sol = %f", &temperature, &humidite, &humiditeSol) == 3) {
      SerialMon.printf("Reçu: Temp=%.1f Hum=%.1f Sol=%.1f\n", temperature, humidite, humiditeSol);
      sendToFirebase(temperature, humidite, humiditeSol);
    }
  }

  // Partie 2: Vérification périodique des commandes Firebase
  if (millis() - lastFirebaseCheck > FIREBASE_CHECK_INTERVAL) {
    processFirebaseCommands();
    lastFirebaseCheck = millis();
  }

  delay(10);  // Pause minimale pour stabilité
}