#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define DATA_PIN1     3
#define DATA_PIN2     5

#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

//LEDs in Reihe pro Player
#define NUM_LEDS      12
//Es können maximal 12 HEX Werte übergeben werden, einen pro Button
#define MAX_INPUT_STREAM_VALUES   12

#define BRIGHTNESS         150
#define FRAMES_PER_SECOND  120

//Doku: http://fastled.io/docs/3.1/index.html
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

//In den nachfolgenden Arrays werden die übergebenden Werte als String abgespeichert. (12 LEDs pro Player)
String player1[NUM_LEDS];
String player2[NUM_LEDS];

//TODO kann eventuell ersetzt werden!
//counter wird verwendet um die maximal 6 Input Werte auf das Array von 12 LEDs zu mappen.
int counter = 0;

//TODO wofür sind die beiden Variablen?
//Muss wahrscheinlich nicht global deklariert werden!
int stringData;
String sectionData;

//TODO bessere Doku!
//Rainbow Modul ist sowas wie ein Idle State, wenn die Arcade z.B. im Menü ist
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

  delay(500); // 0,5 Sekunden delay um sicherzustellen, dass die Daten korrekt übernommen wurden. (optional)

  /*
   * Zum Testen können inital einfach einmal alle LEDs auf Rot gesetzt werden:
   * for(int i = 0; i < NUM_LEDS; i++){
   *  leds[i] = CRGB::Red;
   * }
   * FastLED.show();
   */

  //Starten des seriellen Ports mit 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
    ; //Warten bis ein serieller Port verbunden ist.
  }

  //Test der Seriellen Verbindung. Kann noch entfernt werden.
  byte test = 0xFF;
  Serial.println(test, HEX);
}

void loop() {
  //Beispieleingaben
  //000000;FF00FF;FFFFFF;00FFFF;00FF00;0000FF;000000;FF00FF;FFFFFF;00FFFF;00FF00;0000FF;

  //Soll der Array gelöscht werden oder alle Werte auf Schwarz gesetzt werden:
  //memset(buttonArray, 0x000000, sizeof(player1));

  //Solange der serielle Port Daten im Input erkennt wird diese Schleife ausgeführt.
  while (Serial.available() > 0) {

    //Wenn ein valider String empfangen wird, soll dieser eingelesen werden.
    String input = Serial.readString();

    //Testen, ob Input valide ist:
    Serial.println("Anzahl Zeichen: " +input.length());

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
    }
  }

  //Prüft in der loop(), ob rainbow aktiv sein soll.
  if(rainbow){
    rainbowState();
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
    String buttonColor = getData(input, ';', i);
  
    if(buttonColor != ""){

      if(i <= MAX_INPUT_STREAM_VALUES / 2){
        
        //Überschreibt den Wert im player1 Array an der Stelle des Counters und auch die nachfolgende Stelle.
        //Zum Ende wird der Counter dann um 2 erhöht für das nächste LED Tuple.
        player1[counter] = buttonColor;
        player1[counter+1] = buttonColor;
        counter = counter +2;

        //Wenn die ersten Inputwerte erfolgreich dem ersten Spieler zugewiesen wurden muss der Counter zurück gesetzt werden.
        if(counter >= NUM_LEDS){
          counter = 0;
        }
        
      }else{
        
        //Überschreibt den Wert im player2 Array an der Stelle des Counters und auch die nachfolgende Stelle.
        //Zum Ende wird der Counter dann um 2 erhöht für das nächste LED Tuple.
        player2[counter] = buttonColor;
        player2[counter+1] = buttonColor;
        counter = counter +2;
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
