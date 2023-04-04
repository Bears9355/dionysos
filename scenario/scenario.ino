#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <SoftwareSerial.h>
#include <SD.h>

#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme(BME_CS); 

#define BuzzPin 7
#define JackPin 4
#define humsoilPin A5
int in1 = 5; 
int in2 = 3; // FORAGE
int in3 = 6;
int in4 = 8; // EJECTION

// Calcul de temp avec Steinhart-Hart
#define Vcc 5
#define Ro 33000 // Résistance du pont
#define C1 0.001123350537530867
#define C2 0.0002062877904942806
#define C3 8.063933695988366e-08
float u; // Tension CTN
float R; // Résistance CTN
float logR; // ln(R)
float Temp; // Temp en *C
int ldrr;
int ldrv;
int ldrb;
int humsoil;

const int SD_CS = 9;
File monFichier;

int time = 0;
int down = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("Initialisation BME680..."));
  if (!bme.begin()) {
    Serial.println(F("Impossible de trouver BME680."));
    while (1);
  }
  Serial.println(F("BME680 initialisé."));

  Serial.println(F("Initialisation carte SD :"));
  if (!SD.begin(SD_CS)) {
    Serial.println(F("Échec initialization SD"));
    while (1);
  }
  Serial.println(F("Initialisation terminée."));
  Serial.println();
  
  // Test buzzer
  pinMode(BuzzPin, OUTPUT);
  digitalWrite(BuzzPin, HIGH);
  delay(1000);
  digitalWrite(BuzzPin, LOW);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(4, INPUT);

  // Etalonnage
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
}

void loop() {
  if (! bme.performReading()) {
    Serial.println("Erreur lecture BME680");
    return;
  }

  u = analogRead(A0) *5.0/1023; // Lecture tension
  R = Ro * (Vcc-u)/u; // Calcul résistance
  logR = log(R); // Calcul ln(R)
  Temp = (1.0 / (C1 + C2*logR + C3*logR*logR*logR)); // Calcul temp
  Temp = Temp - 273.15; // Conversion en °C

  
  float humidite = bme.humidity;
  float pression = (bme.pressure / 100.0);
  float airquality = (bme.gas_resistance / 1000.0);
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  String humiditeArrondie = String(humidite,1);
  String pressionArrondie = String(pression,1);
  String airqualityArrondie = String(airquality,1);
  String altitudeArrondie = String(altitude,1);
  String temperature = String(Temp, 2); //Arrondie après 2 chiffres car capteur linéaire et précis
  humiditeArrondie.replace(".", ",");
  pressionArrondie.replace(".", ",");
  airqualityArrondie.replace(".", ",");
  altitudeArrondie.replace(".", ",");
  temperature.replace(".", ",");

  ldrr = analogRead(1);
  ldrv = analogRead(2);
  ldrb = analogRead(3);
  String ldrrString = String(ldrr);
  String ldrvString = String(ldrv);
  String ldrbString = String(ldrb);

  humsoil = analogRead(humsoilPin);
  String humsoilString = String(humsoil);

  if (digitalRead(4) != 1 && time < 40 && time > 0) { // Si Jack OFF et temps entre 0 et 40s, transmettre données
  // Affichage : temp;hum;press;iqa;alt.....
    Serial.println(" ");
    Serial.print(temperature + ";");
    Serial.print(humiditeArrondie + ";");
    Serial.print(pressionArrondie + ";");
    Serial.print(airqualityArrondie + ";");
    Serial.print(altitudeArrondie + ";");
    Serial.print(ldrrString + ";");
    Serial.print(ldrvString + ";");
    Serial.print(ldrbString + ";");
  } else if (digitalRead(4) != 1 && time > 40) {
    Serial.println(" ");
    Serial.print("S-" + temperature + ";" + humiditeArrondie + ";" + pressionArrondie + ";" + airqualityArrondie + ";" + altitudeArrondie + ";" + humsoilString + ";");
  }

  monFichier = SD.open("donnees.csv", FILE_WRITE);
  if (monFichier) {
    monFichier.print(temperature);
    monFichier.print(";");  
    monFichier.print(humiditeArrondie);              // Écriture dans le fichier
    monFichier.print(";");
    monFichier.print(pressionArrondie);
    monFichier.print(";");
    monFichier.print(airqualityArrondie);
    monFichier.print(";");
    monFichier.print(altitudeArrondie);
    monFichier.print(";");
    if (time >= 0 && time < 40) {
      monFichier.print(time);
      monFichier.print(";");
    } else {
      monFichier.print("END_OF_FALL");
      monFichier.print(";");
    }
    if (time > 40) {
      monFichier.print(ldrrString);
      monFichier.print(";");
      monFichier.print(ldrvString);
      monFichier.print(";");
      monFichier.print(ldrbString);
      monFichier.print(";");
      monFichier.print(humsoilString);
      monFichier.print(";");
    }
    monFichier.println("");
    monFichier.close(); 
    } else {
    Serial.println("Impossible ouvrir fichier");
    while (1);
  }

  if (digitalRead(4) != 1 && time > 40 && down == 0) {
    down = 1; // le forage a été effectué 
    digitalWrite(BuzzPin, HIGH);

    digitalWrite(in2, HIGH);
    digitalWrite(in1, LOW); // Faire tourner EJECTION (vérifier sens)
    delay(500); // à bien déterminer !
    digitalWrite(in2, LOW);
    delay(5000);
    digitalWrite(in4, LOW); // Faire tourner FORAGE vers le bas
    digitalWrite(in3, HIGH);
    delay(16000);
    digitalWrite(in3, LOW);
  }

  if (digitalRead(4) != 1) {
    time = time + 1;
  }
  delay(210);
}
