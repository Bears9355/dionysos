#include <Adafruit_BMP280.h>                           // Inclusion de la librairie BME280 d'Adafruit

// Constantes du programme
#define adresseI2CduBME280                0x76            // Adresse I2C du BME280 (0x76, dans mon cas, ce qui est souvent la valeur par défaut)
#define pressionAuNiveauDeLaMerEnHpa      1012         // https://fr.wikipedia.org/wiki/Pression_atmospherique (1013.25 hPa en moyenne, valeur "par défaut")
#define delaiRafraichissementAffichage    1500            // Délai de rafraîchissement de l'affichage (en millisecondes)

// Instanciation de la librairie BME280
Adafruit_BMP280 bme;


// ========================
// Initialisation programme
// ========================
void setup() {
  
  // Initialisation du port série (pour l'envoi d'infos via le moniteur série de l'IDE Arduino)
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Programme de test du BME280");
  Serial.println("===========================");
  Serial.println();

  // Initialisation du BME280
  Serial.print(F("Initialisation du BME280, à l'adresse [0x"));
  Serial.print(adresseI2CduBME280, HEX);
  Serial.println(F("]"));
  
  if(!bme.begin(adresseI2CduBME280)) {
    Serial.println(F("--> ÉCHEC…"));
  } else {
    Serial.println(F("--> RÉUSSIE !"));
  }
  Serial.println();
  
}


// ======================================
// Boucle principale (boucle perpétuelle)
// ======================================
void loop() {

  // Affichage de la TEMPÉRATURE
  Serial.print(F("Température = "));
  Serial.print(bme.readTemperature());
  Serial.println(F(" °C"));
  
  // Affichage de la PRESSION ATMOSPHÉRIQUE
  Serial.print(F("Pression atmosphérique = "));
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(F(" hPa"));

  // Affichage de l'estimation d'ALTITUDE
  Serial.print(F("Altitude estimée = "));
  Serial.print(bme.readAltitude(pressionAuNiveauDeLaMerEnHpa));
  Serial.println(F(" m"));


  // ... et on répète ce cycle à l'infini !
  delay(delaiRafraichissementAffichage);                // Avec x secondes d'attente, avant chaque rebouclage
  Serial.println();                                     // … et un saut de ligne pour faire joli ;)

}
