/*Program is to control temperature in two enclosures.
Objects are given a designation of 0 or 1 since only
two enclosures are required, so a binary system is the
simplist. Thermocouples measure the temperature in each
each enclosure, and relays control the heaters. Target
Temperatures are set in a parameters section. Ramps rates
are automatically calculated by the time and temps defined.

Modification from original have been made. Current use
is controlling the temperature in a single enclosure,
with one relay controlling the heater, and one controlling
a cooling fan.*/

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

//heater pins. Heater1 is now used to control a fan
int heater0 = 2,heater1 = 3;

//global variables for reference through functions
int cycles = 0;    //cycles completed counter
int target0, target1, T_high0, T_high1, T_low0, T_low1;
double threshold0, threshold1;  //ramped target values

void setup() {
  //declare pinmods
  pinMode(heater0,OUTPUT);
  pinMode(heater1,OUTPUT);
  //begin serial communication
  Serial.begin(2400);
  delay(500);
  threshold0 = thermocouple0.readCelsius(); //stores ramp limit
  threshold1 = thermocouple1.readCelsius();
  Serial.println(threshold1);
}

void loop() {
  //USER PARAMETERS-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      T_high0 = 150;    //target high temp
      T_low0 = 40;      //target low temp
      T_high1 = 1;
      T_low1 = -1;
  int t = 60;          //segment run-time (minutes)
  int st = 10;          //sample rate (seconds)
  //sample rate is currently hard coded into Processing
  //should be passed in with string
  //No inputs below this line-=-=-=-=-=-=-=-=-=-=

  //each segment will run with the number of loops calculated below
  int l = t*60/st;    //# loops in segment (minutes converted to seconds)
  int lmod = 60;      //modify soak times to 5 minutes
  //calculate ramp rate with incrementations
  //that add to the thresholds
  double inc0 = (double) (T_high0-T_low0)/(l);
  double inc1 = (double) (T_high1-T_low1)/(l);

  /*Runs a segment (ramp up, ramp down, or hold)
  Since segments are defined by a time and target
  temperatures, a hold can be done by a repeat of
  the a warm or cool segment.*/
  //warm
  //Serial.println("Warming");
  for(int i=0; i < l; i++){
    //fucntion will run a heater and temp check
    //select heater
    //select high or low target temp
    //corresponding threshold# (could be simplified)
    //corresponding increment#
    threshold0 = seg(heater0,T_high0,threshold0,inc0);
    //functions for second eclosure are all off
    //threshold1 = seg(heater1,T_high1,threshold1,inc1);
    delay(st*1000);  //time between samples
  }
  //repeating a segment will hold at limit
  //hold warm
  //Serial.println("Holding");
  for(int i=0; i < lmod; i++){
    threshold0 = seg(heater0,T_high0,threshold0,inc0);
    //threshold1 = seg(heater1,T_high1,threshold1,inc1);
    delay(st*1000);
  }

  /*all segments are heater controlled, excpet for the cooling
  segment that is regulated by fan. Increments are multiplied by
  by -1 to ramp down.*/
  digitalWrite(heater0,LOW);  //Ensure heater is off before fan control
  //cool
  //Serial.println("Cooling");
  for(int i=0; i < l; i++){
    threshold0 = seg2(heater1,T_low0,threshold0,inc0*(-1));
    //threshold1 = seg(heater1,T_low1,threshold1,inc1*(-1));
    delay(st*1000);
  }
  digitalWrite(heater1,LOW); //Ensure fan is off before resuming heating
  
  //hold cool
  //Serial.println("Holding");
  for(int i=0; i < lmod; i++){
    threshold0 = seg(heater0,T_low0,threshold0,inc0*(-1));
    //threshold1 = seg(heater1,T_low1,threshold1,inc1*(-1));
    delay(st*1000);
  }

  cycles++;
}

//READ TEMPS & SEND TO PROCESSING
int readTemp(int s, double thresh){
  double T0 = thermocouple0.readCelsius();
  double T1 = thermocouple1.readCelsius();
  if(s == heater0) target0 = thresh;
  if(s == heater1) target1 = thresh;
  String Temps = String(target0) +","+ String(target1) +","+ String(T0) +","+ String(T1) +","+ String(cycles);
  Serial.println(Temps);      //send to processing
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

/*Modification/patch of original function to allow for fan
control. Differs by the >< signs, and which thermocouple to read
from. Selecting heater1 in the function of the "cool" segment tells
the select variable to select thermocouple1.*/
double seg2(int select, double limit, double thresh, double inc){
  float temp = readTemp(2,thresh);  //hard coded to read thermocouple0
  //checks if heat needs to be added to the system
  if(temp > thresh) digitalWrite(select,HIGH);
  else digitalWrite(select,LOW);
  //else statements ramp up each loop
  //if statements prevent overramp
  if(((limit == T_high0) || (limit == T_high1)) && (thresh >= limit)) thresh = limit;
  else if(((limit == T_low0) || (limit == T_low1)) && (thresh <= limit)) thresh = limit;
  else thresh += inc;
  return thresh;
}
