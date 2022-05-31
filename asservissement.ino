int pin_boutton_compass = 8 ; //pin ou est connecté le bouton
int pin_boutton_profondeur = 11;  //the pin where we connect the button

void setup() {
  Serial.begin(9600);

  pinMode(pin_boutton_compass, INPUT); //set the button pin as INPUT
  pinMode(pin_boutton_profondeur, INPUT); //set the button pin as INPUT

  DirectionSetup();
  ProfondeurSetup();
}

int stateProfondeur = digitalRead(pin_boutton_profondeur); //Permet de creer une variable pour savoir si la profondeur est fixer
int stateDirection = digitalRead(pin_boutton_compass); //Permet de creer une variable pour savoir si la direction est fixer

void loop() {

  if ( (digitalRead(pin_boutton_profondeur) == 1 ) && (stateProfondeur == 0) ) {
    Serial.println("Profondeur fixé !");
    stateProfondeur = 1;
    delay(100);
  } else if ( (digitalRead(pin_boutton_profondeur) == 1 ) && (stateProfondeur == 1) ) {
    stateProfondeur = 0;
    delay(100);
  }

  if ( (digitalRead(pin_boutton_compass) == 1 ) && (stateDirection == 0) ) {
    Serial.println("Direction fixé !");
    stateDirection = 1;
    delay(100);
  } else if ( (digitalRead(pin_boutton_compass) == 1 ) && (stateDirection == 1) ) {
    stateDirection = 0;
    delay(100);
  }

  if (stateDirection == 1) {
    Serial.println("Direction fixé !");
    DirectionLoop();
  }

  if (stateProfondeur == 1) {
    ProfondeurLoop();
  }

}
