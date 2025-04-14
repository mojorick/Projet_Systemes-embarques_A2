// Include libraries and define constants...
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <ChainableLED.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>  // Utilisation de la bibliothèque TinyGPS++

#define DATA_PIN 4
#define CLOCK_PIN 5
#define NUM_LEDS 1

#define button2 3 // Bouton rouge (pour le mode économique)
#define button3 2 // Bouton vert (pour le mode maintenance et configuration)

RTC_DS3231 rtc;
const int chipSelect = 4;
Adafruit_BME280 bme;
File logFile;
ChainableLED leds(DATA_PIN, CLOCK_PIN, NUM_LEDS);
TinyGPSPlus gps;                // Initialisation de l'objet TinyGPS++
SoftwareSerial gpsSerial(7, 8);  // RX (7), TX (8) pour le GPS

const int luminosityPin = A0; // Broche analogique pour le capteur de luminosité

enum etatsysteme { STANDARD, CONFIGURATION, MAINTENANCE, ECONOMIQUE };
etatsysteme modeActuel = STANDARD;
etatsysteme modePrecedent = STANDARD;
unsigned long nombre=0;
unsigned long lastActiveTime = 0;
unsigned long inactivityLimit = 1800000; // 30 minutes en millisecondes

enum ErreurType {
    ERREUR_RTC,
    ERREUR_GPS,
    ERREUR_CAPTEUR,
    ERREUR_CAPTEUR_INCOHERENT,
    ERREUR_SD_PLEINE,
    ERREUR_SD_ACCES
};

int LUMIN = 1, LUMIN_LOW = 255, LUMIN_HIGH = 768;
int TEMP_AIR = 1, MIN_TEMP_AIR = -10, MAX_TEMP_AIR = 60;
int HYGR = 1, HYGR_MINT = 0, HYGR_MAXT = 50;
int PRESSURE = 1, PRESSURE_MIN = 850, PRESSURE_MAX = 1080;

// Déclaration des variables globales pour l'heure, la date et le jour de la semaine
int CLOCK_HOUR = 0;
int CLOCK_MINUTE = 0;
int CLOCK_SECOND = 0;

int DATE_MONTH = 1;
int DATE_DAY = 1;
int DATE_YEAR = 2000;

int DAY = 1; // 1 pour lundi, 2 pour mardi, ..., 7 pour dimanche

void signalerErreur(ErreurType erreur) {
    while (1) {
        switch (erreur) {
            case ERREUR_RTC: 
                leds.setColorRGB(0, 255, 0, 0);
                delay(500);
                leds.setColorRGB(0, 0, 0, 255);
                delay(500);
                break;
            case ERREUR_GPS: 
                leds.setColorRGB(0, 255, 0, 0);
                delay(500);
                leds.setColorRGB(0, 255, 165, 0);
                delay(500);
                break;
            case ERREUR_CAPTEUR:
                leds.setColorRGB(0, 255, 0, 0);
                delay(500);
                leds.setColorRGB(0, 0, 255, 0);
                delay(500);
                break;
            case ERREUR_CAPTEUR_INCOHERENT:
                leds.setColorRGB(0, 255, 0, 0);
                delay(500);
                leds.setColorRGB(0, 0, 255, 0);
                delay(1000);
                break;
            case ERREUR_SD_PLEINE:
                leds.setColorRGB(0, 255, 0, 0);
                delay(500);
                leds.setColorRGB(0, 255, 255, 255);
                delay(500);
                break;
            case ERREUR_SD_ACCES:
                leds.setColorRGB(0, 255, 0, 0);
                delay(500);
                leds.setColorRGB(0, 255, 255, 255);
                delay(1000);
                break;
        }
    }
}

int LOG_INTERVAL = 1;
int FILE_MAX_SIZE = 4096;
int TIMEOUT = 30;
const String VERSION = "1.0.0";
const int EEPROM_SIZE = 512;

unsigned long lastLogTime = 0;
int revisionNumber = 0;
char logFileName[13];

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
        delay(100);
    }
}

void updateLogFileName() {
    DateTime now = rtc.now();
    snprintf(logFileName, sizeof(logFileName), "%02d%02d%02d_%d.LOG",
             now.year() % 100, now.month(), now.day(), revisionNumber);
}

float readLightLevel() {
    return analogRead(A0);
}

void setup() {
    Serial.begin(9600);
    gpsSerial.begin(9600);  // Initialiser le port série pour le GPS
    pinMode(button2, INPUT_PULLUP);
    pinMode(button3, INPUT_PULLUP);

    delay(2000); // Délai pour stabiliser le démarrage

    if (digitalRead(button3) == LOW) {
        modeActuel = CONFIGURATION;
        Serial.println(F("mode config"));
        leds.setColorRGB(0, 255, 175, 0);
        return;
    }

    // Initialisation RTC
    if (!rtc.begin()) {
        Serial.println(F("Erreur d'accès à l'horloge RTC"));
        signalerErreur(ERREUR_RTC);
    } else if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    if (!bme.begin(0x76)) {
        Serial.println(F("Erreur d'accès au capteur BME280"));
        signalerErreur(ERREUR_CAPTEUR);
    }

    // Vérification de cohérence des données du capteur (exemple de vérification)
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F; // hPa
    int luminosity = analogRead(luminosityPin);
    if (temperature < -40 || temperature > 85 || humidity < 0 || humidity > 100 || pressure<300 || pressure>1100 || luminosity<0 || luminosity>1023) {
        Serial.println("Données reçues d'un capteur incohérentes");
        signalerErreur(ERREUR_CAPTEUR_INCOHERENT);
    }

    // Vérifiez si le GPS est connecté
    bool gpsConnected = gpsSerial.available(); // Vérifier la disponibilité des données GPS
    if (!gpsConnected) {
        Serial.println("Erreur d'accès aux données du GPS");
        signalerErreur(ERREUR_GPS);
    }



    // Initialisation carte SD
    delay(500);
    if (!SD.begin(chipSelect)) {
        Serial.println(F("Erreur d'accès ou d'écriture sur la carte SD"));
        signalerErreur(ERREUR_SD_ACCES);
    }
    else {
        // Vérifier si la carte SD est pleine
        File root = SD.open("/");
        if (!root) {
            Serial.println("Erreur : Impossible d'accéder au système de fichiers SD");
        } else {
            root.rewindDirectory();
            bool isSDFull = false; // Assume a way to check if SD is full
            // Example check (you may need to adapt this part)
            if (isSDFull) {
                Serial.println("Erreur : Carte SD pleine");
                signalerErreur(ERREUR_SD_PLEINE);
            }
            root.close();
        }
    }


    leds.setColorRGB(0, 0, 255, 0); // LED verte pour le mode standard
    Serial.println(F("Mode Std"));
    updateLogFileName();
    lastLogTime = millis();
}

void logSensorData() {
    updateLogFileName();
    logFile = SD.open(logFileName, FILE_WRITE);

    if (!logFile) {
        Serial.println("Impossible d'ouvrir le fichier pour l'écriture.");
        signalerErreur(ERREUR_SD_ACCES);
        return;
    }

    if (logFile.size() >= FILE_MAX_SIZE) {
        logFile.close();
        revisionNumber++;
        updateLogFileName();
        logFile = SD.open(logFileName, FILE_WRITE);
        if (!logFile) {
            signalerErreur(ERREUR_SD_ACCES);
            return;
        }
    }

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

    logFile.print("Tempé: ");
    logSensorValue([&]() { return bme.readTemperature(); }, "NA");
    logFile.print("°C");

    logFile.print(", Humi: ");
    logSensorValue([&]() { return bme.readHumidity(); }, "NA");
    logFile.print("g/m3");

    logFile.print(", Pres: ");
    logSensorValue([&]() { return bme.readPressure() / 100.0F; }, "NA");
    logFile.print("HPa");

    logFile.print(", Lumi: ");
    logSensorValue([&]() { return readLightLevel(); }, "NA");
    logFile.print("lx");

    if (gps.location.isValid()) {
        logFile.print(", Lat: ");
        logFile.print(gps.location.lat(), 6);
        logFile.print(", Long: ");
        logFile.print(gps.location.lng(), 6);
        logFile.print(", Alti: ");
        logFile.print(gps.altitude.meters());
    } else {
        logFile.print(", Lat: NA, Long: NA, Alt: NA");
    }

    logFile.println();
    logFile.flush();
    logFile.close();
    Serial.println("Données enregistrées");
}

void logSensorData1() {
    updateLogFileName();
    logFile = SD.open(logFileName, FILE_WRITE);

    if (!logFile) {
        Serial.println("Impossible d'ouvrir le fichier pour l'écriture.");
        signalerErreur(ERREUR_SD_ACCES);
        return;
    }

    if (logFile.size() >= FILE_MAX_SIZE) {
        logFile.close();
        revisionNumber++;
        updateLogFileName();
        logFile = SD.open(logFileName, FILE_WRITE);
        if (!logFile) {
            signalerErreur(ERREUR_SD_ACCES);
            return;
        }
    }

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

    logFile.print("Tempé: ");
    logSensorValue([&]() { return bme.readTemperature(); }, "NA");
    logFile.print("°C");

    logFile.print(", Humi: ");
    logSensorValue([&]() { return bme.readHumidity(); }, "NA");
    logFile.print("g/m3");

    logFile.print(", Pres: ");
    logSensorValue([&]() { return bme.readPressure() / 100.0F; }, "NA");
    logFile.print("HPa");

    logFile.print(", Lumi: ");
    logSensorValue([&]() { return readLightLevel(); }, "NA");
    logFile.print("lx");
    if(nombre%2==0){
    if (gps.location.isValid()) {
        logFile.print(", Lat: ");
        logFile.print(gps.location.lat(), 6);
        logFile.print(", Long: ");
        logFile.print(gps.location.lng(), 6);
        logFile.print(", Alti: ");
        logFile.print(gps.altitude.meters());
    } else {
        logFile.print(", Lat: NA, Long: NA, Alt: NA");
    }
    }

    logFile.println();
    logFile.flush();
    logFile.close();
    nombre++;
    Serial.println("Données enregistrées");
}

void loop() {
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        gps.encode(c); // Lecture et décodage des données GPS
    }

    if (modeActuel == CONFIGURATION && (millis() - lastActiveTime >= inactivityLimit)) {
        modeActuel = STANDARD;
        leds.setColorRGB(0, 0, 255, 0);
        Serial.println("mode std");
    }

    if (modeActuel != CONFIGURATION) {
        if (digitalRead(button2) == LOW) {
            delay(5000);
            if (digitalRead(button2) == LOW) {
                if (modeActuel != MAINTENANCE) {
                    if (modeActuel == ECONOMIQUE) {
                        modeActuel = STANDARD;
                        leds.setColorRGB(0, 0, 255, 0);
                        Serial.println("mode std");
                    } else {
                        modePrecedent = modeActuel;
                        modeActuel = ECONOMIQUE;
                        leds.setColorRGB(0, 0, 0, 255); 
                        Serial.println("mode eco");
                    }
                }
            }
        }

        if (digitalRead(button3) == LOW) {
            delay(5000); 
            if (digitalRead(button3) == LOW) {
                if (modeActuel == MAINTENANCE) {
                    modeActuel = modePrecedent;
                    leds.setColorRGB(0, 0, 255, 0);
                    Serial.println("mode pcd");
                } else {
                    modePrecedent = modeActuel;
                    modeActuel = MAINTENANCE;
                    leds.setColorRGB(0, 255, 65, 0); 
                    Serial.println("mode maint");
                }
            }
        }
    }

    switch (modeActuel) {
        case STANDARD:
            Standardmode();
            break;
        case CONFIGURATION:
            ModeConfig();
            break;
        case MAINTENANCE:
            Maintenancemode();
            break;
        case ECONOMIQUE:
            Ecomode();
            break;
    }
}

void Standardmode() {
    leds.setColorRGB(0, 0, 255, 0); // LED verte pour le mode standard
    unsigned long currentMillis = millis();

    if (currentMillis - lastLogTime >= LOG_INTERVAL * 60000) {
        lastLogTime = currentMillis;
        logSensorData();
    }
}


void ModeConfig() {
    leds.setColorRGB(0, 255, 165, 0); // LED jaune
    static bool commandsDisplayed = false;

    // Afficher les commandes une seule fois lors de l'entrée en mode configuration
    if (!commandsDisplayed) {
        Serial.println(F("Commandes disponibles : LOG_INTERVAL, FILE_MAX_SIZE, TIMEOUT, CLOCK, DATE, DAY, RESET, VERSION, LUMIN, TEMP_AIR, HYGR, PRESSURE"));
        commandsDisplayed = true;
    }

    if (Serial.available()) {
        char command[32];
        Serial.readBytesUntil('\n', command, sizeof(command) - 1);

        int value, hour, minute, second, day, month, year;
        char dayOfWeek[4];

        // Utiliser une seule instance de Serial.print pour les valeurs mises à jour
        if (sscanf(command, "LOG_INTERVAL=%d", &value) == 1) {
            LOG_INTERVAL = value;
            EEPROM.update(0, LOG_INTERVAL);
            Serial.println(F("LOG_INTERVAL mis à jour."));
        } else if (sscanf(command, "FILE_MAX_SIZE=%d", &value) == 1) {
            FILE_MAX_SIZE = value;
            EEPROM.update(1, FILE_MAX_SIZE / 256);
            EEPROM.update(2, FILE_MAX_SIZE % 256);
            Serial.println(F("FILE_MAX_SIZE mis à jour."));
        } else if (sscanf(command, "TIMEOUT=%d", &value) == 1) {
            TIMEOUT = value;
            EEPROM.update(3, TIMEOUT);
            Serial.println(F("TIMEOUT mis à jour."));
        } else if (sscanf(command, "LUMIN=%d", &value) == 1) {
            LUMIN = value;
            EEPROM.update(4, LUMIN);
            Serial.println(F("LUMIN mis à jour."));
        } else if (sscanf(command, "LUMIN_LOW=%d", &value) == 1) {
            LUMIN_LOW = value;
            EEPROM.update(5, LUMIN_LOW);
            Serial.println(F("LUMIN_LOW mis à jour."));
        } else if (sscanf(command, "LUMIN_HIGH=%d", &value) == 1) {
            LUMIN_HIGH = value;
            EEPROM.update(6, LUMIN_HIGH);
            Serial.println(F("LUMIN_HIGH mis à jour."));
        } else if (sscanf(command, "HYGR=%d", &value) == 1) {
            HYGR = value;
            EEPROM.update(10, HYGR);
            Serial.println(F("HYGR mis à jour."));
        } else if (sscanf(command, "HYGR_MINT=%d", &value) == 1) {
            HYGR_MINT = value;
            EEPROM.update(11, HYGR_MINT);
            Serial.println(F("HYGR_MINT mis à jour."));
        } else if (sscanf(command, "HYGR_MAXT=%d", &value) == 1) {
            HYGR_MAXT = value;
            EEPROM.update(12, HYGR_MAXT);
            Serial.println(F("HYGR_MAXT mis à jour."));
        } else if (sscanf(command, "PRESSURE=%d", &value) == 1) {
            PRESSURE = value;
            EEPROM.update(13, PRESSURE);
            Serial.println(F("PRESSURE mis à jour."));
        } else if (sscanf(command, "PRESSURE_MIN=%d", &value) == 1) {
            PRESSURE_MIN = value;
            EEPROM.update(14, PRESSURE_MIN);
            Serial.println(F("PRESSURE_MIN mis à jour."));
        } else if (sscanf(command, "PRESSURE_MAX=%d", &value) == 1) {
            PRESSURE_MAX = value;
            EEPROM.update(15, PRESSURE_MAX);
            Serial.println(F("PRESSURE_MAX mis à jour."));
        } else if (sscanf(command, "TEMP_AIR=%d", &value) == 1) {
            TEMP_AIR = value;
            EEPROM.update(7, TEMP_AIR);
            Serial.println(F("TEMP_AIR mis à jour."));
        } else if (sscanf(command, "MIN_TEMP_AIR=%d", &value) == 1) {
            MIN_TEMP_AIR = value;
            EEPROM.update(8, MIN_TEMP_AIR);
            Serial.println(F("MIN_TEMP_AIR mis à jour."));
        } else if (sscanf(command, "MAX_TEMP_AIR=%d", &value) == 1) {
            MAX_TEMP_AIR = value;
            EEPROM.update(9, MAX_TEMP_AIR);
            Serial.println(F("MAX_TEMP_AIR mis à jour."));
        } else if (sscanf(command, "CLOCK=%d:%d:%d", &hour, &minute, &second) == 3) {
            CLOCK_HOUR = hour;
            CLOCK_MINUTE = minute;
            CLOCK_SECOND = second;
            Serial.print(F("Heure mise à jour : "));
            Serial.print(CLOCK_HOUR); Serial.print(":");
            Serial.print(CLOCK_MINUTE); Serial.print(":");
            Serial.println(CLOCK_SECOND);
        } else if (sscanf(command, "DATE=%d,%d,%d", &month, &day, &year) == 3) {
            DATE_MONTH = month;
            DATE_DAY = day;
            DATE_YEAR = year;
            Serial.print(F("Date mise à jour : "));
            Serial.print(DATE_MONTH); Serial.print("/");
            Serial.print(DATE_DAY); Serial.print("/");
            Serial.println(DATE_YEAR);
        } else if (sscanf(command, "DAY=%3s", dayOfWeek) == 1) {
            if (strcmp(dayOfWeek, "MON") == 0) DAY = 1;
            else if (strcmp(dayOfWeek, "TUE") == 0) DAY = 2;
            else if (strcmp(dayOfWeek, "WED") == 0) DAY = 3;
            else if (strcmp(dayOfWeek, "THU") == 0) DAY = 4;
            else if (strcmp(dayOfWeek, "FRI") == 0) DAY = 5;
            else if (strcmp(dayOfWeek, "SAT") == 0) DAY = 6;
            else if (strcmp(dayOfWeek, "SUN") == 0) DAY = 7;
            Serial.print(F("Jour de la semaine mis à jour : "));
            Serial.println(dayOfWeek);
        } else if (strncmp(command, "RESET", 5) == 0) {
            LOG_INTERVAL = 10;
            FILE_MAX_SIZE = 4096;
            TIMEOUT = 30;
            LUMIN = 1;
            EEPROM.update(0, LOG_INTERVAL);
            EEPROM.update(1, FILE_MAX_SIZE / 256);
            EEPROM.update(2, FILE_MAX_SIZE % 256);
            EEPROM.update(3, TIMEOUT);
            EEPROM.update(4, LUMIN);
            Serial.println(F("Config RESET."));
        } else if (strncmp(command, "VERSION", 7) == 0) {
            Serial.print(F("Version : "));
            Serial.println(VERSION);
        } else {
            Serial.println(F("Commande non reconnue."));
        }
  }
  lastActiveTime = millis(); // Met à jour le temps d'inactivité

}


void Maintenancemode() {
    leds.setColorRGB(0, 255, 65, 0); // LED orange
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    int luminosity = analogRead(luminosityPin);

    Serial.print("Temp : "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Hum : "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Pres : "); Serial.print(pressure); Serial.println(" hPa");
    Serial.print("Lum : "); Serial.print(luminosity); Serial.println(" lx");

    if (gps.location.isValid()) {
        Serial.print("Lat : ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Long : ");
        Serial.println(gps.location.lng(), 6);
        Serial.print("Alt : ");
        Serial.print(gps.altitude.meters());
    } else {
        Serial.println("Signal GPS non valide");
    }

    Serial.println();

    delay(2000);

   // Sortie du mode maintenance si bouton 3 pressé longtemps
   if (digitalRead(button3) == LOW) {
       delay(5000); // Validation de la sortie
       if (digitalRead(button3) == LOW) {
           Serial.println("Fin Maint");
           modeActuel = modePrecedent;

           // Mise à jour des LEDs selon le mode précédent
           if (modePrecedent == STANDARD) {
              leds.setColorRGB(0, 0, 255, 0); // LED verte
              Serial.println("Mode Std");
              lastLogTime = millis(); // Réinitialisation du minuteur
            } else if (modePrecedent == ECONOMIQUE) {
                leds.setColorRGB(0, 0, 0, 255); // LED bleue
                Serial.println("Mode Eco");
            }
        }
    }

}

void Ecomode() {
    leds.setColorRGB(0, 0, 0, 255); // LED bleue
    unsigned long currentMillis = millis();

    if (currentMillis - lastLogTime >= LOG_INTERVAL * 60000*2) {
        lastLogTime = currentMillis;
        logSensorData1();
    }
}