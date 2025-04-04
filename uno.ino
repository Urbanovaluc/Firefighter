/*
arduino uno nám sbíra data ze senzorů
ty senzory jsou levný, takže pokud všechny přestanou fungovat do prezentace nebudu překvapen
z una nám jde seriová linka rx,tx,gnd do esp zapojit (rx<->tx, tx<->rx, gnd(g)<->gnd(g))
dále je na uno napojeny 4 senzory mq2,mq9,dallas18b20,ten infrared detektor plamene, dohromady to bere něco do 400ma (uno dává safely do 400)
proto máme napájení(5v) a zem(gnd) společnou pro všechny senzory, piny na zapojení jsou popsaný více dolejš
na mq4 senzor zemního plynu jsme se vykašlali protože je to prakticky to samé jak mq2
*/

#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/** 
########################################################
senzory napojit na arduino uno na níže specifikované piny
#########################################################
*/ 

//plamenosenzor
#define analogPin A0 //analog pin
#define digitalPin 2 //dialog pin

//metanosenzor
const int aOut = A1; //analog pin
const int dOut = 3; // dialog pin

//oxidouhelnatosenzor
#define pinA A2 //analog pin
#define pinD 4 //dialog pin

//teploměrosenzor
#define pinCidlaDS 6 //prostřední pin na teploměru (ten u led diody je zem btw!!! špatné zapojení to může usmažit)

//#########################################################

const float RL_VALUE = 5.0; 
const float RO_CLEAN_AIR_FACTOR = 9.6;
float Ro = 10.0; 
SoftwareSerial mySerial(0, 1); // RX, TX
OneWire oneWire(pinCidlaDS);
DallasTemperature senzors(&oneWire);
float sensor_volt;//MQ9
float RS_gas;//MQ9
const float R0 = 1.62;//MQ9
float ratio;//MQ9
float sensorValue;//MQ9
const int sensorPin = A3; //MQ4
const int threshold = 500; //MQ4
float teplota ;
bool ohen = false;
bool plyn = false;
bool co = false;
float mq2V = 0;//MQ2
void setup() {
  Ro = calibrateMQ9Sensor();
  mySerial.begin(9600);
  pinMode(analogPin, INPUT);
  pinMode(digitalPin, INPUT);
  pinMode(dOut, INPUT);
  pinMode(aOut, INPUT);
  attachInterrupt(0, MQ2Plyn, FALLING);
  attachInterrupt(digitalPinToInterrupt(digitalPin), Ohen, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinD), MQ9Plyn, RISING);

}

void loop() {
  vypisHodnoty();
}

int InfraSenzor()
{
 
	int napetiSenzor = analogRead(analogPin);
  
	int prepocet = map(napetiSenzor, 0, 1024, 0, 80);
  //pokud to vidí oheň až moc daleko tak tam spíš není
  if(prepocet > 75)
  {
    prepocet = -1;
  }
  return prepocet;
}
void Ohen() {
  ohen = true
}
float MQ2()
{
 // načtení hodnoty analogového vstupu
  mq2V = map(analogRead(aOut), 0, 1023, 0, 1000);
  mq2V = mq2V/10;
  return mq2V;
}
bool MQ2Plyn(){
  plyn = true
}
float calibrateMQ9Sensor() {
  float RS_AIR = 0;
  int samples = 100;

  for (int i = 0; i < samples; i++) {
    RS_AIR += readMQ9();
    delay(100);
  }

  RS_AIR /= samples;
  return RS_AIR / RO_CLEAN_AIR_FACTOR;
}
float readMQ9() {
  int sensorValue = analogRead(pinA);
  float voltage = (sensorValue / 1023.0) * 5.0;  // Convert ADC value to voltage (assuming 5V supply)
  float RS = (5.0 - voltage) / voltage * RL_VALUE;  // Calculate sensor resistance
  return RS;
}
void MQ9Plyn() {
  co = true
}


 int MQ4()
 {
  int sensorValue = analogRead(sensorPin)/10000;
  return sensorValue;
 }
float Teplomer()
 {
  senzors.requestTemperatures();
  teplota = senzors.getTempCByIndex(0);
  return teplota;
 }
void vypisHodnoty() {
  ohen = false
  plyn = false
  co = false
  float infra = InfraSenzor();
  float mq2 = MQ2();
  float mq4 = MQ4();
  float Rs = readMQ9();
  float ratio =Rs / Ro;  // Calculate ratio of Rs/Ro
  
  // Calculate CO concentration in ppm using datasheet formula
  float mq9 = pow(10, ((-0.47 * log10(ratio)) + 1.32));
  teplota = Teplomer();
  //ohen = Ohen(); funkce je volana sama v attachinteruptu
  //plyn = MQ2Plyn(); funkce je volana sama v attachinteruptu
  //co = MQ9Plyn(); funkce je volana sama v attachinteruptu
  
  delay(2000); // Odesílání každé 2 sekundy

  mySerial.print(infra);
  mySerial.print(",");
    
  mySerial.print(mq2);
  mySerial.print(",");
  
  mySerial.print(mq4);
  mySerial.print(",");
  
  mySerial.print(mq9);
  mySerial.print(",");

  mySerial.print(teplota);
  mySerial.print(",");
  
  mySerial.print(ohen ? "1" : "0");
  mySerial.print(",");
    
  mySerial.print(plyn ? "1" : "0");
  mySerial.print(",");

  mySerial.println(co ? "1" : "0");



}