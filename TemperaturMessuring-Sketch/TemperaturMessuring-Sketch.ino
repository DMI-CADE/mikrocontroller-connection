#define ThermistorPin0   0
#define ThermistorPin1   1
#define ThermistorPin2   2

int input0;
int input1;
int input2;

float T0;
float T1;
float T2;
double Intern;

float R1 = 10000;
float c1 = 1.009249522e-03; 
float c2 = 2.378405444e-04;
float c3 = 2.019202697e-07;

void setup() {
  Serial.begin(9600);
}

void loop() {

  input0 = analogRead(ThermistorPin0);
  T0 = transformInputToTemperatur(input0, 0);
  
  input1 = analogRead(ThermistorPin1);
  T1 = transformInputToTemperatur(input1, 1);
  
  input2 = analogRead(ThermistorPin2);
  T2 = transformInputToTemperatur(input2, 2);

  Intern = readInternalTemperature();
  
  Serial.print("Temperature0: ");
  Serial.print(T0);
  Serial.print(" C; ");
  Serial.print("Temperature1: ");
  Serial.print(T1);
  Serial.print(" C; ");
  Serial.print("Temperature2: ");
  Serial.print(T2);
  Serial.print(" C; ");
  Serial.print("Temperature Intern: ");
  Serial.print(Intern);
  Serial.println(" C; ");

  delay(3000);
}

float transformInputToTemperatur(int Input, int test){

  float logR2, R2, Temperature;
  
  R2 = R1 * (1023.0 / (float)Input - 1.0);
  logR2 = log(R2);
  Temperature = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Temperature = Temperature - 273.15;

  //Feintuning für den zweiten Temperatursensor. 
  //Dieser hat deutlich dickere Kabel, was den Widerstand erhöht und dadurch das Ergebnis leicht geringer ist, als bei den beiden anderen Sensoren.
  if(test == 1){
    Temperature = Temperature + 0.7;
  }
  
  return Temperature;
}

//Folgende Funktion stammt von Amisha aus dieser Quelle: https://www.engineersgallery.com/arduino-internal-temperature-sensor/
//Zuletzt abgerufen am 23.03.2022
double readInternalTemperature(){
  
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return (t);
}
