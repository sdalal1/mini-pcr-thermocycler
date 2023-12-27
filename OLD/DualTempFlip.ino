//thermocouple setup
#include "Adafruit_MAX31855.h"
//thermocouple0
#define MAXDO0   4
#define MAXCS0   5
#define MAXCLK0  6
Adafruit_MAX31855 thermocouple0(MAXCLK0, MAXCS0, MAXDO0);
//thermocouple1
#define MAXDO1   7
#define MAXCS1   8
#define MAXCLK1  9
Adafruit_MAX31855 thermocouple1(MAXCLK1, MAXCS1, MAXDO1);

int heater0 = 2,heater1 = 3;
//global variables for references
int cycles = 0;    //cycles completed
int target0, target1, T_high0, T_high1, T_low0, T_low1;
double threshold0, threshold1;

void setup() {
  //decalre pinmods
  pinMode(heater0,OUTPUT);
  pinMode(heater1,OUTPUT);
  //begin serial communication
  Serial.begin(9600);
  delay(500);
  threshold0 = thermocouple0.readCelsius(); //stores ramp limit
  threshold1 = thermocouple1.readCelsius();
}

void loop() {
  //USER PARAMETERS-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      T_high0 = 80;    //target high temp
      T_low0 = 30;    //target low temp
      T_high1 = 2;
      T_low1 = -2;
  int t = 60;          //segment run-time (minutes)
  int st = 30;         //sample rate (seconds)
  //do not change below this line-=-=-=-=-=-=-=-=-=-=


  
  int l = t*60/st;    //# loops in segment (minutes >> seconds)
  double inc0 = (double) (T_high0-T_low0)/(l); //ramp rate
  double inc1 = (double) (T_high1-T_low1)/(l);

  //runs a segment
  //warm
  Serial.println("Warming");
  for(int i=0; i < l; i++){
    //fucntion runs a heater and temp check
    //select heater
    //select high or low with coressponding value
    //corresponding threshold
    //corresponding increment
    threshold0 = seg(heater0,T_high0,threshold0,inc0);
    //threshold1 = seg(heater1,T_high1,threshold1,inc1);
    delay(st*1000);  //time between samples
  }
  //repeating a segment will hold at limit
  //hold warm
  Serial.println("Holding");
  for(int i=0; i < l; i++){
    threshold0 = seg(heater0,T_high0,threshold0,inc0);
    //threshold1 = seg(heater1,T_high1,threshold1,inc1);
    delay(st*1000);
  }
  //cool
  Serial.println("Cooling");
  digitalWrite(3,HIGH); //fan to pull in cold air
  for(int i=0; i < l; i++){
    threshold0 = seg(heater0,T_low0,threshold0,inc0*(-1));
    //threshold1 = seg(heater1,T_low1,threshold1,inc1*(-1));
    delay(st*1000);
  }
  //Turn off cooling if temperature is reached
  if(thermocouple0.readCelsius() <= T_low0){
    digitalWrite(3,LOW);
  }
  //hold cool
  Serial.println("Holding");
  for(int i=0; i < l; i++){
    threshold0 = seg(heater0,T_low0,threshold0,inc0*(-1));
    //threshold1 = seg(heater1,T_low1,threshold1,inc1*(-1));
    delay(st*1000);
  }
  digitalWrite(3,LOW); //cooling fan off

  cycles++;
}

//READ TEMPS & SEND TO PROCESSING
int readTemp(int s, double thresh){
  double T0 = thermocouple0.readCelsius();
  double T1 = thermocouple1.readCelsius();
  if(s == heater0) target0 = thresh;
  if(s == heater1) target1 = thresh;
  String Temps = String(target0) +","+ String(target1) +","+ String(T0) +","+ String(T1) +","+ String(cycles);
  Serial.println(Temps);      //display
  if(s == heater0) return T0;
  if(s == heater1) return T1;
}

//SEGMENT
/*Designed to apply to every segment. Special modifications
for individual segments would need to pull in a value to
indicate which state the segment is in, logic for applying
to that state, and then logic for that action.*/
double seg(int select, double limit, double thresh, double inc){
  float temp = readTemp(select,thresh);
  //checks if heat needs to be added to the system
  if(temp < thresh) digitalWrite(select,HIGH);
  else digitalWrite(select,LOW);
  //else statements ramp up each loop
  //if statements prevent overramp
  if(((limit == T_high0) || (limit == T_high1)) && (thresh >= limit)) thresh = limit;
  else if(((limit == T_low0) || (limit == T_low1)) && (thresh <= limit)) thresh = limit;
  else thresh += inc;
  return thresh;
}
