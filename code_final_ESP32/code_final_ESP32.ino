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

#define TINY_GSM_RX_BUFFER 2048

const char apn[] = "internet.tn";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[] = "testlilygo-default-rtdb.firebaseio.com";
const char resource[] = "/data.json";
const int port = 443;
const String FIREBASE_AUTH = "BDWr2Ie3r6dBvKFIG1tT8vzlaQgE20fHHw9B33MQ";

// Météo OpenWeatherMap
const char weatherHost[] = "api.openweathermap.org";
const String city = "Tozeur";
const String country = "TN";
const String apiKey = "6603a9954dc7b45216a532ffadccf87a";  // clé API

float temperature, humidite, humiditeSol;
unsigned long lastFirebaseCheck = 0;
const unsigned long FIREBASE_CHECK_INTERVAL = 1000;
bool motopompeState = false;
bool vanne1State = false;
bool vanne2State = false;

TinyGsm modem(Serial1);
TinyGsmClient client(modem, 1);              // HTTP (port 80 pour météo) en utilisant soket 1 pour éviter le conflit avec firebase
TinyGsmClientSecure secureClient(modem, 0);  // HTTPS pour Firebase en utilisant soket 0 pour éviter le conflit avec OWM
HttpClient http_client(secureClient, server, port);

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

void getWeatherData(float &tempExt, float &humExt, float &windSpeed) {
  String url = "/data/2.5/weather?q=" + city + "," + country + "&units=metric&appid=" + apiKey;
  SerialMon.println("URL météo: " + url);

  // Client HTTP standard

  HttpClient weatherClient(client, weatherHost, 80);  // Port 80 pour HTTP
  weatherClient.setTimeout(10000);

  SerialMon.println("Connexion à OpenWeatherMap...");
  if (!weatherClient.connect(weatherHost, 80)) {
    SerialMon.println("Échec connexion HTTP");
    return;
  }

  weatherClient.get(url);

  int statusCode = weatherClient.responseStatusCode();
  String response = weatherClient.responseBody();

  if (statusCode != 200) {
    SerialMon.print("Erreur HTTP: ");
    SerialMon.println(statusCode);
    weatherClient.stop();
    return;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    SerialMon.print("Erreur parsing JSON: ");
    SerialMon.println(error.c_str());
    weatherClient.stop();
    return;
  }

  tempExt = doc["main"]["temp"];
  humExt = doc["main"]["humidity"];
  windSpeed = doc["wind"]["speed"];

  SerialMon.printf("Météo: %.1f°C, %d%% humidité, vent %.1f m/s\n", tempExt, (int)humExt, windSpeed);
  weatherClient.stop();
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
  float extTemp, extHum, extWind;
  getWeatherData(extTemp, extHum, extWind);
  DynamicJsonDocument docPost(1024);
  docPost["temperature"] = temperature;
  docPost["humidite"] = humidite;
  docPost["humiditeSol"] = humiditeSol;
  docPost["temperatureExt"] = extTemp;
  docPost["humiditeExt"] = extHum;
  docPost["vitesseVent"] = extWind*3.6;

  String postData;
  serializeJson(docPost, postData);

  http_client.beginRequest();
  http_client.patch(resource);  // data.json
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


void processFirebaseCommands() {
  if (!modem.isGprsConnected() && !modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println("Échec reconnexion GPRS");
    return;
  }
  if (LoRa.parsePacket() > 0) {
    delay(50);
    return;
  }
  if (!secureClient.connect(server, port)) {
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

    if (newMotopompe != motopompeState) {
      digitalWrite(MOTOPOMPE_PIN, newMotopompe ? LOW : HIGH);
      motopompeState = newMotopompe;
      SerialMon.println("Motopompe: " + String(newMotopompe ? "ON" : "OFF"));
    }

    if (newVanne1 != vanne1State) {
      digitalWrite(Vanne1_PIN, newVanne1 ? HIGH : LOW);
      vanne1State = newVanne1;
      SerialMon.println("Vanne 1: " + String(newVanne1 ? "ON" : "OFF"));
      delay(100);
    }

    if (newVanne2 != vanne2State) {
      digitalWrite(Vanne2_PIN, newVanne2 ? HIGH : LOW);
      vanne2State = newVanne2;
      SerialMon.println("Vanne 2: " + String(newVanne2 ? "ON" : "OFF"));
      delay(100);
    }
  }

  http_client.stop();
}

void loop() {
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

  if (millis() - lastFirebaseCheck > FIREBASE_CHECK_INTERVAL) {
    processFirebaseCommands();
    lastFirebaseCheck = millis();
  }

  delay(10);
}
