// DHT22 Setup
#include <DHT.h> // Sensor de temperature e umidade relativa do ar
DHT dht(6, DHT22); // Porta Digital 6 para leitura de temperatura e umidade relativa

#include <SPI.h> //Cartao de memoria externo
#include <SD.h> //Cartao de memoria externo

File myFile; //Cartao de memoria externo

#include <Servo.h> //S Servo motor
#define SERVO 3 // Porta Digital 3 PWM, acionamento do servo para controlar a comporta 

#include <HX711.h> //Amplificador do conjunto de celulas de carga
HX711 balanca(A3, A4); // // Portas para leitura referente as celulas de carga
//HX711.DOUT - pin #A3
//HX711.PD_SCK    – pin #A4
 
Servo s; // Variável Servo

float * vbal = new float[30]; // Vetor que armazena os dados de MAP
float * vcap = new float[30]; // Vetor que armazena os dados de capacitancia
float cap;
float sumbal; //Variavel de estatistica
float sumcap; //Variavel de estatistica
float avgbal; //Variavel de estatistica
float avgcap; //Variavel de estatistica
float avg2bal; //Variavel de estatistica
float avg2cap; //Variavel de estatistica
float stdbal; //Variavel de estatistica
float stdcap; //Variavel de estatistica
float t; //Variavel de estatistica
float uboundbal; //Variavel de estatistica
float lboundbal; //Variavel de estatistica
float uboundcap; //Variavel de estatistica
float lboundcap; //Variavel de estatistica
int i;
float CVbal; //Variavel de estatistica
float CVcap; //Variavel de estatistica
float temp; //Temperatura
float URA; //Umidade relativa do ar

const int OUT_PIN = A2;
const int IN_PIN = A0;
const float IN_STRAY_CAP_TO_GND = 24.48; // Valor empirico de calibracao
const float IN_CAP_TO_GND  = IN_STRAY_CAP_TO_GND;
const float R_PULLUP = 34.8; //Resistencia interna PULLUP do chip do Arduino
const int MAX_ADC_VALUE = 1023;

void setup()
{
      
  Serial.begin(9600);
  Serial.println("Starting the calibration");

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
   
  s.attach(SERVO);
  s.write(50); // Fecha a comporta
  delay(15);
  
  balanca.set_scale(133.f);  // Valor empirico de calibracao
  balanca.tare(); // Zerar as medicoes da celulas de carga
  for(i=0; i<40; i++){ //Tempo para o usuario alimentar o sistema
    Serial.print("Time left to fill up ");
    Serial.print(40-i);
    Serial.println(" s");
    delay(1000);
  }
    
  for(i=1; i<31;i++){ //Realiza 30 leituras referete a celula de carga e capacitor
    
      pinMode(OUT_PIN, OUTPUT); //Capacitancia
      pinMode(IN_PIN, OUTPUT);

      delay(100);

      pinMode(IN_PIN, INPUT);
      digitalWrite(OUT_PIN, HIGH); //Carrega o capacitor
      int val = analogRead(IN_PIN);
      digitalWrite(OUT_PIN, LOW);
    
      if (val < 1000)
        {
          pinMode(IN_PIN, OUTPUT);
    
          float capacitance = (float)val * IN_CAP_TO_GND / (float)(MAX_ADC_VALUE - val);
          cap = capacitance;
        }
      else
        {
          pinMode(IN_PIN, OUTPUT);
          delay(1);
          pinMode(OUT_PIN, INPUT_PULLUP); //Ativa a resistencia interna do Arduino
          unsigned long u1 = micros(); //Inicializa a contagem do tempo para descarregar o capacitor
          unsigned long t;
          int digVal;
    
          do
          {
            digVal = digitalRead(OUT_PIN); //Descarrega o capacitor
            unsigned long u2 = micros();
            t = u2 > u1 ? u2 - u1 : u1 - u2;
          } while ((digVal < 1) && (t < 400000L));
    
          pinMode(OUT_PIN, INPUT);  
          val = analogRead(OUT_PIN);
          digitalWrite(IN_PIN, HIGH);
          int dischargeTime = (int)(t / 1000L) * 5;
          delay(dischargeTime);   
          pinMode(OUT_PIN, OUTPUT);  
          digitalWrite(OUT_PIN, LOW);
          digitalWrite(IN_PIN, LOW);
    
          float capacitance = -(float)t / R_PULLUP
                                  / log(1.0 - (float)val / (float)MAX_ADC_VALUE);
    
          if (capacitance > 1000.0)
          {
            cap = capacitance*1000;
          }
          else
          {
            cap = capacitance;
          }
        }
        
          vbal[i] = balanca .get_units(); //Armazena a leitura das celulas de carga
          vcap[i] = cap; //Armazena a leitura do capacitor
          Serial.print("Reading...");
          Serial.println(i);
          while (millis() % 1000 != 0)
          ;
        }
         //Estatistica - Media, Desvio Padrao, Coeficiente de Variacao e Intervalo de Confianca
sumbal = 0;
sumcap = 0;
avgbal = 0;
avgcap = 0;
avg2bal = 0;
avg2cap = 0;
t = 2.045;
        
for (i=1; i<31;i++){
    sumbal = sumbal+vbal[i];
    sumcap = sumcap+vcap[i];
  }
  avgbal = sumbal/30;
  avgcap = sumcap/30; 
for(i=1; i<31;i++){
    avg2bal = avg2bal+(vbal[i]-avgbal)*(vbal[i]-avgbal);
    avg2cap = avg2cap+(vcap[i]-avgcap)*(vcap[i]-avgcap);
  }
  avg2bal = avg2bal/(30-1);
  avg2cap = avg2cap/(30-1);
  stdbal = sqrt(avg2bal);
  stdcap = sqrt(avg2cap);
  CVbal = abs(stdbal/avgbal)*100;
  CVcap = abs(stdcap/avgcap)*100;
  uboundbal = avgbal+t*stdbal/sqrt(30);
  lboundbal = avgbal-t*stdbal/sqrt(30);
  uboundcap = avgcap+t*stdcap/sqrt(30);
  lboundcap = avgcap-t*stdcap/sqrt(30);
  s.write(125);
  delay(5);
  delay(5000); 
  temp = dht.readTemperature();
  URA = dht.readHumidity();

//Informa os valores medidos ao usuario por meio do Serial Monitor

  
for (i=1; i<31;i++){
    Serial.print("Bulk Density,");
    Serial.println(vbal[i],2);
    delay(1);
    }
    Serial.print("Mean ");
    Serial.println(avgbal,2);
    Serial.print("Standard Deviation ");
    Serial.println(stdbal,2);
    Serial.print("CV (%)");
    Serial.println(CVbal,2);
    Serial.print("Confidence Interval ");
    Serial.print(lboundbal,2);
    Serial.print(" - ");
    Serial.println(uboundbal,2);
for(i=1; i<31;i++){
    Serial.print("Capacitance,");
    Serial.println(vcap[i],2);
    delay(1);
    }
    Serial.print("Mean ");
    Serial.println(avgcap,2);
    Serial.print("Standard Deviation ");
    Serial.println(stdcap,2);
    Serial.print("CV (%)");
    Serial.println(CVcap,2);
    Serial.print("Confidence Interval ");
    Serial.print(lboundcap,2);
    Serial.print(" - ");
    Serial.println(uboundcap,2);
    Serial.println("Temperature ");
    Serial.println(temp,2);

//Grava os dados no cartao de memoria externo
        
myFile = SD.open("test.txt", FILE_WRITE); 
myFile.println("BD,Std,LB,UB,MC,Std,LB,UB,T");
myFile.print(avgbal,2);
myFile.print(",");
myFile.print(stdbal,2);
myFile.print(",");
myFile.print(lboundbal,2);
myFile.print(",");
myFile.print(uboundbal,2);
myFile.print(",");
myFile.print(avgcap,2);
myFile.print(",");
myFile.print(stdcap,2);
myFile.print(",");
myFile.print(lboundcap,2);
myFile.print(",");
myFile.print(uboundcap,2);
myFile.print(",");
myFile.println(temp,2);
myFile.close();
}

void loop(){
//
}
