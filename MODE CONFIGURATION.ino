#include <EEPROM.h>  // Bibliothèque pour l'utilisation de l'EEPROM
#include <Wire.h>    // Bibliothèque pour la communication I2C
#include <RTClib.h>  // Bibliothèque pour la gestion du module RTC

// Paramètres par défaut
int LOG_INTERVAL = 10; // Intervalle par défaut entre les logs (en minutes)
int FILE_MAX_SIZE = 4096; // Taille max d'un fichier de log (en octets)
int TIMEOUT = 30; // Timeout par défaut pour les capteurs (en secondes)
const String VERSION = "1.0.0"; // Version du programme
const int EEPROM_SIZE = 512; // Taille EEPROM

// Nouveaux paramètres
int LUMIN = 1;            // Activation du capteur de luminosité
int LUMIN_LOW = 255;      // Seuil bas de luminosité
int LUMIN_HIGH = 768;     // Seuil haut de luminosité
int TEMP_AIR = 1;         // Activation du capteur de température de l'air
int MIN_TEMP_AIR = -10;   // Température minimale de l'air
int MAX_TEMP_AIR = 60;    // Température maximale de l'air
int HYGR = 1;             // Activation du capteur d'hygrométrie
int HYGR_MINT = 0;        // Température minimale pour hygrométrie
int HYGR_MAXT = 50;       // Température maximale pour hygrométrie
int PRESSURE = 1;         // Activation du capteur de pression
int PRESSURE_MIN = 850;   // Pression minimale
int PRESSURE_MAX = 1080;  // Pression maximale

// Déclaration des variables globales pour l'heure, la date et le jour de la semaine
int CLOCK_HOUR = 0;
int CLOCK_MINUTE = 0;
int CLOCK_SECOND = 0;

int DATE_MONTH = 1;
int DATE_DAY = 1;
int DATE_YEAR = 2000;

int DAY = 1; // 1 pour lundi, 2 pour mardi, ..., 7 pour dimanche



void setup() {
  Serial.begin(9600); // Initialisation de la communication série

  Serial.println("Mode Configuration: Entrez une commande.");
  Serial.println("Commandes disponibles : LOG_INTERVAL, FILE_MAX_SIZE, TIMEOUT, CLOCK, DATE, DAY, RESET, VERSION, LUMIN, TEMP_AIR, HYGR, PRESSURE");
}

void loop() {
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
