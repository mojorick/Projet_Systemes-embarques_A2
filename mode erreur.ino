#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <ChainableLED.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>

#define DATA_PIN 4
#define CLOCK_PIN 5
#define NUM_LEDS 1
RTC_DS3231 rtc;
const int chipSelect = 4;
Adafruit_BME280 bme;

ChainableLED leds(DATA_PIN, CLOCK_PIN, NUM_LEDS);

// Enum pour les types d'erreurs
enum ErreurType {
    ERREUR_RTC,              // Rouge et bleu, 1Hz, durée identique
    ERREUR_GPS,              // Rouge et jaune, 1Hz, durée identique
    ERREUR_CAPTEUR,          // Rouge et vert, 1Hz, durée identique
    ERREUR_CAPTEUR_INCOHERENT, // Rouge et vert, 1Hz, durée double pour le vert
    ERREUR_SD_PLEINE,        // Rouge et blanc, 1Hz, durée identique
    ERREUR_SD_ACCES          // Rouge et blanc, 1Hz, durée double pour le blanc
};

// Fonction pour signaler une erreur avec une LED clignotante spécifique
void signalerErreur(ErreurType erreur) {
    while (1) {
        switch (erreur) {
            case ERREUR_RTC:  // Rouge et bleu, 1Hz (500ms chaque couleur)
                leds.setColorRGB(0, 255, 0, 0); // Rouge
                delay(500);
                leds.setColorRGB(0, 0, 0, 255); // Bleu
                delay(500);
                break;
            case ERREUR_GPS:  // Rouge et jaune, 1Hz (500ms chaque couleur)
                leds.setColorRGB(0, 255, 0, 0); // Rouge
                delay(500);
                leds.setColorRGB(0, 255, 165, 0); // Jaune
                delay(500);
                break;
            case ERREUR_CAPTEUR:  // Rouge et vert, 1Hz (500ms chaque couleur)
                leds.setColorRGB(0, 255, 0, 0); // Rouge
                delay(500);
                leds.setColorRGB(0, 0, 255, 0); // Vert
                delay(500);
                break;
            case ERREUR_CAPTEUR_INCOHERENT:  // Rouge et vert, 1Hz (rouge 500ms, vert 1000ms)
                leds.setColorRGB(0, 255, 0, 0); // Rouge
                delay(500);
                leds.setColorRGB(0, 0, 255, 0); // Vert
                delay(1000);
                break;
            case ERREUR_SD_PLEINE:  // Rouge et blanc, 1Hz (500ms chaque couleur)
                leds.setColorRGB(0, 255, 0, 0); // Rouge
                delay(500);
                leds.setColorRGB(0, 255, 255, 255); // Blanc
                delay(500);
                break;
            case ERREUR_SD_ACCES:  // Rouge et blanc, 1Hz (rouge 500ms, blanc 1000ms)
                leds.setColorRGB(0, 255, 0, 0); // Rouge
                delay(500);
                leds.setColorRGB(0, 255, 255, 255); // Blanc
                delay(1000);
                break;
        }
    }
}

void setup() {
    Serial.begin(9600);
        // Initialisation RTC
    if (!rtc.begin()) {
        Serial.println("Erreur d'accès à l'horloge RTC");
        signalerErreur(ERREUR_RTC);
    }

    // Initialisation capteur BME280
    if (!bme.begin(0x76)) {
        Serial.println("Erreur d'accès au capteur");
        signalerErreur(ERREUR_CAPTEUR);
    }

    // Initialisation carte SD
    if (!SD.begin(chipSelect)) {
        Serial.println("Erreur d'accès ou d'écriture sur la carte SD");
        signalerErreur(ERREUR_SD_ACCES);
    }

    // Pour tester les erreurs, décommentez la ligne correspondant à l'erreur que vous souhaitez tester :
    //signalerErreur(ERREUR_RTC);               // Test de l'erreur RTC (rouge et bleu)
    // signalerErreur(ERREUR_GPS);            // Test de l'erreur GPS (rouge et jaune)
    //signalerErreur(ERREUR_CAPTEUR);        // Test de l'erreur de capteur (rouge et vert)
    // signalerErreur(ERREUR_CAPTEUR_INCOHERENT); // Test de l'erreur de capteur incohérent (rouge et vert, vert plus long)
    // signalerErreur(ERREUR_SD_PLEINE);      // Test de l'erreur carte SD pleine (rouge et blanc)
    //signalerErreur(ERREUR_SD_ACCES);       // Test de l'erreur d'accès à la carte SD (rouge et blanc, blanc plus long)
    //en gros ce code etait seulement le code erreur qui etait juste pour tester si les led clignotaient donc tout ce qui est en commentaire permettaient de tester
}

void loop() {
    // Rien à faire dans la boucle principale pour ce test
}
