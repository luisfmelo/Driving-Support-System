
/* Tiago Miguel da Silva Tavares SEAI 2016/2017
  ----------------------       ----------------------  ----------------------
*/
// Temperatura 

#include "cactus_io_AM2302.h"
#define AM2302_PIN 38    // what pin on the arduino is the DHT22 data line connected to
AM2302 dht(AM2302_PIN);
float temperatura = 0;
float humidade = 0;

//  Batimento Cardiaco
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 5;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

//sensor de pressao
int SensorL = A2;
int conta_pressao = 0;
int threshold = 992; //valor definido por nos, valor normal da pressao
int soma_pressao = 0;
int pressao_gradualmente = 0;

//piscas

int buttonEsq = A14;
int buttonDir = A15;
int buttonStateEsq = 0;
int buttonStateDir = 0;

int piscaDir = 22;
int piscaEsq = 26;
unsigned long timeEsq = 0;
unsigned long timeDir = 0;



//velocidade
int vel = A6;
int velocidade = 0;

//throttle position
int throt = A8;
int threshold_Pedal = 400;
int conta_pedal = 0;
int soma_pedal = 0;
int gradualmente_pedal = 0;

//LED
#define REDPIN 6
#define GREENPIN 5
#define BLUEPIN 11

//Sensor de chuva
int chuva = A12;
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum

//sensor pressao batimento

int sensor_batimento_presso = A4;
int bate_bate = 0;


int vetor[8];
int a = 0;
int soma = 0;
int conta = 0;
int somaperfil = 0;
int r, g, b;
int perfil = 0;
int stressado = 0;
#define FADESPEED   1  // make this higher to slow down

int batimento = 0;
int ataque = 0;

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
boolean QS = false;        // becomes true when Arduoino finds a beat.

// Regards Serial OutPut  -- Set This Up to your needs
static boolean serialVisual = false;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse


void setup() {
  pinMode(REDPIN, OUTPUT);          //DEFINIR PINS DOS LEDS
  pinMode(GREENPIN, OUTPUT);        //DEFINIR PINS DOS LEDS
  pinMode(BLUEPIN, OUTPUT);         //DEFINIR PINS DOS LEDS
  pinMode(blinkPin, OUTPUT);        // pin that will blink to your heartbeat!
  pinMode(fadePin, OUTPUT);         // pin that will fade to your heartbeat!
  pinMode(buttonEsq, INPUT_PULLUP);
  pinMode(buttonDir, INPUT_PULLUP);
  pinMode(piscaDir, OUTPUT);
  pinMode(piscaEsq, OUTPUT);
  Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS
  dht.begin();

  for (r = 0; r < 256; r++) {
    analogWrite(REDPIN, r);
    delay(FADESPEED);
  }

}
void loop() {

  //serialOutput() ;

  dht.readTemperature();
  dht.readHumidity();
  /*if (isnan(dht.humidity)) {
    Serial.println("DHT sensor read failure!");
    return;
    }*/

  vetor[2] = analogRead(SensorL);
  vetor[3] = analogRead(vel);
  vetor[4] = analogRead(throt);
  vetor[5] = analogRead(chuva);




  /*if (SenvalL <= 30 && SenvalR <= 30)
    Serial.println("sem maos");

    if ((SenvalL > 0 && SenvalR <= 30) || (SenvalL <= 30 && SenvalR > 0)  )
    Serial.println("Apenas uma mao no volante");
  */
  //ledFadeToBeat();                      // Makes the LED Fade Effect Happen
  //delay(50);                            //  take a break
  if (QS == true) {    // A Heartbeat Was Found
    // BPM and IBI have been Determined
    // Quantified Self "QS" true when arduino finds a heartbeat
    fadeRate = 255;         // Makes the LED Fade Effect Happen
    // Set 'fadeRate' Variable to 255 to fade LED with pulse
    serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.
    QS = false;                      // reset the Quantified Self flag for next time
  }


  /* Temperatura

    ----------------------------------------------------------------------
  */
  Serial.print("-");
  if ((dht.temperature_C >= 0) && (dht.temperature_C < 10)) {
    Serial.print("T+0");  //temperatura
    Serial.print(dht.temperature_C, 0);
    Serial.print("-");
  }
  if (dht.temperature_C >= 10) {
    Serial.print("T+");  //temperatura
    Serial.print(dht.temperature_C, 0);
    Serial.print("-");
  }


  if (dht.temperature_C < 0) {
    Serial.print("Tn0");  //temperatura
    temperatura = abs(dht.temperature_C); //testar isto
    Serial.print(temperatura, 0);
    Serial.print("-");
  }



  /* humidade

      ----------------------------------------------------------------------
  */

  if ((dht.humidity >= 0) && (dht.humidity < 10)) {
    Serial.print("H0");  //temperatura
    Serial.print(dht.humidity, 0);
    Serial.print("-");
  }
  if (dht.humidity >= 10) {
    Serial.print("H");  //temperatura
    Serial.print(dht.humidity, 0);
    Serial.print("-");
  }



  /* Pressao volante

    ----------------------------------------------------------------------
  */

  if (vetor[2] < threshold) {
    conta_pressao++;
    threshold = vetor[2];
  }
  else
  {
    conta_pressao = 0;
    threshold = 992;
  }
  // Serial.println(conta_pressao);
  if (conta_pressao == 3) {
    soma_pressao++;
    if (soma_pressao == 4) {
      pressao_gradualmente = 1; //variavel para determinar que esta a perder pressap
      soma_pressao = 0;
    }
  }

  if (vetor[2] < 940 && pressao_gradualmente == 0) { //SEM MAOS
    Serial.print("V0");  //
    Serial.print("-");
  }
  if (vetor[2] >= 940 && pressao_gradualmente == 0) {
    Serial.print("V1");  //com maos
    Serial.print("-");
  }
  if (pressao_gradualmente == 1) {
    Serial.print("V2");    //TIRA GRADUALMENTE
    Serial.print("-");
    pressao_gradualmente = 0;

  }

  /* Velocidade

    ----------------------------------------------------------------------
  */



  Serial.print("S");  //velocidade
  velocidade = vetor[3] / 3.41;
  if (velocidade > 0 && velocidade < 10)
  {
    Serial.print("00");
    Serial.print(velocidade);
    Serial.print("-");

  }
  if (velocidade >= 10 && velocidade <= 99)
  {
    Serial.print("0");
    Serial.print(velocidade);
    Serial.print("-");

  }
  if (velocidade > 99)
  {
    Serial.print(velocidade);
    Serial.print("-");

  }

  /* Pressao pedal

    ----------------------------------------------------------------------
  */
  if (vetor[4] < threshold_Pedal) {
    conta_pedal++;
    threshold_Pedal = vetor[4];

  }
  else
  {
    conta_pedal = 0;
    threshold_Pedal = 400;
  }
  // Serial.println(conta_pressao);
  if (conta_pedal == 3) {
    soma_pedal++;
    if (soma_pedal == 4) {
      gradualmente_pedal = 1;

      soma_pedal = 0;
    }
  }

  if (vetor[4] < 50 && gradualmente_pedal == 0) {
    Serial.print("P2");  //sem pressao
    Serial.print("-");

  }
  else if (vetor[4] >= 800 && gradualmente_pedal == 0) {
    Serial.print("P1");  //totalmente pressionado
    Serial.print("-");

  }

  else if (gradualmente_pedal == 1) {
    Serial.print("P0");  //totalmente pressionado
    Serial.print("-");
    gradualmente_pedal = 0;

  }
  else {
    Serial.print("Px");
    Serial.print("-");  //totalmente pressionado
  }


  /* Pluviosidade

    -----------------------------------------------------------------------
  */



  if (vetor[5] < 500) {
    Serial.print("Rr");//chuva
    Serial.print("-");
  }
  else {
    Serial.print("Rs");//nao chuva
    Serial.print("-");
  }


  /* PISCAS

    -----------------------------------------------------------------------
  */

  buttonStateEsq  = analogRead(buttonEsq);
  buttonStateDir = analogRead(buttonDir);

  if (buttonStateEsq < 800) {
    Serial.print("Id-");
    if ( millis() - timeEsq >= 1000)
    {
      digitalWrite(piscaDir ,HIGH);   // turn the LED on (HIGH is the voltage level)
      timeEsq = millis();
    }
    else if ( millis() - timeEsq >= 500)
      digitalWrite(piscaDir, LOW);    // turn the LED off by making the voltage LOW

  }
  else if (buttonStateDir < 800) {
    Serial.print("Ie-");
    if ( millis() - timeDir >= 1000)
    {
      digitalWrite(piscaEsq ,HIGH);   // turn the LED on (HIGH is the voltage level)
      timeDir = millis();
    }
    else if ( millis() - timeDir >= 500)
      digitalWrite(piscaEsq, LOW);    // turn the LED off by making the voltage LOW
  }

  else {
    Serial.print("Ix-");
    digitalWrite(piscaDir, LOW); 
     digitalWrite(piscaEsq, LOW);
  }


  /* Batimento Cardiaco

      -----------------------------------------------------------------------
  */
  if (ataque == 1) {
    Serial.print("B1"); //estado critico
  }
  else {
    Serial.print("B0"); //batimento normal
  }
  Serial.println(".");

  delay(100);
}





void ledFadeToBeat() {
  fadeRate -= 15;                         //  set LED fade value
  fadeRate = constrain(fadeRate, 0, 255); //  keep LED fade value from going into negative numbers!
  analogWrite(fadePin, fadeRate);         //  fade LED
}



/*void serialOutput(){   // Decide How To Output Serial.
  if (serialVisual == true){
     arduinoSerialMonitorVisual('-', Signal);   // goes to function that makes Serial Monitor Visualizer
  } else{
      sendDataToSerial('S', Signal);     // goes to sendDataToSerial function
  }
  }
*/

//  Decides How To OutPut BPM and IBI Data
void serialOutputWhenBeatHappens() {
  /*if (serialVisual == true){            //  Code to Make the Serial Monitor Visualizer Work
       Serial.print("*** Heart-Beat Happened *** ");  //ASCII Art Madness
       Serial.print("BPM: ");
       Serial.print(BPM);
       Serial.print("  ");
      } else{*/
  if (a == 15) {
    somaperfil = somaperfil + soma;
    soma = soma / 15;
    Serial.println(" ");
    sendDataToSerial('B', soma);  // send heart rate with a 'B' prefix

    Serial.println(" ");
    Serial.println("comeca a contar ate 15: ");
    conta++;

    if (conta == 4)
    {
      Serial.println("Perfil ativo");
      perfil = somaperfil / 60;
      Serial.println(perfil);
      if (perfil > 130) {  //serve para ver se ha algum erro, se houver cria um novo perfil
        Serial.println("ERRO!! Tenta outra vez");
        perfil = 0;
        conta = 0;
        somaperfil = 0;
      }

    }
    if (conta > 4 && soma < 200)
    {
      ataque = 1;

      batimento = soma;
      if ((soma > 70 && soma < 180) || (soma < 40))
      {
        ataque = 1;
      }
      ataque = 0;

      if (soma > 50 && soma < 150 && perfil > 50)
      {
        stressado = 1;
        for (g = 100; g > 20; g--) {
          // Serial.println(g);
          analogWrite(GREENPIN, g);
          delay(FADESPEED);
        }
        Serial.println("azul");
        for (b = 0; b < 180; b++) {
          //Serial.println(b);
          analogWrite(BLUEPIN, b);
          delay(FADESPEED);
        }
        delay(2000);
      }

      if (soma > 60 && soma < 150 && perfil > 60 && stressado == 1)
      {
        for (g = 100; g > 20; g--) {
          // Serial.println(g);
          analogWrite(GREENPIN, g);
          delay(FADESPEED);
        }
        Serial.println("azul");
        for (b = 0; b < 180; b++) {
          analogWrite(BLUEPIN, b);
          delay(FADESPEED);
        }
        Serial.println("diminuiu azul");
        for (b = 200; b > 0; b--) {
          analogWrite(BLUEPIN, b);
          delay(FADESPEED);
        }
        stressado = 0;
      }



    }

    a = 0;


  }

  bate_bate = analogRead(sensor_batimento_presso);
  //Serial.println(a);
  if (bate_bate >= 50) {
    a++;
  }

  soma = soma + BPM;
}



//  Sends Data to Pulse Sensor Processing App, Native Mac App, or Third-party Serial Readers.
void sendDataToSerial(char symbol, int data ) {
  Serial.print(symbol);

  Serial.println(data);
}
