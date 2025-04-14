#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ChainableLED.h>
#include <avr/pgmspace.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>



#define DATA_PIN 4
#define CLOCK_PIN 5
#define NUM_LEDS 1

#define button3 2

const int chipSelect = 4;
Adafruit_BME280 bme;
File logFile;
ChainableLED leds(DATA_PIN, CLOCK_PIN, NUM_LEDS);
TinyGPSPlus gps;                // Initialisation de l'objet TinyGPS++
SoftwareSerial gpsSerial(7, 8);  // RX (7), TX (8) pour le GPS

const int luminosityPin = A0; // Broche analogique pour le capteur de luminosité

void setup() {
    Serial.begin(9600);
    gpsSerial.begin(9600);  // Initialiser le port série pour le GPS

    pinMode(button3, INPUT_PULLUP);

  Serial.println("Mode Maintenance : Les données peuvent être consultées en direct. Vous pouvez retirer la carte SD en toute sécurité.");
}

void loop() {
   
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
  // Pause avant la prochaine lecture
  delay(2000);

  // Sortir du mode maintenance avec le bouton (bouton 3)
  if (digitalRead(bouton3) == LOW) {
    Serial.println("Fin du mode Maintenance. Rebranchez la carte SD avant de quitter.");
    while (1); // Bloquer le programme après la sortie
  }
  Serial.println();
}
