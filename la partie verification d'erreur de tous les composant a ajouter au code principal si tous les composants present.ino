// Initialisation de l'horloge RTC
    if (!rtc.begin()) {
        Serial.println("Erreur d'accès à l'horloge RTC");
        // LED clignotante rouge et blanche
    for (int i = 0; i < 10; i++) { // Clignote 10 fois (modifiable)
        // Allumer en rouge
        leds.setColorRGB(0, 255, 0, 0);  // LED 0, rouge
        delay(500); // Durée en rouge (0.5 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
 
        // Allumer en blanc
        leds.setColorRGB(0, 0, 0, 255); // LED 0, bleu
        delay(500); // Durée en bleu (1 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
    }
        while (1); // Bloquer si le RTC ne s'initialise pas
    }
   
   
   
    // Vérifier l'accès aux données du GPS
    if (!gps.begin()) { // Remplacez par l'appel correct pour initialiser le GPS
        Serial.println("Erreur d'accès aux données du GPS");
        // LED clignotante rouge et blanche
    for (int i = 0; i < 10; i++) { // Clignote 10 fois (modifiable)
        // Allumer en rouge
        leds.setColorRGB(0, 255, 0, 0);  // LED 0, rouge
        delay(500); // Durée en rouge (0.5 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
 
        // Allumer en blanc
        leds.setColorRGB(0, 255, 255, 0); // LED 0, jaune
        delay(500); // Durée en jaune (1 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
    }
        while(1); // Bloquer si l'accès au GPS échoue
    }
   
   
    //erreur accès capteur
   if (!bme.begin(0x76)) {
        Serial.println("Erreur d'accès au capteur");
    // LED clignotante rouge et blanche
    for (int i = 0; i < 10; i++) { // Clignote 10 fois (modifiable)
        // Allumer en rouge
        leds.setColorRGB(0, 255, 0, 0);  // LED 0, rouge
        delay(500); // Durée en rouge (0.5 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
 
        // Allumer en blanc
        leds.setColorRGB(0, 0, 255,0 ); // LED 0, VERTE
        delay(500); // Durée en VERT (1 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
    }
    while(1);
   }
 
 
// Vérification de la taille du fichier sur la carte SD
    File logFile = SD.open("your_log_file.log"); // Remplacez par votre nom de fichier
    if (logFile.size() >= FILE_MAX_SIZE) {
        logFile.close();
        Serial.println("Erreur : la carte SD est pleine !");
       
        // LED clignotante rouge et blanche
        for (int i = 0; i < 10; i++) {
            // Allumer en rouge
            leds.setColorRGB(0, 255, 0, 0);  // LED 0, rouge
            delay(500); // Durée en rouge (0.5 seconde)
 
            // Éteindre la LED
            leds.setColorRGB(0, 0, 0, 0);  // Éteindre
            delay(500); // Durée éteinte (0.5 seconde)
 
            // Allumer en blanc
            leds.setColorRGB(0, 255, 255, 255); // LED 0, blanc
            delay(500); // Durée en blanc (0.5 seconde)
 
            // Éteindre la LED
            leds.setColorRGB(0, 0, 0, 0);  // Éteindre
            delay(500); // Durée éteinte (0.5 seconde)
        }
        while (1); // Bloquer si la carte SD est pleine
    }
 
 
 
    // Initialisation de la carte SD
    if (!SD.begin(chipSelect)) {
    Serial.println("Erreur d'accès ou d'écriture sur la carte SD");
 
    // LED clignotante rouge et blanche
    for (int i = 0; i < 10; i++) { // Clignote 10 fois (modifiable)
        // Allumer en rouge
        leds.setColorRGB(0, 255, 0, 0);  // LED 0, rouge
        delay(500); // Durée en rouge (0.5 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
 
        // Allumer en blanc
        leds.setColorRGB(0, 255, 255, 255); // LED 0, blanc
        delay(1000); // Durée en blanc (1 seconde)
 
        // Éteindre la LED
        leds.setColorRGB(0, 0, 0, 0);  // Éteindre
        delay(500); // Durée éteinte (0.5 seconde)
    }
 
    while (1); // Bloquer si la carte SD ne s'initialise pas
}