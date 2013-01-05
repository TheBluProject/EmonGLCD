//------------------------------------------------------------------------------------------------------------------------------------------------
// emonGLCD HEM plus Funky/DHT22 monitor example - compatiable with Arduino 1.0 
// based on SolarPV template
// emonGLCD documentation http://openEnergyMonitor.org/emon/emonglcd

// For use with emonTx setup with CT 1 monitoring consumption, additional Dallas temp sensor and Funky remote node with DHT22 Temp/humidity sensor installed.

// Correct time is updated via RaspbPi which gets time from internet, this is used to reset Kwh/d counters at midnight. 

// Temperature recorded on the emonglcd is also sent to the base station for online graphing


// GLCD library by Jean-Claude Wippler: JeeLabs.org
// 2010-05-28 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
//
// History page by vworp https://github.com/vworp
//
// Authors: Glyn Hudson and Trystan Lea
// Part of the: openenergymonitor.org project
// Licenced under GNU GPL V3
// http://openenergymonitor.org/emon/license

// Libraries in the standard arduino libraries folder:
//
//	- OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
//	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature
//                           or https://github.com/milesburton/Arduino-Temperature-Control-Library
//	- JeeLib		https://github.com/jcw/jeelib
//	- RTClib		https://github.com/jcw/rtclib
//	- GLCD_ST7565		https://github.com/jcw/glcdlib
//
// Other files in project directory (should appear in the arduino tabs above)
//	- icons.ino
//	- templates.ino
//
//-------------------------------------------------------------------------------------------------------------------------------------------------

#include <JeeLib.h>
#include <GLCD_ST7565.h>
#include <avr/pgmspace.h>
#include <math.h>
GLCD_ST7565 glcd;

#include <OneWire.h>		    // http://www.pjrc.com/teensy/td_libs_OneWire.html
#include <DallasTemperature.h>      // http://download.milesburton.com/Arduino/MaximTemperature/ (3.7.2 Beta needed for Arduino 1.0)
#include <RTClib.h>                 // Real time clock (RTC) - used for software RTC to reset kWh counters at midnight
#include <Wire.h>                   // Part of Arduino libraries - needed for RTClib
RTC_Millis RTC;

#define RETRY_PERIOD 5    // How soon to retry (*10ms) if ACK didn't come in
#define RETRY_LIMIT 5     // Maximum number of times to retry
#define ACK_TIME 10       // Number of milliseconds to wait for an ack

//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 20            // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210            // network group, must be same as emonTx and emonBase

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------
typedef struct { int power1, power2, power3, Vrms, temp; } PayloadTX;         // neat way of packaging data for RF comms
PayloadTX emontx;

typedef struct { int temperature; } PayloadGLCD;
PayloadGLCD emonglcd;

typedef struct { int hour, mins, sec; } PayloadBase;			// new payload def for time data reception
PayloadBase emonbase; 

typedef struct { int temperature, humidity; } PayloadFunky;
PayloadFunky emonfunky;

//---------------------------------------------------
// emonGLCD SETUP
//---------------------------------------------------
//#define emonGLCDV1.3               // un-comment if using older V1.3 emonGLCD PCB - enables required internal pull up resistors. Not needed for V1.4 onwards 

const int greenLED=6;               // Green tri-color LED
const int redLED=9;                 // Red tri-color LED
const int LDRpin=4;    		    // analog pin of onboard lightsensor 
const int switch1=15;               // Push switch digital pins (active low for V1.3, active high for V1.4)
const int switch2=16;
const int switch3=19;

//---------------------------------------------------
// emonGLCD variables 
//---------------------------------------------------
int hour = 0, minute = 0;
double usekwh = 0;
double otemp, humi, dewpoint = 0, etemp; 
double minotemp = 99, maxotemp = -99, minhumi = 99, maxhumi = 0, minetemp = 99, maxetemp = 0, batt, dpcalc; //Dirty half-working hack to avoid min[x]temp = 0 all the time (well, until the next ice age, that is ..)
double use_history[7];
int cval_use;
byte page = 1;


//---------------------------------------------------
// Temperature Sensor Setup
//---------------------------------------------------
#define ONE_WIRE_BUS 5              // temperature sensor connection - hard wired 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
double temp, maxtemp,mintemp;



//-------------------------------------------------------------------------------------------- 
// Flow control
//-------------------------------------------------------------------------------------------- 
unsigned long last_emontx;                   // Used to count time from last emontx update
unsigned long last_emonbase;                   // Used to count time from last emontx update
boolean last_switch_state, switch_state; 
unsigned long fast_update, slow_update;

//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  rf12_initialize(MYNODE, freq,group);
  glcd.begin(0x20);
  glcd.backLight(200);
  
  sensors.begin();                         // start up the DS18B20 temp sensor onboard  
  sensors.requestTemperatures();
  temp = (sensors.getTempCByIndex(0));     // get inital temperture reading
  mintemp = temp; maxtemp = temp;          // reset min and max

  pinMode(greenLED, OUTPUT); 
  pinMode(redLED, OUTPUT); 
  
  #ifdef emonGLCDV1.3                      //enable internal pull up resistors for push switches on emonGLCD V1.3 (old) 
  pinMode(switch1, INPUT); pinMode(switch2, INPUT); pinMode(switch2, INPUT);
  digitalWrite(switch1, HIGH); digitalWrite(switch2, HIGH); digitalWrite(switch3, HIGH); 
  #endif
}

//--------------------------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------------------------
void loop()
{
  
  if (rf12_recvDone())
  {
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
    {
      int node_id = (rf12_hdr & 0x1F);
      if (node_id == 10) {emontx = *(PayloadTX*) rf12_data; last_emontx = millis();}
      if (node_id == 18) {emonfunky = *(PayloadFunky*) rf12_data ;}
      
      if (node_id == 15)
      {
       emonbase = *(PayloadBase*) rf12_data;                           
       RTC.adjust(DateTime(2012, 1, 1, emonbase.hour, emonbase.mins, emonbase.sec));
       last_emonbase = millis(); 
      } 
    }
  }
    otemp = double (emonfunky.temperature * 0.01);
    humi = double (emonfunky.humidity * 0.01);
    etemp = double (emontx.temp * 0.01);
    
  //--------------------------------------------------------------------------------------------
  // Display update every 200ms
  //--------------------------------------------------------------------------------------------
  if ((millis()-fast_update)>200)
  {
    fast_update = millis();
    
    DateTime now = RTC.now();
    int last_hour = hour;
    hour = now.hour();
    minute = now.minute();

    usekwh += (emontx.power1 * 0.2) / 3600000;
    batt = emontx.Vrms;
    if (last_hour == 23 && hour == 00) 
    { 
      int i; for(i=6; i>0; i--) use_history[i] = use_history[i-1];
      usekwh = 0;  
    }
    use_history[0] = usekwh;
    cval_use = cval_use + (emontx.power1 - cval_use)*0.50;
   
    last_switch_state = switch_state;
    switch_state = digitalRead(switch1);  
    if (!last_switch_state && switch_state) { page += 1; if (page>2) page = 1; }
    
    //Dew point calculation
    dpcalc = (log10(humi)-2.0)/0.4343+(17.62*otemp)/(243.12+otemp);
    dewpoint = 243.12*dpcalc/(17.62-dpcalc);
    
   
    if (page==1)
    {            
      draw_dash_page(cval_use, usekwh, humi, otemp, minotemp, maxotemp, dewpoint, temp, mintemp, maxtemp, hour, minute, batt, last_emontx, last_emonbase);
      glcd.refresh();
    }
    else if (page==2)
    {
      draw_dash2_page(cval_use, usekwh, etemp, minetemp, maxetemp, otemp, minotemp, maxotemp, temp, mintemp, maxtemp, hour, minute, batt, last_emontx, last_emonbase);
      glcd.refresh();
    }
//    else if (page==3)
//    {
//      draw_power_page( "POWER" ,cval_use, "USAGE", usekwh);
//      draw_temperature_time_footer(temp, mintemp, maxtemp, hour,minute);
//      glcd.refresh();
//    }
    int LDR = analogRead(LDRpin);                     // Read the LDR Value so we can work out the light level in the room.
    int LDRbacklight = map(LDR, 0, 1023, 50, 250);    // Map the data from the LDR from 0-1023 (Max seen 1000) to var GLCDbrightness min/max
    LDRbacklight = constrain(LDRbacklight, 0, 255);   // Constrain the value to make sure its a PWM value 0-255
    // if ((hour > 22) ||  (hour < 5)) glcd.backLight(0); else // We want the backlight constantly on
    
    glcd.backLight(LDRbacklight);  


//	analogWrite(redLED, 0);         
//	analogWrite(greenLED, 0);    
  

  } 
  
  if ((millis()-slow_update)>10000)
  {
    slow_update = millis();

    sensors.requestTemperatures();
    double rawtemp = (sensors.getTempCByIndex(0));
    if ((rawtemp>-20) && (rawtemp<50)) temp=rawtemp;                  //is temperature withing reasonable limits?
    if (temp > maxtemp) maxtemp = temp;
    if (temp < mintemp) mintemp = temp;
    if (otemp > maxotemp) maxotemp = otemp;
    if (otemp < minotemp) minotemp = otemp;
    if (humi > maxhumi) maxhumi = humi;
    if (humi < minhumi) minhumi = humi;
    if (etemp > maxetemp) maxetemp = etemp;
    if (etemp < minetemp) minetemp = etemp;
       
    emonglcd.temperature = (int) (temp * 100);                          // set emonglcd payload
    send_rf_data();
  }
}
