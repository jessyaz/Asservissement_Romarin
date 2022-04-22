#include "Arduino.h"
#include <Wire.h>
#include "bmm150.h"
#include "bmm150_defs.h"
#include "CytronMotorDriver.h"

CytronMD motor1(PWM_DIR, 3, 2); //Declaration de l'objet motor1
CytronMD motor2(PWM_DIR, 5, 4);
CytronMD motor3(PWM_DIR, 6, 7);

BMM150 bmm = BMM150(); //On crée un objet global, bmm de type BMM150 qui permettra de stocker la data du capteur et de l'injecté dans value. Ainsi que récupéré dans tout notre code les valeurs du compass.

int which_dir(int ref, int dir) { //on cherche et on determine l'écart de la direction par rapport.
  int result;
  int resultneg = 360 - dir;
  int resultpos = dir;
  if (resultneg > dir) {
    result = (-1) * dir;
  } else {
    result = resultneg;
  }
  return result;
}


void pression() {
  float capteur_pression;
  capteur_pression = analogRead(A0);
  capteur_pression = capteur_pression * 5 / 1013.25; //etalonnage patm = 0,18 || p(1m d'eau) = 0,37 || normalisationNature : Ax+b ; A = 51554,1316 ; b = 92045,2563
  capteur_pression = capteur_pression * (51554.1316) + 92045, 2563;
  Serial.println(capteur_pression);
}




const int numReadings  = 10; //nb lecture pour faire la moyenne glissante
int readings [numReadings];
int readIndex  = 0;
long total  = 0;

long smooth(int ref) { /* function smooth */
  long average;
  total = total - readings[readIndex];
  readings[readIndex] = which_dir(ref, compass());
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = total / numReadings;
  return average;
}



int compass() {

  /* Partie objet */
  bmm150_mag_data value; //On crée un objet local value de type bmm150_mag_data
  
  bmm.read_mag_data(); //On lit les données du capteur et on les écrits dans l'objet bmm déclaré globalement ! (voir partie déclaration du code)
  
  value.x = bmm.raw_mag_data.raw_datax; //On stock ici dans l'objet value, la variable x qui prend dans l'objet bmm, la valeur de raw_mag_data.raw_datax.
  value.y = bmm.raw_mag_data.raw_datay;  //On stock ici dans l'objet value, la variable y qui prend dans l'objet bmm, la valeur de raw_mag_data.raw_datay.
  /* x, y deux coordonnées cartésiennes, ciblant un point sur un graphique plan */
  /* Fin partie objet */

  /* On cherche l'angle qui associe les coordonées (x,y) par rapport à la coordonnée (0,0) */
  float heading = atan2(value.x, value.y); 
  //atan2() permet de convertir une coordonnée cartesienne en radian angulaire. voir ref sur internet.

  if (heading < 0) /* Si la valeur heading passe en dessous de 0 */
    heading += 2 * PI; /* On lui ajoute 2PI */
  if (heading > 2 * PI) /* si la valeur heading dépasse 360 */
    heading -= 2 * PI; /* On lui soustrait 2PI */
    
  float headingDegrees = heading * 180 / M_PI; //On convertie heading qui était en radian, en dégré angulaire. 
  return (int)headingDegrees; //On retourne headingDegrees convertie en (int).
}


void ctrl_motor(int m1, int m2, int m3) {//Fonction de controle des moteurs de manière globale.
  delay(10); //delay de décalage minimum
  motor1.setSpeed(m1);   
  motor2.setSpeed(m2); 
  motor3.setSpeed(m3);
}



void setup() {

  //A MODIF
  Serial.begin(9600);
  
  pinMode(A0, INPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  
  //FIN
  

  //Initialisation et test de la boussole.
  if (bmm.initialize() == BMM150_E_ID_NOT_CONFORM) {
    Serial.println("Chip ID can not read!");
    while (1);
  } else {
    Serial.println("Initialize done!");
  }
  //FIN

}

int vitesse() { //Fonction de vitesse retournant la vitesse d'approche
  int vitesse = 256;
  return vitesse;
}


float k_p(int dir_p) { //Coeffiction, moteur 1, adapte vitesse pour rotation.
  //k_p doit varier de 0 a 1
  int dir = abs(dir_p);
  if (dir == 0 ) {
    return 0;
  }

  if (1 < dir <= 5) {
    return (0.025 * (dir - 1) );
  } else if (6 <= dir <= 10) {
    return (0.03 * (dir - 5) + 0.1);
  }
  else if (11 <= dir <= 20) {
    return (0.025 * (dir - 10) + 0.25);
  }
  else if (dir > 20) {
    return 1;
  }
}


float k_a(int dir_p) { //Coeffiction, moteur 2, adapte vitesse pour rotation. (coefficient de reduction)
  //varie de 0 a 1
  
  int dir = abs(dir_p);
    if(11 <= dir <= 20) {
  return (0.025 * (dir - 10) + 0.25);
   }
  if (dir > 20) {
    return 1;
  }
  return 0;
}


int i = 0;
void loop() {

  int ref = 357;

  long dir_long = smooth(ref);
  int dir = (int)dir_long;

  Serial.println(dir);

  int ma =  vitesse();
  int mp = ( 1 - k_p(dir)) * vitesse(); //( 1 - k_a(dir))

  if (dir < 0) { 
    // moteur droit actif
    motor1.setSpeed(mp);
    motor3.setSpeed((-1)*ma);
  } else {
    //moteur gauche actif
    motor1.setSpeed(ma);
    motor3.setSpeed((-1)*mp);
  }
  delay(100);
}
