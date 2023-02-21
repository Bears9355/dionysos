/***************************************************************************
  Code de Basile pour le cansat

  Fonctionnalités : 
    - Lecture des données du BME680
    - Envoie de ces données à la carte SD

  Les capteurs/modules utilisent le bus SPI ou analogique.

  /!\ IMPORTANT : /!\
  /!\ Veuillez mettre un fichier nommé "donnees.csv" dans la carte SD.  /!\
  /!\ Ce fichier doit être VIDE. Importez le code avant d'insérer la    /!\
  /!\ carte SD pour éviter d'avoir des données qui ne sont pas dans la  /!\
  /!\ bonne case...                                                     /!\
  /!\ Modifiez également la valeur SEALEVELPRESSURE_HPA en mettant la   /!\
  /!\ valeur obtenue ici : https://metar-taf.com/LFFW (à droite, en hPa)/!\
  /!\ Cette valeur permet de déterminer l'altitude grâce à la pression  /!\
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <SD.h>

const int SD_CS = 9;
const char* nomDuFichier = "donnees.csv";
File monFichier;
const long delaiIntervalleDeMesures = 2000;

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1015)

//Adafruit_BME680 bme; // I2C
Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
  Serial.begin(9600);
  SD.begin(SD_CS);
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  Serial.print(F("Initialisation de la carte SD..."));
  if (!SD.begin(SD_CS)) {
    Serial.println();
    Serial.println();
    Serial.println(F("Échec de l'initialisation du lecteur de SD card. Vérifiez :"));
    Serial.println(F("1. que la carte SD soit bien insérée"));
    Serial.println(F("2. que votre câblage soit bon"));
    Serial.println(F("3. que la variable 'sdCardPinChipSelect' corresponde bien au branchement de la pin CS de votre carte SD sur l'Arduino"));
    Serial.println(F("Et appuyez sur le bouton RESET de votre Arduino une fois le pb résolu, pour redémarrer ce programme !"));
    while (true);
  }
  Serial.println(F(" réussi !"));
  Serial.println();

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}



void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  // Lecture des données
  float tauxHumidite = bme.humidity;        // Lecture du taux d'humidité, exprimé en %
  float temperature = bme.temperature;      // Lecture de la température ambiante, exprimée en degrés celsius
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float pression = (bme.pressure / 100.0);
  float resistanceGaz = (bme.gas_resistance / 1000.0);

  // Vérification si données bien reçues
  if (isnan(tauxHumidite) || isnan(temperature) || isnan(altitude) || isnan(pression) || isnan(resistanceGaz)) {
    Serial.println(F("Une des valeurs du BME n'est pas valide ou absente. Est-il bien branché ?"));
    delay(2000);
    return;         // Si aucune valeur n'a été reçue par l'Arduino, on attend 2 secondes, puis on redémarre la fonction loop()
  }

  // Mise en forme de ces données (un seul chiffre après la virgule)
  String tauxHumiditeArrondi = String(tauxHumidite,1);    // Affichage d'une seule décimale (car précision de 0,1 % ici)
  String temperatureArrondie = String(temperature,1);     // Affichage d'une seule décimale (car précision de 0,1 °C ici)
  String altitudeArrondie = String(altitude,1);           // Affichage d'une seule décimale (car précision de 0.1 m ici)
  String pressionArrondie = String(pression,1);           // Affichage d'une seule décimale (car précision de 0.6 hPa ici)
  String gazArrondie = String(resistanceGaz,1);           // Affichage d'une seule décimale 
  tauxHumiditeArrondi.replace(".", ",");                  // et on remplace les séparateurs de décimale, initialement en "." pour devenir ","
  temperatureArrondie.replace(".", ",");                  // (permet par exemple d'affichier 12,76 au lieu de 12.76 (ce qui facilite l'interprétation dans Excel ou tout autre tableur)
  altitudeArrondie.replace(".", ",");
  pressionArrondie.replace(".", ",");
  gazArrondie.replace(".", ",");

  // Affichage des valeurs sur le moniteur série de l'IDE Arduino
  Serial.print("Humidity = "); Serial.print(tauxHumiditeArrondi); Serial.print(" % - ");
  Serial.print("Temperature = "); Serial.print(temperatureArrondie); Serial.println(" *C - ");
  Serial.print("Altitude = "); Serial.print(altitudeArrondie); Serial.println(" m - ");
  Serial.print("Pression = "); Serial.print(pressionArrondie); Serial.println(" hPa - ");
  Serial.print("Gaz = "); Serial.print(gazArrondie); Serial.println(" kOhm - ");

  // Gérer le temps pour avoir des données selon un temps
  unsigned long time = millis();

  // Enregistrement de ces données sur la carte SD
  monFichier = SD.open(nomDuFichier, FILE_WRITE);
  if (monFichier) {    
    monFichier.print(tauxHumiditeArrondi);
    monFichier.print(";");                  // Délimiteur du fichier CSV
    monFichier.println(temperatureArrondie);
    monFichier.print(";");
    monFichier.print(altitudeArrondie);
    monFichier.print(";");
    monFichier.print(pressionArrondie);
    monFichier.print(";");
    monFichier.print(gazArrondie);
    monFichier.print(";");
    monFichier.print(time);
    monFichier.close();                     // L'enregistrement des données se fait au moment de la clôture du fichier
    Serial.println(F("Enregistrement réussi en carte SD"));
  }
  else {
    Serial.println(F("Erreur lors de la tentative d'ouverture du fichier en écriture, sur la carte SD"));
  }

  // Temporisation de X secondes (2 sec min, pour que le BME680 ait le temps de faire ses mesures)
  Serial.println();
  delay(2000);
}


/*
Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
  */