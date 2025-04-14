#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> // Capteur BME280
#include <RTClib.h>          // Horloge RTC
#include <SD.h>              // Carte SD
#include <SoftwareSerial.h>  // Pour le GPS

RTC_DS3231 rtc;
Adafruit_BME280 bme;
SoftwareSerial gpsSerial(3, 4); // Pins RX et TX pour le GPS
const int chipSelect = 4; // Pin pour la carte SD

// Variables pour le mode économique
bool economicMode = true;
unsigned long lastLogTime = 0;
unsigned long LOG_INTERVAL = 60000; // Intervalle entre deux mesures (par défaut 60 secondes)
bool skipGPS = false; // Variable pour sauter une mesure GPS sur deux

void setup() {
  Serial.begin(9600);
  
  if (!rtc.begin()) {
    Serial.println("Erreur d'accès à l'horloge RTC");
    while (1); // Bloquer si le RTC ne s'initialise pas
  }

  if (!bme.begin(0x76)) {
    Serial.println("Erreur d'accès au capteur BME280");
    while (1); // Bloquer si le capteur ne s'initialise pas
  }

  if (!SD.begin(chipSelect)) {
    Serial.println("Erreur d'accès à la carte SD");
    while (1); // Bloquer si la carte SD ne s'initialise pas
  }

  gpsSerial.begin(9600); // Initialisation de la communication série avec le GPS
}

void loop() {
  if (economicMode) {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastLogTime >= LOG_INTERVAL * 2) { // Multiplie l'intervalle par 2 en mode économique
      lastLogTime = currentMillis;

      // Lire les capteurs
      float temperature = bme.readTemperature();
      float humidity = bme.readHumidity();
      float pressure = bme.readPressure() / 100.0F; // Convertir la pression en hPa

      // Lecture conditionnelle du GPS (une fois sur deux)
      String gpsData = "";
      if (!skipGPS) {
        gpsData = readGPS();
      }
      skipGPS = !skipGPS; // Inverse la variable pour sauter une mesure sur deux

      // Affichage des données dans le moniteur série
      Serial.print("Température: "); Serial.println(temperature);
      Serial.print("Humidité: "); Serial.println(humidity);
      Serial.print("Pression: "); Serial.println(pressure);
      if (!skipGPS) {
        Serial.print("GPS: "); Serial.println(gpsData);
      } else {
        Serial.println("GPS: Ignoré cette fois");
      }

      // Écriture des données sur la carte SD
      logData(temperature, humidity, pressure, gpsData);
    }
  }
}

// Fonction pour lire les données GPS
String readGPS() {
  String gpsData = "";
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    gpsData += c;
  }
  return gpsData;
}

// Fonction pour écrire les données dans un fichier sur la carte SD
void logData(float temperature, float humidity, float pressure, String gpsData) {
  DateTime now = rtc.now();
  File logFile = SD.open("log.txt", FILE_WRITE);
  
  if (logFile) {
    logFile.print(now.year(), DEC);
    logFile.print('/');
    logFile.print(now.month(), DEC);
    logFile.print('/');
    logFile.print(now.day(), DEC);
    logFile.print(' ');
    logFile.print(now.hour(), DEC);
    logFile.print(':');
    logFile.print(now.minute(), DEC);
    logFile.print(':');
    logFile.print(now.second(), DEC);
    logFile.print(" - ");
    logFile.print("Temp: ");
    logFile.print(temperature);
    logFile.print(" C, ");
    logFile.print("Humidité: ");
    logFile.print(humidity);
    logFile.print(" %, ");
    logFile.print("Pression: ");
    logFile.print(pressure);
    logFile.print(" hPa, ");
    logFile.print("GPS: ");
    logFile.println(gpsData);
    logFile.close(); // Fermer le fichier après l'écriture
  } else {
    Serial.println("Erreur d'ouverture du fichier log.txt");
  }
}
