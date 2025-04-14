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
#include <TinyGPS++.h>

#define DATA_PIN 4
#define CLOCK_PIN 5
#define NUM_LEDS 1

#define button_rouge 2
#define button_vert 3

RTC_DS3231 rtc;
const int chipSelect = 4;
Adafruit_BME280 bme;
File logFile;
ChainableLED leds(DATA_PIN, CLOCK_PIN, NUM_LEDS);
TinyGPSPlus gps;
SoftwareSerial gpsSerial(7, 8);  // RX (7), TX (8) pour le GPS

const int luminosityPin = A0;

enum etatsysteme { STANDARD, CONFIGURATION, MAINTENANCE, ECONOMIQUE };
etatsysteme modeActuel = STANDARD;
etatsysteme modePrecedent = STANDARD;

unsigned long lastLogTime = 0;
unsigned long lastActiveTime = 0;
unsigned long inactivityLimit = 1800000;

enum ErreurType {
    ERREUR_RTC,
    ERREUR_GPS,
    ERREUR_CAPTEUR,
    ERREUR_CAPTEUR_INCOHERENT,
    ERREUR_SD_PLEINE,
    ERREUR_SD_ACCES
};

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

void interruption_button_vert() {
    lastActiveTime = millis();
}

void interruption_button_rouge() {
    lastActiveTime = millis();
}

void setup() {
    Serial.begin(9600);
    gpsSerial.begin(9600);  // Initialiser le port série pour le GPS
    pinMode(button_vert, INPUT_PULLUP);
    pinMode(button_rouge, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(button_vert), interruption_button_vert, FALLING);
    attachInterrupt(digitalPinToInterrupt(button_rouge), interruption_button_rouge, FALLING);

    delay(2000); // Délai pour stabiliser le démarrage

    if (digitalRead(button_rouge) == LOW) {
        modeActuel = CONFIGURATION;
        Serial.println(F("mode config"));
        leds.setColorRGB(0, 255, 175, 0);
        return;
    }

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

    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    int luminosity = analogRead(luminosityPin);
    if (temperature < -40 || temperature > 85 || humidity < 0 || humidity > 100 || pressure < 300 || pressure > 1100 || luminosity < 0 || luminosity > 1023) {
        Serial.println("Données reçues d'un capteur incohérentes");
        signalerErreur(ERREUR_CAPTEUR_INCOHERENT);
    }

    bool gpsConnected = gpsSerial.available();
    if (!gpsConnected) {
        Serial.println("Erreur d'accès aux données du GPS");
        signalerErreur(ERREUR_GPS);
    }

    delay(500);
    if (!SD.begin(chipSelect)) {
        Serial.println(F("Erreur d'accès ou d'écriture sur la carte SD"));
        signalerErreur(ERREUR_SD_ACCES);
    } else {
        File root = SD.open("/");
        if (!root) {
            Serial.println("Erreur : Impossible d'accéder au système de fichiers SD");
        } else {
            root.rewindDirectory();
            bool isSDFull = false; // Example check (adapt if necessary)
            if (isSDFull) {
                Serial.println("Erreur : Carte SD pleine");
                signalerErreur(ERREUR_SD_PLEINE);
            }
            root.close();
        }
    }

    leds.setColorRGB(0, 0, 255, 0);
    Serial.println(F("Mode Std"));
    lastLogTime = millis();
}

bool buttonvertPressed = false;
bool buttonrougePressed = false;

void loop() {
    if (modeActuel == CONFIGURATION && (millis() - lastActiveTime >= inactivityLimit)) {
        modeActuel = STANDARD;
        leds.setColorRGB(0, 0, 255, 0);
        Serial.println("mode std");
    }

    if (modeActuel != CONFIGURATION) {
        // Button 2 handling
        if (digitalRead(button_vert) == LOW && !buttonvertPressed && (millis() - lastActiveTime >= 5000)) {
            buttonvertPressed = true; // Mark button as pressed
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
            lastActiveTime = millis();
        } else if (digitalRead(button_vert) == HIGH) {
            buttonvertPressed = false; // Reset flag when button is released
        }

        // Button 3 handling
        if (digitalRead(button_rouge) == LOW && !buttonrougePressed && (millis() - lastActiveTime >= 5000)) {
            buttonrougePressed = true; // Mark button as pressed
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
            lastActiveTime = millis();
        } else if (digitalRead(button_rouge) == HIGH) {
            buttonrougePressed = false; // Reset flag when button is released
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

void Standardmode() {leds.setColorRGB(0, 0, 255, 0);}
void ModeConfig() {leds.setColorRGB(0, 255, 165, 0);}
void Maintenancemode() {leds.setColorRGB(0, 255, 65, 0);}
void Ecomode() {leds.setColorRGB(0, 0, 0, 255);}
