#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// Définition des broches LoRa
#define SS D8
#define RST D0
#define DIO0 D1

// Définition des broches pour les capteurs
#define DHTPIN D3
#define SOIL_MOISTURE_SENSOR_PIN A0

/*// Broches des électrovannes
#define vanne1Pin D2
#define vanne2Pin D4
*/
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastSendTime = 0;
unsigned long sendInterval = 8000;

void setup() {

  Serial.begin(9600);
  while (!Serial);

  /*// Initialisation des broches des électrovannes
  pinMode(vanne1Pin, OUTPUT);
  pinMode(vanne2Pin, OUTPUT);
  digitalWrite(vanne1Pin, LOW);
  digitalWrite(vanne2Pin, LOW);
  Serial.println("Broches des vannes initialisées");
*/

  

  // Initialisation LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Erreur de démarrage LoRa !");
    while (1);
  }
  Serial.println("LoRa initialisé avec succès");
  
  // Initialisation capteur DHT
  dht.begin();
}

void loop() {


  unsigned long currentTime = millis();

  // Envoi périodique des données capteurs
  if (currentTime - lastSendTime >= sendInterval) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int soilMoistureValue = analogRead(SOIL_MOISTURE_SENSOR_PIN);
    
    soilMoistureValue = 1023 - soilMoistureValue;
    int soilMoisturePercentage = map(soilMoistureValue, 0, 1023, 0, 100);
    soilMoisturePercentage = constrain(soilMoisturePercentage, 0, 100);

    String data = "T = " + String(t, 1) + "," + "H = " +String(h, 1) + "," + "H.Sol = " + String(soilMoisturePercentage);
    
    Serial.print("Envoi données: ");
    Serial.println(data);
    
    LoRa.beginPacket();
    LoRa.print(data);
    LoRa.endPacket();
    
    lastSendTime = currentTime;
  }

  /*// Réception des commandes LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    
    // Nettoyage du message (retour chariot, espace, etc.)
    message.trim();
    
    Serial.print("Message brut reçu: '");
    Serial.print(message);
    Serial.println("'");

    // Traitement des commandes
    if (message == "VANNE1_ON") {
      digitalWrite(vanne1Pin, HIGH);
      Serial.println("Vanne 1 activée - Niveau broche: " + String(digitalRead(vanne1Pin)));
    } 
    else if (message == "VANNE1_OFF") {
      digitalWrite(vanne1Pin, LOW);
      Serial.println("Vanne 1 désactivée - Niveau broche: " + String(digitalRead(vanne1Pin)));
    }
    else if (message == "VANNE2_ON") {
      digitalWrite(vanne2Pin, HIGH);
      Serial.println("Vanne 2 activée - Niveau broche: " + String(digitalRead(vanne2Pin)));
    }
    else if (message == "VANNE2_OFF") {
      digitalWrite(vanne2Pin, LOW);
      Serial.println("Vanne 2 désactivée - Niveau broche: " + String(digitalRead(vanne2Pin)));
    }
    else if (message == "STATUS") {
      String status = "V1:" + String(digitalRead(vanne1Pin)) + ",V2:" + String(digitalRead(vanne2Pin));
      LoRa.beginPacket();
      LoRa.print(status);
      LoRa.endPacket();
      Serial.println("Statut envoyé: " + status);
    }
    else {
      Serial.println("Commande non reconnue");
    }
  }*/
  
  delay(100);
}