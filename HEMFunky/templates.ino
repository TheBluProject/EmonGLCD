#include "utility/font_helvB24.h"
#include "utility/font_helvB14.h"
#include "utility/font_helvB12.h"
#include "utility/font_clR4x6.h"
#include "utility/font_clR6x8.h"

////------------------------------------------------------------------
//// Draws a page showing a single power and energy value in big font
////------------------------------------------------------------------
//void draw_power_page(char* powerstr, double powerval, char* energystr,  double energyval)
//{ 
//  glcd.clear();
//  glcd.fillRect(0,0,128,64,0);
//  
//  char str[25];    			 //variable to store conversion 
//  glcd.setFont(font_clR6x8);      
//  strcpy(str,powerstr);  
//  strcat(str," CONSUMPTION"); 
//  glcd.drawString(0,0,str);
//  strcpy(str,energystr);  
//  strcat(str," TODAY"); 
//  glcd.drawString(0,38,str);
//
//  // power value
//  glcd.setFont(font_helvB24);
//  itoa((int)powerval,str,10);
//  strcat(str,"w");   
//  glcd.drawString(3,9,str);
//  
//  // kwh per day value
//  glcd.setFont(font_clR6x8);
//  if (energyval<10.0) dtostrf(energyval,0,1,str); else itoa((int)energyval,str,10);
//  strcat(str,"kWh");
//  glcd.drawString(85,38,str);        
//}
//
//
////------------------------------------------------------------------
//// Draws a footer showing GLCD temperature and the time
////------------------------------------------------------------------
//void draw_temperature_time_footer(double temp, double mintemp, double maxtemp, double hour, double minute)
//{
//  glcd.drawLine(0, 47, 128, 47, WHITE);     //middle horizontal line 
//
//  char str[25];
//  // GLCD Temperature
//  glcd.setFont(font_helvB12);  
//  dtostrf(temp,0,1,str); 
//  strcat(str,"C");
//  glcd.drawString(0,50,str);  
//  
//  // Minimum and maximum GLCD temperature
//  glcd.setFont(font_clR4x6);             
//  itoa((int)mintemp,str,10);
//  strcat(str,"C");
//  glcd.drawString_P(46,51,PSTR("MIN"));
//  glcd.drawString(62,51,str);
//               
//  itoa((int)maxtemp,str,10); 
//  strcat(str,"C");
//  glcd.drawString_P(46,59,PSTR("MAX"));
//  glcd.drawString(62,59,str);
//  
//  // Time
//  char str2[5];
//  itoa((int)hour,str,10);
//  if  (minute<10) strcat(str,":0"); else strcat(str,":");
//  itoa((int)minute,str2,10);
//  strcat(str,str2); 
//  glcd.setFont(font_helvB12);
//  glcd.drawString(88,50,str);
//
//}

//------------------------------------------------------------------
// Draws the Dasboard page
//------------------------------------------------------------------
void draw_dash_page(double use, double usekwh, double humi, double otemp, double minotemp,double maxotemp, double dewpoint, double temp, double mintemp, double maxtemp, double hour, double minute, double batt, unsigned long last_emontx, unsigned long last_emonbase)
{
  
  int MINTEMP = -15;
  int MAXTEMP = 40;
  int MINOTEMP = -20;
  int MAXOTEMP = 40;
  byte imageindex;

  glcd.clear();
  glcd.fillRect(0,0,128,64,0);
  
  glcd.drawLine(64, 0, 64, 64, WHITE);      //top vertical line
  glcd.drawLine(0, 32, 128, 32, WHITE);     //middle horizontal line 

  //variables to store conversion
  char str[50];
  char str2[5];

  // Last seen information from EmonTX
  if ((millis()-last_emontx)>20000)
  {
    // small font
    glcd.setFont(font_clR4x6);
    int emonTx_fail=(millis()-last_emontx)/1000;
    if (emonTx_fail<100){
      itoa(emonTx_fail, str, 10);
      strcat(str,"sec TxFail");
      glcd.drawString(66,0,str);
    }
    else
      glcd.drawString(66,0,PSTR("TxFail"));
    
  }
  if ((millis()-last_emonbase)>120000)
  {
    // small font
    glcd.setFont(font_clR4x6);
    int emonbase_fail=((millis()-last_emonbase)/60000);
    if (emonbase_fail<100){
      itoa(emonbase_fail, str, 10);
      strcat(str,"min baseFail");
      glcd.drawString(67,34,str);
    }
    else glcd.drawString(67,34,PSTR("baseFail"));
    
  }
  //EmonTX batt
  dtostrf(batt,0,0,str);
  glcd.drawString_P(0,0,PSTR("Batt "));
  strcat(str,"mV");
  glcd.drawString(19,0,str);
  
   //Dewpoint 
  dtostrf(dewpoint,0,1,str);
  strcat(str,"c");
  glcd.drawString_P(0,34,PSTR("DewPT "));
  glcd.drawString(23,34,str);
  
  //big bold font
  glcd.setFont(font_helvB14);

  // Amount of power currently being used
  if(use > 1000)
  {
    dtostrf(use/1000,2,1,str);
    strcat(str,"kw");   
  }
  else
  {
    itoa((int)use,str,10);
    strcat(str,"w");   
  }  
  glcd.drawBitmap(49,0,icon_home_13x12,16,12,1);
  glcd.drawString(3,9,str);

  // Temperature Inside             
  dtostrf(temp,0,1,str); 
  strcat(str,"c");
  if (temp>MAXTEMP)
  {
    imageindex=5;
  }
  else
  {
    if (temp<MINTEMP) imageindex=0; else imageindex=int((temp-MINTEMP)/(MAXTEMP-MINTEMP)*5-0.5);
  }
  glcd.drawBitmap(120,35,icon_heating_8x16[imageindex],8,16,1);
  glcd.drawString(70,42,str);
  glcd.setFont(font_clR4x6); 
  glcd.drawString_P(110,35,PSTR("IN"));
  glcd.setFont(font_helvB14);  
  
  // Temperature Outside             
  dtostrf(otemp,0,1,str); 
  strcat(str,"c");
  if (otemp>MAXOTEMP)
  {
    imageindex=5;
  }
  else
  {
    if (otemp<MINOTEMP) imageindex=0; else imageindex=int((otemp-MINOTEMP)/(MAXOTEMP-MINOTEMP)*5-0.5);
  }
  glcd.drawBitmap(120,2,icon_heating_8x16[imageindex],8,16,1);
  glcd.drawString(70,9,str);
  glcd.setFont(font_clR4x6); 
  glcd.drawString_P(107,2,PSTR("OUT"));
  glcd.setFont(font_helvB14);    
  
  // Humidity
  dtostrf(humi,0,1,str); 
  strcat(str,"%");
  glcd.drawBitmap(51,34,icon_solar_12x12,16,12,1);
  glcd.drawString(3,42,str);
  
  glcd.setFont(font_clR4x6);   		//small font - Kwh

  // Kwh consumed today
  dtostrf(usekwh,0,1,str);
  strcat(str,"kWh today");  
  glcd.drawString(6,26,str);

  // Minimum and maximum temperatures outside
  itoa((int)minotemp,str,10);
  strcat(str,"c");
  glcd.drawString_P(68,26,PSTR("min"));
  glcd.drawString(82,26,str);
  itoa((int)maxotemp,str,10); 
  strcat(str,"c");
  glcd.drawString_P(97,26,PSTR("max"));
  glcd.drawString(111,26,str);

  // Minimum and maximum temperatures inside
  itoa((int)mintemp,str,10);
  strcat(str,"c");
  glcd.drawString_P(68,58,PSTR("min"));
  glcd.drawString(82,58,str);
  itoa((int)maxtemp,str,10); 
  strcat(str,"c");
  glcd.drawString_P(97,58,PSTR("max"));
  glcd.drawString(111,58,str);

  // Current time
  glcd.drawString_P(5,58,PSTR("Time "));
  itoa((int)hour,str,10);
  if  (minute<10) strcat(str,":0"); else strcat(str,":"); 
  itoa((int)minute,str2,10);
  strcat(str,str2); 
  glcd.drawString(28,58,str); 

}

//------------------------------------------------------------------
// Draws the Dasboard 2 page
//------------------------------------------------------------------
void draw_dash2_page(double use, double usekwh, double etemp, double minetemp, double maxetemp, double otemp, double minotemp,double maxotemp, double temp, double mintemp, double maxtemp, double hour, double minute, double batt, unsigned long last_emontx, unsigned long last_emonbase)
{
  
  int MINTEMP = -15;
  int MAXTEMP = 40;
  int MINOTEMP = -20;
  int MAXOTEMP = 40;
  int MINETEMP = 15;
  int MAXETEMP = 75;
  byte imageindex;

  glcd.clear();
  glcd.fillRect(0,0,128,64,0);
  
  glcd.drawLine(64, 0, 64, 64, WHITE);      //top vertical line
  glcd.drawLine(0, 32, 128, 32, WHITE);     //middle horizontal line 

  //variables to store conversion
  char str[50];
  char str2[5];

  // Last seen information from EmonTX
  if ((millis()-last_emontx)>20000)
  {
    // small font
    glcd.setFont(font_clR4x6);
    int emonTx_fail=(millis()-last_emontx)/1000;
    if (emonTx_fail<100){
      itoa(emonTx_fail, str, 10);
      strcat(str,"sec TxFail");
      glcd.drawString(66,0,str);
    }
    else
      glcd.drawString(66,0,PSTR("TxFail"));
    
  }
  if ((millis()-last_emonbase)>120000)
  {
    // small font
    glcd.setFont(font_clR4x6);
    int emonbase_fail=((millis()-last_emonbase)/60000);
    if (emonbase_fail<100){
      itoa(emonbase_fail, str, 10);
      strcat(str,"min baseFail");
      glcd.drawString(67,34,str);
    }
    else glcd.drawString(67,34,PSTR("baseFail"));
    
  }
  //EmonTX batt
  dtostrf(batt,0,0,str);
  glcd.drawString_P(0,0,PSTR("Batt "));
  strcat(str,"mV");
  glcd.drawString(19,0,str);
  
//   //Dewpoint 
//  dtostrf(dewpoint,0,1,str);
//  strcat(str,"c");
//  glcd.drawString_P(0,34,PSTR("DewPT "));
//  glcd.drawString(23,34,str);
  
  //big bold font
  glcd.setFont(font_helvB14);

  // Amount of power currently being used
  if(use > 1000)
  {
    dtostrf(use/1000,2,1,str);
    strcat(str,"kw");   
  }
  else
  {
    itoa((int)use,str,10);
    strcat(str,"w");   
  }  
  glcd.drawBitmap(49,0,icon_home_13x12,16,12,1);
  glcd.drawString(3,9,str);

  // Temperature Inside             
  dtostrf(temp,0,1,str); 
  strcat(str,"c");
  if (temp>MAXTEMP)
  {
    imageindex=5;
  }
  else
  {
    if (temp<MINTEMP) imageindex=0; else imageindex=int((temp-MINTEMP)/(MAXTEMP-MINTEMP)*5-0.5);
  }
  glcd.drawBitmap(120,35,icon_heating_8x16[imageindex],8,16,1);
  glcd.drawString(70,42,str);
  glcd.setFont(font_clR4x6); 
  glcd.drawString_P(110,35,PSTR("IN"));
  glcd.setFont(font_helvB14);  
  
  // Temperature Outside             
  dtostrf(otemp,0,1,str); 
  strcat(str,"c");
  if (otemp>MAXOTEMP)
  {
    imageindex=5;
  }
  else
  {
    if (otemp<MINOTEMP) imageindex=0; else imageindex=int((otemp-MINOTEMP)/(MAXOTEMP-MINOTEMP)*5-0.5);
  }
  glcd.drawBitmap(120,2,icon_heating_8x16[imageindex],8,16,1);
  glcd.drawString(70,9,str);
  glcd.setFont(font_clR4x6); 
  glcd.drawString_P(107,2,PSTR("OUT"));
  glcd.setFont(font_helvB14);

// Temperature EmonTX             
  dtostrf(etemp,0,1,str); 
  strcat(str,"c");
  if (etemp>MAXETEMP)
  {
    imageindex=5;
  }
  else
  {
    if (etemp<MINETEMP) imageindex=0; else imageindex=int((etemp-MINETEMP)/(MAXETEMP-MINETEMP)*5-0.5);
  }
  glcd.drawBitmap(55,35,icon_heating_8x16[imageindex],8,16,1);
  glcd.drawString(3,42,str);
  glcd.setFont(font_clR4x6); 
  glcd.drawString_P(30,35,PSTR("C.Heat"));
    
  glcd.setFont(font_clR4x6);   		//small font - Kwh

  // Kwh consumed today
  dtostrf(usekwh,0,1,str);
  strcat(str,"kWh today");  
  glcd.drawString(6,26,str);

  // Minimum and maximum temperatures outside
  itoa((int)minotemp,str,10);
  strcat(str,"c");
  glcd.drawString_P(68,26,PSTR("min"));
  glcd.drawString(82,26,str);
  itoa((int)maxotemp,str,10); 
  strcat(str,"c");
  glcd.drawString_P(97,26,PSTR("max"));
  glcd.drawString(111,26,str);

  // Minimum and maximum temperatures inside
  itoa((int)mintemp,str,10);
  strcat(str,"c");
  glcd.drawString_P(68,58,PSTR("min"));
  glcd.drawString(82,58,str);
  itoa((int)maxtemp,str,10); 
  strcat(str,"c");
  glcd.drawString_P(97,58,PSTR("max"));
  glcd.drawString(111,58,str);
  
   // Minimum and maximum temperatures emon
  itoa((int)minetemp,str,10);
  strcat(str,"c");
  glcd.drawString_P(2,58,PSTR("min"));
  glcd.drawString(16,58,str);
  itoa((int)maxetemp,str,10); 
  strcat(str,"c");
  glcd.drawString_P(31,58,PSTR("max"));
  glcd.drawString(45,58,str);
  

  // Current time
  itoa((int)hour,str,10);
  if  (minute<10) strcat(str,":0"); else strcat(str,":"); 
  itoa((int)minute,str2,10);
  strcat(str,str2); 
  glcd.drawString(1,35,str); 

}

