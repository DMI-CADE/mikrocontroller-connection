#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
//LEDs in einer Kette
#define NUM_LEDS    18
//LEDs für einen Spieler (6 Button mit jeweils 2 LEDs)
#define LEDS_PLAYER    12
//Es können maximal 6 HEX Werte übergeben werden
#define MAX_INPUT_STREAM_VALUES    6

//Mögliche Farben von CRGB: http://fastled.io/docs/3.1/struct_c_r_g_b.html
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         250
#define FRAMES_PER_SECOND  120

//TODO Muss noch an die aktuelle Anzahl von LEDs angepasst werden!
String buttonArray[12];
//Counter wird verwendet um die maximal 6 Input Werte auf das Array von 12 LEDs zu mappen.
int counter = 0;
int stringData;
String sectionData;

//Rainbow Modul ist sowas wie ein Idle State, wenn die Arcade z.B. im Menü ist
bool rainbow = false;
int rainbow_cycle_state = 0;

void setup() {
  delay(500); // 0,5 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  delay(500); // 0,5 second delay for recovery

  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Red;
  }
  FastLED.show();

  //Starten des seriellen Ports mit 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
    ; //Warten bis ein serieller Port verbunden ist.
  }

  //Test der Seriellen Verbindung. Kann noch entfernt werden.
  byte test = 0xFF;
  Serial.println(test, HEX);

  //Verbaute LED des Arduino zum Testen verwenden
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  //Beispieleingaben
  //000000;FF00FF;FFFFFF;00FFFF;00FF00;0000FF;

  //Soll der Array gelöscht werden oder alle Werte auf Schwarz gesetzt werden:
  //memset(buttonArray, 0x000000, sizeof(buttonArray));
  
  while (Serial.available() > 0) {

    //Wenn ein valider String empfangen wird, soll dieser eingelesen werden.
    String input = Serial.readString();

    if(input == "rainbow;\n"){
      
      rainbow = true;
      
    }else{
      
      rainbow = false;

      for(int i = 0; i < MAX_INPUT_STREAM_VALUES; i++){

        //Sucht in dem String nach dem Trennzeichen. i ist der i-te Teilstring, den getData() zurück geben soll.
        String buttonColor = getData(input, ';', i);
  
        if(buttonColor != ""){

          //Überschreibt den Wert im buttonArray an der Stelle des Counters und auch das nachfolgende.
          //Zum Ende wird der Counter dann um 2 erhöht für das nächste LED Tuple.
          buttonArray[counter] = buttonColor;
          buttonArray[counter+1] = buttonColor;
          Serial.println("Array: " + buttonArray[counter]);
          Serial.println("Array: " + buttonArray[counter+1]);
          counter = counter +2;
        }
      }

      //Nach dem alle (max 6) Werte eingelesen und abgespeichert wurden, wird der counter zurückgesetzt.
      counter = 0;

      //Nun wird einmal über das buttonArray itteriert.
      for (int i = 0; i < LEDS_PLAYER; i++){

        //Die String Werte müssen noch als Zahlen gecastet werden. Dafür diese magische Funktion
        long strToByte = strtol(&(buttonArray[i])[0], NULL, 16);
        //Farbwert der LED an Stelle i setzten.
        leds[i] = strToByte;
      }

      //Zum Schluss noch die neuen Farbwerte auf die LEDs laden.
      FastLED.show();
    }
  }

  //Prüft in der loop(), ob rainbow aktiv sein soll.
  if(rainbow){

      //Läd Farbwerte auf die LEDs im Regenbogen Stil. rainbow_cycle_state wird fortlaufend angepasst, weshalb sich der Regenbogen "bewegt".
      fill_rainbow( leds, NUM_LEDS, rainbow_cycle_state, 7);
      FastLED.show();
      //Updaten vom rainbow_cycle_state
      rainbow_cycle_state++;
      if(rainbow_cycle_state == 255){
        rainbow_cycle_state = 0;
      }
      //Das delay bestimmt wie schnell sich die Farbwerte ändern sollen!
      delay(20);
  }
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
