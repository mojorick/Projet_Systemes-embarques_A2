#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;  // Crée une instance de l'horloge DS3231

void setup() {
  Serial.begin(9600);

  // Vérifie la connexion à l'horloge RTC
  if (!rtc.begin()) {
    Serial.println("Erreur: Impossible de trouver le module RTC");
    while (1);  // Bloque le programme si l'horloge n'est pas détectée
  }

  // Initialise l'horloge avec la date et l'heure de compilation, uniquement si elle a perdu l'heure
  if (rtc.lostPower()) {
    Serial.println("RTC a perdu l'alimentation, initialisation de l'heure...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  DateTime now = rtc.now();  // Lit la date et l'heure actuelles

  // Affiche la date et l'heure dans le moniteur série
  Serial.print("Date : ");
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(" Heure : ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  delay(1000);  // Attend une seconde avant de mettre à jour l'heure
}
