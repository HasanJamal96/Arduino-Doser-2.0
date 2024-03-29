#define DEBUG // Comment this line to stop dubug messages
//#define SHOW_DHT_OUTPUT

#ifdef DEBUG
  #define BAUDRATE 115200
#endif


#include "DHT.h"
#include <Wire.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <virtuabotixRTC.h>
#include <LiquidCrystal_I2C.h>

/*
Schedule type
0 -> trigger only Once
1 -> trigger only Once per week
2 -> trigger daily, for this type no need to set weekday
*/


// Globals
String current_screen = "Home";
String last_screen = "Home";
unsigned long last_activity = 0;
unsigned long LastSync = 0;
const unsigned long SYNC_AFTER = 1; // time in hours


unsigned long DoseStartTime = 0;
unsigned long DosePhase2 = 0;
unsigned long DosePhase3 = 0;
unsigned long DosePhase4 = 0;
unsigned long DosePhase5 = 0;
uint8_t DosingLiquid = 0;
bool isDosing = false;
uint8_t selected_liquid = 0;
uint32_t qdq = 0;
uint8_t Running_Schedule = 0;
uint8_t Running_Schedule_Liquid = 0;
bool isScheduleRunning = false;
bool StartSchedule = false;


// DC Motors
const uint8_t DC_EN_Pins[4] = {2,3,4,5};
bool Accel_DC = false;
uint8_t which_dc = 0;
uint8_t dc_speed = 0;
unsigned long last_accel = 0;
const int ACCEL_AFTER = 100;


/*
 *  provided that stepper required 18 sec at 600 steps/sec for 25 drops
 *  and 1 ml = 25 drops
 *  
 *  calculations:
 *    1 drop = 1ml/25
 *    1 drop = 0.04 ml
 *  
 *    600 steps/sec for 18 sec to do 25 drops
 *    total steps in 18 sec
 *      600*18
 *      10800 to do 25 drops
 *  
 *  arduino is providing 490 steps/sec
 *  
 *  so at 490
 *    10800 steps require (10800/490) = 22.04 sec
 *  
 *  so for 1 drop at 490, time required is:
 *    25 drops = 22.04
 *    1 drop = (22.04/25) = 881.6 milli sec
 */


/* 
 *  Time duration constants
 *  All time define below are in miili seconds
 *  
 */
const float one_drop_time = 881.6; // 
const float one_drop_ml = 0.04; //1 drop = 0.04ml
const uint32_t DC_MOTOR_DURATION = 30000; // DC motor running duration
const uint32_t PRIME_TIME = 27600;
const uint32_t PRIME_TIME_7 = 27600;
const uint32_t CLEAR_TIME = 72000;
const uint32_t FLUSH_CW = 36000;  // Flushing in Clock wise
const uint32_t FLUSH_CCW = 72000; // Flushing in Counter clock wise
/*
 * End of time duration constatnts
 */



uint32_t Max_Dosing_Duration = 0;
uint32_t Remaining_Dosing_Duration = 0;

unsigned long progess_last_update = 0;

int32_t DosingTime = 0;
String DosingPhase = "0";



// Steppers
const uint8_t SteppersEN[2] = {31, 33};
const uint8_t SteppersDIR[2] = {30, 32};
const uint8_t SteppersSTEP[2] = {44, 45};
uint8_t activeStepper = 0;
bool isFlush = false;

// Screen Function Defination
void DisplayMenuScreen();



// Liquids
float Total_Liquid[7] = {100, 20, 50, 70, 90, 100, 50};
float Remaining_Liquid[7] = {100, 20, 50, 70, 90, 100, 50};
int Drops[7][3] = {1, 2, 3,
                   4, 5, 1,
                   5, 6, 5,
                   8, 9, 1,
                   1, 2, 3,
                   8, 6 ,4,
                   1, 9 ,7};

String LiquidNames[7] = {"Liquid 1","Liquid 2","Liquid 3","Liquid 4","Liquid 5","Liquid 6","Liquid 7"};
const int LiquidNameAddr[7] = {266, 279, 292, 305, 318, 331, 344};
const int Total_Liquid_Addr[7] = {210, 214, 218, 222, 226, 230, 234};
const int Remain_Liquid_Addr[7] = {238, 242, 246, 250, 254, 258, 262};
const int Drops_Addr[7][3] = {357, 359, 361, 
                              363, 365, 367, 
                              369, 371, 373,
                              375, 377, 379,
                              381, 383, 385,
                              387, 389, 391,
                              393, 395, 397};


const int schedule_Type_Addr[7][3] = {399, 400, 401,
                                 402, 403, 404, 
                                 405, 406, 407,
                                 408, 409, 410,
                                 411, 412, 413,
                                 414, 415, 416,
                                 417, 418, 419};


uint8_t Schedule_Type[7][3]= {0,0,0,
                           0,0,0,
                           0,0,0,
                           0,0,0,
                           0,0,0,
                           0,0,0,
                           0,0,0};

const uint16_t LIQUID_UPDATE_AFTER = 10000;
unsigned long liquid_last_update = 0;

// LCD
LiquidCrystal_I2C lcd(0x27, 20,4);

byte L0[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
};
byte L1[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
};
byte L2[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
};
byte L3[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte L4[8] = {
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte L5[8] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte L6[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

const String AlphabetMap[9] = {"ABC1", "DEF2", "GHI3", "JKL4", "MNO5", "PQR6", "STU7", "VWX8", "YZ.9"};



// RTC
#define RTC_CLK 11
#define RTC_DATA 12
#define RTC_RESET 13
virtuabotixRTC myRTC(RTC_CLK, RTC_DATA, RTC_RESET);
const uint8_t mth_key[12] = {1,3,3,0,2,5,0,3,6,1,4,6};
unsigned long last_rtc_update = 0;
const uint16_t RTC_UPDATE_INTERVAL = 1000; // time im ms
timeDayOfWeek_t weekDays[7] = {dowSunday, dowMonday, dowTuesday, dowWednesday, dowThursday, dowFriday, dowSaturday};
const String weeaysNames[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
AlarmID_t AlarmIDs[7][3];
typedef void (*function) () ;

// DHT
#define DHT_PIN A15
#define DHTTYPE DHT11 
DHT dht(DHT_PIN, DHTTYPE);
unsigned long dht_last_update = 0;
const uint16_t DHT_UPDATE_AFTER = 5000;
float humidity = 0;
float temperature = 0;

// LEDs
#define LEDlatch  9
#define LEDclock  10
#define LEDdata   8
#define LEDoe 7
uint8_t Bled1 = 1;
uint8_t Bled2 = 16;
uint8_t Fade_Pwm = 0;
bool fade_state = false;
bool generateRandom = true;
bool Blink_state = false;
unsigned long last_fade=0;
unsigned long last_chase=0;
const int CHASE_DELAY = 100;
String LEDs_Status = "";
unsigned long LastBlink = 0;
const uint16_t BLINK_INTERVAL = 500;


// MOSFETS
const uint8_t MOSFET_PINS[8] = {22, 23, 24, 25, 26 ,27, 28, 29};


//Shedules

String Dose_Shedules[7][3];
const int Dose_Sched_Addr[7][3] = {
          {0,10,20},
          {30,40,50},
          {60,70,80},
          {90,100,110},
          {120,130,140},
          {150,160,170},
          {180,190,200}
       };


/*
  ["15,12,12,7", "", ""],
  ["", "", ""],
  ["", "", ""],
  ["", "", ""],
  ["", "", ""],
  ["", "", ""],
  ["", "", ""]
*/



void ActivateStepper(uint8_t ID, uint8_t DIR);
void DeactivateStepper(uint8_t ID);






void InitializePins(){
  #ifdef DEBUG
    Serial.println("[Main] Initializing pins");
  #endif
  for(uint8_t i=0; i<8; i++)
    pinMode(MOSFET_PINS[i], OUTPUT);
  for(uint8_t i=0; i<4; i++)
    pinMode(DC_EN_Pins[i], OUTPUT);
  
  pinMode(LEDlatch, OUTPUT);
  pinMode(LEDdata, OUTPUT);  
  pinMode(LEDclock, OUTPUT);
  pinMode(LEDoe, OUTPUT);


  pinMode(SteppersEN[0], OUTPUT);
  pinMode(SteppersDIR[0], OUTPUT);
  pinMode(SteppersSTEP[0], OUTPUT);
  
  pinMode(SteppersEN[1], OUTPUT);
  pinMode(SteppersDIR[1], OUTPUT);
  pinMode(SteppersSTEP[1], OUTPUT);

  digitalWrite(SteppersEN[0], HIGH);
  digitalWrite(SteppersEN[1], HIGH);

  
  #ifdef DEBUG
    Serial.println("[Main] Pins initialization complete");
  #endif
}

bool read_dht(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)){
    #ifdef DEBUG
      Serial.println(F("[DHT] Failed to read"));
    #endif
    return false;
  }
  else{
    #ifdef SHOW_DHT_OUTPUT
      Serial.print("\n[DHT] Temperature: ");
      Serial.print(temperature);
      Serial.print(" Humidity: ");
      Serial.println(humidity);
    #endif
    return true;
  }
}


void InitializeLCD(){
  #ifdef DEBUG
    Serial.println("[LCD] Initializing");
  #endif
  lcd.init();
  lcd.backlight();
  dht.begin();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.createChar(0, L0);
  lcd.createChar(1, L1);
  lcd.createChar(2, L2);
  lcd.createChar(3, L3);
  lcd.createChar(4, L4);
  lcd.createChar(5, L5);
  lcd.createChar(6, L6);
  #ifdef DEBUG
    Serial.println("[LCD] Initialization complete");
  #endif
}


void ResetAllSchedules(){
  #ifdef DEBUG
    Serial.println("[EEPROM] Resetting all schedules");
  #endif
  for (uint8_t x=0; x<7; x++){
    for (uint8_t i=0; i<3; i++){
      Dose_Shedules[x][i] = "00,00,00:0";
      writeStringToEEPROM(Dose_Sched_Addr[x][i], Dose_Shedules[x][i]);
    } 
  }
  #ifdef DEBUG
    Serial.println("[EEPROM] All schedules have been reset");
  #endif
}

void ReadLiquidVolumes(){
  #ifdef DEBUG
    Serial.println("[EEPROM] Reading saved liquid volumes");
  #endif
  for(uint8_t x=0; x<7; x++){
    Total_Liquid[x] = readFloatFromEEPROM(Total_Liquid_Addr[x]);
    Remaining_Liquid[x] = readFloatFromEEPROM(Remain_Liquid_Addr[x]);
  }
}



void setup(){
  #ifdef DEBUG
    Serial.begin(BAUDRATE);
    Serial.println("[Main] Setup started");
  #endif
  InitializePins();
  #ifdef DEBUG
    Serial.println("[EEPROM] Reading configurations");
  #endif
  ReadSchedulesFromEEPROM();
  ReadNamesFromEEPROM();
  ReadLiquidVolumes();
  ReadScheduleTypes();
  #ifdef DEBUG
    Serial.println("[EEPROM] Configurations successfully raed");
  #endif
  #ifdef DEBUG
    Serial.println("[RTC] Updating controllers time with RTC time");
  #endif
  myRTC.updateTime();
  setTime(myRTC.hours, myRTC.minutes, myRTC.seconds, myRTC.dayofmonth, myRTC.month, myRTC.year); //H:M:S,date, month, year
  AttatchSchedules();
  LastSync = millis()/1000;
  InitializeLCD();
  ClearLeds();
  DisplayHomeScreen();
  SetKeypadParams();

  #ifdef DEBUG
    Serial.println("[Main] Setup complete");
  #endif
}

void loop(){
  Alarm.delay(1);
  if(StartSchedule){
    DosingLiquid = Running_Schedule_Liquid;
    StartDose(Running_Schedule_Liquid, 1);
    isDosing = isScheduleRunning = true;
    StartSchedule = false;
    if(Schedule_Type[DosingLiquid][Running_Schedule] == 0){
      Alarm.free(AlarmIDs[DosingLiquid][Running_Schedule]);
      AlarmIDs[DosingLiquid][Running_Schedule] = 255;
      
      Dose_Shedules[DosingLiquid][Running_Schedule] = "00,00,00:0";
      Drops[DosingLiquid][Running_Schedule] = 0;
      
      writeStringToEEPROM(Dose_Sched_Addr[DosingLiquid][Running_Schedule], "00,00,00:0");
      writeIntIntoEEPROM(Drops_Addr[DosingLiquid][Running_Schedule], 0);
    }
  }
  else if(isScheduleRunning){
    Dosing();
  }
  
  if(current_screen == "Home"){
    if(millis() - last_rtc_update >= RTC_UPDATE_INTERVAL){
      UpdateRTC();
      last_rtc_update = millis();
    }
    if(millis() - dht_last_update >= DHT_UPDATE_AFTER){
      UpdateDHT();
      dht_last_update = millis();
    }
    if(millis() - liquid_last_update >= LIQUID_UPDATE_AFTER){
      UpdateCapacityBars();
      liquid_last_update = millis();
    }
    ReadHomeScreen();
  }
  else if(current_screen == "Menu"){
    ReadMenuScreen();
    if(generateRandom){
      byte hl = random(0, 255);
      byte ll = random(0, 255);
      SetLeds(ll,hl);
      generateRandom = false;
    }
    if(millis() - last_fade >= 50){
      FadeLeds();
      last_fade = millis();
    }
  }
  else if(current_screen == "Schedule"){
    ReadScheduleScreen();
  }
  else if(current_screen == "Liquid"){
    ReadLiquidScreen();
  }
  else if(current_screen == "Rename"){
    ReadRenameScreen();
  }
  else if(current_screen == "EditTotal"){
    ReadEditVolume();
  }
  else if(current_screen == "TD"){
    ReadRTCScreen();
  }
  else if(current_screen == "Dosing"){
    ReadDosingScreen();
  }
  else if(current_screen == "Dosing2"){
    ReadEditDose();
  }
  else if(current_screen == "Dosing3"){
    ReadDisplayEditSchedule();
  }
  else if(current_screen == "Quick"){
    ReadQuickDoseScreen();
    
  }
  else if(current_screen == "DosingQnty"){
    ReadDosingQuantity();
  }
  else if(current_screen == "Flush"){
    ReadFluchScreen();
  }
  else if(current_screen == "Progress"){
    if(isFlush)
      Flushing();
    else if (isDosing)
      Dosing();
    if(millis() - progess_last_update > 1000){
      updateProgressBar();
      progess_last_update = millis();
    }
    ReadProgressScreen();
  }
  LedsLoopCode();
  if(millis()/1000 - LastSync >= (SYNC_AFTER*60*60)){
    doSync();
    LastSync = millis()/1000;
  }
}

void doSync(){
  #ifdef DEBUG
    Serial.println("[RTC] Syncing arduino's time with RTC");
    Serial.println("[RTC] If schedule occur during syncing it will not perform its function");
  #endif
  freeSheculesIDs();
  myRTC.updateTime();
  setTime(myRTC.hours, myRTC.minutes, myRTC.seconds, myRTC.dayofmonth, myRTC.month, myRTC.year);
  AttatchSchedules();
  #ifdef DEBUG
    Serial.println("[RTC] Syncing complete");
  #endif
}


void freeSheculesIDs(){
  for (uint8_t x=0; x<7; x++){
    for (uint8_t i=0; i<3; i++){
      if(AlarmIDs[x][i] != 255){
        Alarm.free(AlarmIDs[x][i]);
        AlarmIDs[x][i] = 255;
      }
    }
  }
}
