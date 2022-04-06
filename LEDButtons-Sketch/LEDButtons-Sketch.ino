/*
 * Projekt: DMI-CADE
 * Autor: Jan Schneider
 * Arduino Sketch zum einlesen von RGB-Werten im HEX Format über eine serielle Schnittstelle.
 * Die Werte werden anschließend auf zwei verschiedene LED-Stripes geladen.
 * Beispieleingaben über ein serielles Terminal (für insgesamt 12 Buttons an der Arcade):
 * 000000;FF00FF;FFFFFF;00FFFF;00FF00;0000FF;000000;FF00FF;FFFFFF;00FFFF;00FF00;0000FF; 
*/

#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define DATA_PIN1     3
#define DATA_PIN2     5

#define MENUEBUTTON   8
#define STARTBUTTON1  9
#define STARTBUTTON2  10
#define COINBUTTON1   11
#define COINBUTTON2   12

//Anzahl der Control Buttons (2xCoin, 2xStart, 1xMenü)
#define CON_BUTTONS   5

#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

//LEDs in Reihe pro Player
#define NUM_LEDS      12
//Es können maximal 12 HEX Werte übergeben werden, einen pro Button. Dazu kommt noch ein Abschnitt für die Coin/Menü/Start Buttons
#define MAX_INPUT_STREAM_VALUES   13

#define BRIGHTNESS         150
#define FRAMES_PER_SECOND  120

//Doku: http://fastled.io/docs/3.1/index.html
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

//In den nachfolgenden Arrays werden die übergebenden Werte als String abgespeichert. (12 LEDs pro Player)
String player1[NUM_LEDS];
String player2[NUM_LEDS];

bool controlButtons[CON_BUTTONS];

//counter wird verwendet um die maximal 6 Input Werte auf das Array von 12 LEDs zu mappen.
int counter = 0;

//Globale Variablen aus der Funktion getData()
int stringData;
String sectionData;

/*
 * Wird rainbow auf true gesetzt, wird bei jedem Durchlauf der Loop ein neuer Farbwert auf die Buttons gepusht.
 * Dieser wird mittels rainbow_cycle_state kontinuierlich angepasst. So werden mit der Zeit alle Farben des 255er RGB Spektrums durchlaufen.
 * Der Befehl rainbow; oder rainbow;\n wird auch über die serielle Schnittstelle abgefangen.
 */
bool rainbow = false;
int rainbow_cycle_state = 0;

void setup() {
  delay(500); // 0,5 Sekunden delay um korrekt hochzufahren (optional)
  
  //Die FastLED Objekte konfigurieren.
  //FastLED.addLeds<Chipsatz der LEDs, Output Pin, Farbreihenfolge der LEDs>(Array der LEDs, Anzahl der zu konfigurierenden LEDs in dem Array).setCorrection(Art der LED Anordnung);
  FastLED.addLeds<LED_TYPE,DATA_PIN1,COLOR_ORDER>(leds1, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN2,COLOR_ORDER>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  //Legt die allgemeine Helligkeit fest. (Werte zwischen 0 und 255)
  FastLED.setBrightness(BRIGHTNESS);

  //Zum Testen der LEDs - wird auch so lange angezeigt, bis eine serielle Verbindung besteht.
  for(int i = 0; i < NUM_LEDS; i++){
   leds1[i] = CRGB::Blue;
   leds2[i] = CRGB::Blue;
  }
  FastLED.show();

  //Starten des seriellen Ports mit 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
    ; //Warten bis ein serieller Port verbunden ist.
  }

  pinMode(MENUEBUTTON, OUTPUT); 
  pinMode(STARTBUTTON1, OUTPUT); 
  pinMode(STARTBUTTON2, OUTPUT); 
  pinMode(COINBUTTON1, OUTPUT); 
  pinMode(COINBUTTON2, OUTPUT);

  /*
   * Test für Pin Buttons setzt alle auf HIGH. Kann später wieder auskommentiert werden!
   */
  digitalWrite(MENUEBUTTON, HIGH);
  digitalWrite(STARTBUTTON1, HIGH);
  digitalWrite(STARTBUTTON2, HIGH);
  digitalWrite(COINBUTTON1, HIGH);
  digitalWrite(COINBUTTON2, HIGH);
}

void loop() {

  //Solange der serielle Port Daten im Input erkennt wird diese Schleife ausgeführt.
  while (Serial.available() > 0) {

    //Wenn ein valider String empfangen wird, soll dieser eingelesen werden.
    String input = Serial.readString();

    //Testen, ob Input valide ist:
    if(correctInput(input)){

      //switch case um verschiede Modi abzubilden: 1 = RGB Werte auf Button pushen, 2 = rainbow Mode
      //Für eigene Module können hier cases ergänzt werden.
      switch(checkState(input)){
        case 1:
          //Normaler Input
          rainbow = false;
          
          saveInputData(input);
          readDataAndPushToLEDs();
          break;
          
        case 2:
          //Rainbow Sate
          rainbow = true;
          break;
          
        default:
          rainbow = false;
          break;
      }
    }
  }

  //Prüft in der loop(), ob rainbow aktiv sein soll.
  if(rainbow){
    rainbowState();
  }
}

bool correctInput(String input){
  /*  
   *   Um fehlerhafte Inputs oder gezielte Manipulation über den seriellen Eingang zu vermeiden wird auf die Länge des Inputs geprüft.
   *   Eine normale Übertragung für alle 12 Buttons sollte 84 Zeichen lang sein (mit einem NewLine \n am Ende 85).
   *   Wird rainbow; übergeben sollte der Input 8 oder 9 Zeichen lang sein.
   *   Sollten noch weitere Module wie rainbow hinzukommen, müssten diese hier mit aufgenommen werden.
   *   Allerdings sollte man auf dem Arduino nur noch Module ergänzen, die nicht über den Processmanager erstellt werden können.
   *   (Dort ist eine Implementierung um vielfaches einfacher!)
   */
  if(input.length() == 90 || input.length() == 91 || input.length() == 8 || input.length() == 9){
    return true;
  }else{
    return false;
  }
}

int checkState(String input){

  //Doppelte Prüfung für Eingabe über Konsole mit einem NewLine Zeichen oder ohne.
  if((input == "rainbow;\n") || (input == "rainbow;")){
    return 2;
  }else{
    return 1;
  } 
}

void saveInputData(String input){
  
  for(int i = 0; i < MAX_INPUT_STREAM_VALUES; i++){

    //Sucht in dem String nach dem Trennzeichen. i ist der i-te Teilstring, den getData() zurück geben soll.
    String buttonData = getData(input, ';', i);
  
    if(buttonData != ""){

      if(i == 0){
        
        for (int j = 0; j < CON_BUTTONS; i++) {
          
          controlButtons[j] = buttonData[j];
        }

        digitalWrite(MENUEBUTTON, controlButtons[0]);
        digitalWrite(STARTBUTTON1, controlButtons[1]);
        digitalWrite(STARTBUTTON2, controlButtons[2]);
        digitalWrite(COINBUTTON1, controlButtons[3]);
        digitalWrite(COINBUTTON2, controlButtons[4]);
        
      }else{
        
        if(i < MAX_INPUT_STREAM_VALUES / 2){
        
          //Überschreibt den Wert im player1 Array an der Stelle des Counters und auch die nachfolgende Stelle.
          //Zum Ende wird der Counter dann um 2 erhöht für das nächste LED Tuple.
          player1[counter] = buttonData;
          player1[counter+1] = buttonData;
          counter = counter +2;
  
          //Wenn die ersten Inputwerte erfolgreich dem ersten Spieler zugewiesen wurden muss der Counter zurück gesetzt werden.
          if(counter >= NUM_LEDS){
            counter = 0;
          }
          
        }else{
          
          //Überschreibt den Wert im player2 Array an der Stelle des Counters und auch die nachfolgende Stelle.
          //Zum Ende wird der Counter dann um 2 erhöht für das nächste LED Tuple.
          player2[counter] = buttonData;
          player2[counter+1] = buttonData;
          counter = counter +2;
        }
      }
    }
  }
  
  //Nach dem alle Werte eingelesen und abgespeichert wurden, wird der counter erneut zurückgesetzt.
  counter = 0;
}

void readDataAndPushToLEDs(){
  
  //Nun wird einmal über die beiden player Arrays itteriert und die Farbwerte in das FastLED Array geladen.
  for (int i = 0; i < NUM_LEDS; i++){

    //Die String Werte müssen noch als Zahlen gecastet werden. Dafür diese magische Funktion:
    long strToByte1 = strtol(&(player1[i])[0], NULL, 16);
    long strToByte2 = strtol(&(player2[i])[0], NULL, 16);
    //Farbwert der LED an Stelle i setzten.
    leds1[i] = strToByte1;
    leds2[i] = strToByte2;
  }

  //Zum Schluss noch die neuen Farbwerte auf die LEDs laden.
  FastLED.show();
}

String getData(String input, char delimiter, int sequence)
{
  stringData = 0;
  sectionData = "";

  for (int i = 0; i < input.length() - 1; i++)
  {

    if (input[i] == delimiter)
    {
      stringData++;
    }

    else if (stringData == sequence)
    {
      sectionData.concat(input[i]);
    }

    else if (stringData > sequence)
    {
      return sectionData;
      break;
    }
  }

  return sectionData;
}

void rainbowState(){
  
  //Läd Farbwerte auf die LEDs im Regenbogen Stil. rainbow_cycle_state wird fortlaufend angepasst, weshalb sich der Regenbogen "bewegt".
  fill_rainbow( leds1, NUM_LEDS, rainbow_cycle_state, 7);
  fill_rainbow( leds2, NUM_LEDS, rainbow_cycle_state, 7);
  FastLED.show();
  //Updaten vom rainbow_cycle_state
  rainbow_cycle_state++;
  if(rainbow_cycle_state == 255){
    rainbow_cycle_state = 0;
  }
  //Das delay bestimmt wie schnell sich die Farbwerte ändern sollen!
  delay(20);
}
