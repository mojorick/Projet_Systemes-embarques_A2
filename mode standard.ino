#include <SD.h>
#include <RTClib.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> // Capteur BME280 pour température, pression et humidité

#define FILE_MAX_SIZE 2048    // Taille maximale du fichier en octets
#define LOG_INTERVAL 1        // Intervalle entre enregistrements (en minutes)
#define TIMEOUT 30000         // Timeout pour les capteurs (en millisecondes)
#define SD_CS_PIN 4           // Broche CS de la carte SD

RTC_DS3231 rtc;
Adafruit_BME280 bme;
File logFile;

unsigned long lastLogTime = 0;
int revisionNumber = 0;
char logFileName[13]; // Format "YYMMDD_X.LOG"

void setup() {
  Serial.begin(9600);

  // Initialisation du module RTC
  if (!rtc.begin()) {
    Serial.println("Erreur: Module RTC non détecté.");
    while (1);
  }

  // Vérification de l'initialisation du RTC
  if (rtc.lostPower()) {
    Serial.println("RTC perdu. Réinitialisation de l'heure !");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Ajuste l'heure selon l'heure de compilation
  }

  // Initialisation du capteur BME280
  if (!bme.begin(0x76)) {
    Serial.println("Erreur: Capteur BME280 non détecté.");
    while (1);
  }

  // Initialisation de la carte SD
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Erreur: Carte SD non détectée.");
    while (1);
  }

  Serial.println("Mode Standard : Prêt à enregistrer les données des capteurs.");
  updateLogFileName();
}

void loop() {
  unsigned long currentMillis = millis();

  // Enregistrer les données à intervalle régulier
  if (currentMillis - lastLogTime >= LOG_INTERVAL * 60000) {
    lastLogTime = currentMillis;
    logSensorData();
  }
}

// Générer un nom de fichier basé sur la date et le numéro de révision
void updateLogFileName() {
  DateTime now = rtc.now();
  snprintf(logFileName, sizeof(logFileName), "%02d%02d%02d_%d.LOG",
           now.year() % 100, now.month(), now.day(), revisionNumber);
}

// Fonction pour enregistrer les données des capteurs
void logSensorData() {
  logFile = SD.open(logFileName, FILE_WRITE);

  if (!logFile) {
    Serial.println("Erreur : Impossible d'ouvrir le fichier pour l'écriture.");
    return;
  }

  // Vérifier la taille du fichier et gérer les révisions si nécessaire
  if (logFile.size() >= FILE_MAX_SIZE) {
    logFile.close();
    revisionNumber++;
    updateLogFileName();
    logFile = SD.open(logFileName, FILE_WRITE);
  }

  // Enregistrement des données des capteurs avec horodatage
  DateTime now = rtc.now();
  logFile.print(now.year());
  logFile.print('/');
  logFile.print(now.month());
  logFile.print('/');
  logFile.print(now.day());
  logFile.print(" ");
  logFile.print(now.hour());
  logFile.print(':');
  logFile.print(now.minute());
  logFile.print(':');
  logFile.print(now.second());
  logFile.print(",");

  logFile.print("Température: ");
  logSensorValue([&]() { return bme.readTemperature(); }, "NA");

  logFile.print(", Humidité: ");
  logSensorValue([&]() { return bme.readHumidity(); }, "NA");

  logFile.print(", Pression: ");
  logSensorValue([&]() { return bme.readPressure() / 100.0F; }, "NA");

  logFile.println();
  logFile.close();

  Serial.println("Données enregistrées sur la carte SD.");
}

// Fonction pour lire une valeur de capteur avec un timeout
template <typename Func>
void logSensorValue(Func getValue, const char *errorValue) {
  unsigned long startTime = millis();
  while (true) {
    float value = getValue();
    if (!isnan(value)) {
      logFile.print(value);
      return;
    } else if (millis() - startTime >= TIMEOUT) {
      logFile.print(errorValue);
      return;
    }
    delay(100); // Attendre avant de réessayer
  }
}
