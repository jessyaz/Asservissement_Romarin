//###############################################################################################//
//-------------------------------------BIBLIOTHEQUES---------------------------------------------//
//###############################################################################################//

#include <Wire.h>                     // Cette librairie vous permet de communiquer avec les composants
#include <CytronMotorDriver.h>

//###############################################################################################//
//-------------------------------------DECLARATION DES VARIABLES---------------------------------//
//###############################################################################################//

CytronMD Mot_Profondeur(PWM_DIR, 5, 4);    //Moteur de profondeur PWM = pin 4 et DIR = pin 5

//Declaration temps.
int time;     //Temps

//Intervalle cible
float Limite_cible = 0.8;    //limite de l'intervalle cible à 80cm de la profondeur cible (Profondeur_Save)
                             //valeur choisie arbitrairement pour simuler les variations de profondeur avec une seringue

//Intervalle d'arrêt
float Limite_arret = 0.3;    //limite de l'intervalle d'arrêt (à l'intérieur duquel Vitesse_mot = 0) à 30cm de la profondeur cible (Profondeur_Save)
                             //valeur choisie arbitrairement pour simuler les variations de profondeur avec une seringue

const int PresSensor = A0;      //on recupère les informations du capteur de pression

//Variables de calcul
float Profondeur_Save = 0.0;      //profondeur sauvegardée = objectif
float Vitesse_mot = 0.0;          //vitesse du moteur
float Diff_prof = 0.0;            //différence de profondeur entre le ROV et l'objectif fixé

//Variables pour conversion.
int Num_Pres_Val = 0;             //valeur numérique capteur de pression
float Pres_tension = 0.0;         //pression en voltes
float Pres_pascals = 0.0;         //pression en pascals
float Profondeur_m = 0.0;         //profondeur en métres

//Variables pour map
float map_min = 0.0;
float map_max = 3.0; //valeur en mètre définie par le cas limite = hauteur du bassin
float Vit_min = 30.0;
float Vit_max = 255.0;

float GetProf(){                                             //fonction calcul profondeur
  Num_Pres_Val = analogRead(PresSensor);
  Pres_tension = ((float)Num_Pres_Val*5.0)/1023;
  Pres_pascals = Pres_tension * 55668.66 + 90177.5;    
  Profondeur_m = (Pres_pascals - 99888)/10000;
}

float GetDiff(){                                     //fonction calcul de la différence de profondeur entre le ROV et l'objectif
  GetProf();
  Diff_prof = (Profondeur_Save - Profondeur_m);
}

void ProfondeurSetup() {
  //Demarrage de la communication avec le shield
  Serial.begin(9600); //Initialisation de la communication avec la console.

  GetProf();                                          
  Profondeur_Save = Profondeur_m;                    //profondeur enregistrer
  GetDiff();
  //Vitesse_mot = map(abs(Diff_prof), map_min, map_max, Vit_min, Vit_max);

}

void ProfondeurLoop() {

  //Temps en secondes
  time = millis() / 1000.0;   //temps en secondes

  //----------PROGRAMME PROFONDEUR STABLE----------//

  //Acquisition des valeurs données par le capteur de pression
  Num_Pres_Val = analogRead(PresSensor);      //On récupère une valeur entre 0 et 1023 pour la pression

  //Calcul différence de profondeur
  GetDiff();

  //Mapage de la vitesse par rapport à la différence de profondeur entre Prodondeur_Save et Profondeur_m
  Vitesse_mot = map(abs(Diff_prof), map_min, map_max, Vit_min, Vit_max);
  
  Serial.print("Profondeur mesurée :"); Serial.println(Profondeur_m);
  Serial.print("Profondeur enregistrée :"); Serial.println(Profondeur_Save);
  Serial.print("Diff_prof = "); Serial.println(Diff_prof);

  //Cas où le ROV dépasse la limite supérieure de l'intervalle cible -> le ROV doit plonger
  if (Diff_prof > Limite_cible) {
    while (Diff_prof > Limite_arret) {          //Continue à plonger tant qu'il n'a pas atteint la limite supérieure de l'intervalle d'arrêt
      delay(100);
      //Serial.println("Plonge");
      Serial.print("Diff_prof = "); Serial.println(Diff_prof);
      
      Vitesse_mot = map(abs(Diff_prof), map_min, map_max, Vit_min, Vit_max);     //Mise à jour de la vitesse à appliquer à Mot_Profondeur
      
      if(Vitesse_mot > 255){      //permet de ne pas dépasser la vitesse max = 255
        Vitesse_mot = 255;
      }
      
      Mot_Profondeur.setSpeed(Vitesse_mot);                                       //Moteur en mode FORWARD (vitesse positive)
      //Serial.print("Vitesse : "); Serial.println(Vitesse_mot);

      GetDiff();                                                                  //Mise à jour de la différence de pression                
      Serial.print("Profondeur mesurée :"); Serial.println(Profondeur_m);
      Serial.print("Diff_prof = "); Serial.println(Diff_prof);
      Serial.println(); 
    }

  }

  //Cas où le ROV dépasse la limite inférieure de l'intervalle cible -> le ROV doit émerger
  if (Diff_prof < (- Limite_cible)) {
    while (Diff_prof < (- Limite_arret)){                                     //Continue à émerger tant qu'il n'a pas atteint la limite inférieure de l'intervalle d'arrêt
      delay(100);
      //Serial.println("Emerge");
      //Serial.print("Diff_prof = "); Serial.println(Diff_prof);
      Vitesse_mot = map(abs(Diff_prof), map_min, map_max, Vit_min, Vit_max);  //Mise à jour de la vitesse à appliquer à Mot_Profondeur

      if(Vitesse_mot > 255){                                                  //permet de ne pas dépasser la vitesse max
        Vitesse_mot = 255;
      }
      
      Mot_Profondeur.setSpeed(-Vitesse_mot);                                  //Moteur en mode BACKWARD (vitesse négative)
      //Serial.print("Vitesse : "); Serial.println(-Vitesse_mot);

      GetDiff();                                                              //Mise à jour de la différence de pression
      Serial.print("Profondeur mesurée :"); Serial.println(Profondeur_m);
      Serial.print("Diff_prof = "); Serial.println(Diff_prof);
      Serial.println();  

    }
  }
 
  Mot_Profondeur.setSpeed(0);                                               //Lorsque l'on sort de la boucle on libére le moteur
}
