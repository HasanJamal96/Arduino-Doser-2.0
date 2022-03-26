#include "DHT.h"
#include <Wire.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <virtuabotixRTC.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG true

// Globals
String current_screen = "Home";
String last_screen = "Home";
unsigned long last_activity = 0;
int one_drop_time = 782; //time in milli sec

unsigned long DoseStartTime = 0;
unsigned long DosePhase2 = 0;
unsigned long DosePhase3 = 0;
unsigned long DosePhase4 = 0;
unsigned long DosePhase5 = 0;
uint8_t DosingLiquid = 0;
bool isDosing = false;
uint8_t selected_liquid = 0;
int qdq = 0;


// DC Motors
const uint8_t DC_EN_Pins[4] = {2,3,4,5};
bool Accel_DC = false;
uint8_t which_dc = 0;
uint8_t dc_speed = 0;
unsigned long last_accel = 0;
const int ACCEL_AFTER = 100;
const uint8_t drops_time_loc = 2;
const uint16_t step_200_duration = 1200;
uint16_t DC_MOTOR_DURATION = 30;//000;
uint16_t PRIME_TIME = 27600;
uint32_t CLEAR_TIME = 72000;
uint32_t DosingTime = 0;
String DosingPhase = "0";



// Steppers
const uint8_t SteppersEN[2] = {31, 33};
const uint8_t SteppersDIR[2] = {30, 32};
const uint8_t SteppersSTEP[2] = {44, 45};
bool SteppersStepsStatus = false;
unsigned long lastStep = 0;

// Screen Function Defination
void DisplayMenuScreen();



// Liquids
float Total_Liquid[7] = {100, 20, 50, 70, 90, 100, 50};
float Remaining_Liquid[7] = {100, 20, 50, 70, 90, 100, 50};
int Drops[7] = {1,2,3,4,5,10,50};

String LiquidNames[7] = {"Liquid 1","Liquid 2","Liquid 3","Liquid 4","Liquid 5","Liquid 6","Liquid 7"};
const int LiquidNameAddr[7] = {266, 279, 292, 305, 318, 331, 344};
const int Total_Liquid_Addr[7] = {210, 214, 218, 222, 226, 230, 234};
const int Remain_Liquid_Addr[7] = {238, 242, 246, 250, 254, 258, 262};
const int Drops_Addr[7] = {357, 359, 361, 363, 365, 367, 369};

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

String AlphabetMap[9] = {"ABC1", "DEF2", "GHI3", "JKL4", "MNO5", "PQR6", "STU7", "VWX8", "YZ.9"};



// RTC
#define RTC_CLK 11
#define RTC_DATA 12
#define RTC_RESET 13
virtuabotixRTC myRTC(RTC_CLK, RTC_DATA, RTC_RESET);
int mth_key[12] = {1,3,3,0,2,5,0,3,6,1,4,6};
unsigned long last_rtc_update = 0;
const uint16_t RTC_UPDATE_INTERVAL = 1000; // time im ms


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
int CHASE_DELAY = 100;
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
  for(uint8_t i=0; i<4; i++){
    pinMode(DC_EN_Pins[i], OUTPUT);
  
  pinMode(LEDlatch, OUTPUT);
  pinMode(LEDdata, OUTPUT);  
  pinMode(LEDclock, OUTPUT);
  pinMode(LEDoe, OUTPUT);


  pinMode(SteppersEN[0], OUTPUT);
  digitalWrite(SteppersEN[0], HIGH);
  pinMode(SteppersDIR[0], OUTPUT);
  pinMode(SteppersSTEP[0], OUTPUT);
  pinMode(SteppersEN[1], OUTPUT);
  digitalWrite(SteppersEN[1], HIGH);
  pinMode(SteppersDIR[1], OUTPUT);
  pinMode(SteppersSTEP[1], OUTPUT);

  
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
    #ifdef DEBUG
      /*Serial.print("\n[DHT] Temperature: ");
      Serial.print(temperature);
      Serial.print(" Humidity: ");
      Serial.println(humidity);*/
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
  for (uint8_t x=0; x<7; x++){
    for (uint8_t i=0; i<3; i++){
      Dose_Shedules[x][i] = "12,34,11:1";
      writeStringToEEPROM(Dose_Sched_Addr[x][i], Dose_Shedules[x][i]);
    } 
  }
}

void ReadLiquidVolumes(){
  for(uint8_t x=0; x<7; x++){
    Total_Liquid[x] = readFloatFromEEPROM(Total_Liquid_Addr[x]);
    Remaining_Liquid[x] = readFloatFromEEPROM(Remain_Liquid_Addr[x]);
    Drops[x] = readIntFromEEPROM(Drops_Addr[x]);
  }
}


void AttatchSchedules(){
  // Work in progress :)
  return;
}

void setup(){
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("[Main] Setup started");
  #endif
  InitializePins();
  ReadSchedulesFromEEPROM();
  ReadNamesFromEEPROM();
  ReadLiquidVolumes();
  InitializeLCD();
  DisplayHomeScreen();
  SetKeypadParams();
  ClearLeds();

  #ifdef DEBUG
    Serial.println("[RTC] setting time ");
    Serial.println("[Main] Setup complete");
  #endif
}

void loop(){
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
    if(isDosing){
      if(DosingPhase == "1"){
        if(Accel_DC){
          if(millis() - last_accel >= ACCEL_AFTER){
            AccelerateDCMotor();
            last_accel = millis();
            Serial.println("DC accel");
          }
        }
        else if(millis() - DoseStartTime >= DC_MOTOR_DURATION){
          digitalWrite(DC_EN_Pins[which_dc], 0);
          StartPhase2();
        }
      }
      else if(DosingPhase == "2"){
        if(millis() - DosePhase2 >= PRIME_TIME + DosingTime){
          ActivateStepper(1, 0);
          DosingPhase = "3";
          DosePhase3 = millis();
          Serial.println("Phase 3 started");
        }
      }
      else if(DosingPhase == "3"){
        if(millis() - DosePhase3 >= CLEAR_TIME){
          DeactivateStepper(1);
          digitalWrite(MOSFET_PINS[selected_liquid], LOW);
          DosingPhase = "4";
          ClearLeds();
          LEDs_Status = "Chase";
          ActivateStepper(0, 1);
          DosePhase4 = millis();
          digitalWrite(MOSFET_PINS[7], HIGH);
          Serial.println("Phase 4 started");
        }
      }
      else if(DosingPhase == "4"){
        if(millis() - DosePhase4 >= 36000){
          ActivateStepper(0, 0);
          DosingPhase = "5";
          DosePhase5 = millis();
          Serial.println("Phase 5 started");
        }
      }
      else if(DosingPhase == "5"){
        if(millis() - DosePhase5 >= 60000){
          EndDose(true);
        }
      }
    }
  }
  else if(current_screen == "DosingQnty"){
    ReadDosingQuantity();
  }
  else if(current_screen == "Flush"){
    ReadFluchScreen();
  }
  
  
  if(LEDs_Status == "Blink"){
    if(millis() - LastBlink >= BLINK_INTERVAL){
      BlinkLeds(Blink_state);
      LastBlink = millis();
      if(Blink_state)
        Blink_state = false;
      else
        Blink_state = true;
    }
  }
  else if(LEDs_Status == "Fade"){
    if(millis() - last_fade >= 50){
      FadeLeds();
      last_fade = millis();
    }
  }
  else if(LEDs_Status == "Chase"){
    if(millis() - last_chase > CHASE_DELAY){
      ChaseLeds();
      last_chase = millis();
    }
  }
  else if(LEDs_Status == "Chase2"){
    if(millis() - last_chase > CHASE_DELAY){
      Chase2();
      last_chase = millis();
    }
  }
}

void AccelerateDCMotor(){
  if(dc_speed < 155)
    dc_speed += 5;
  else
    Accel_DC = false;
  analogWrite(DC_EN_Pins[which_dc], dc_speed);
}

void StartPhase2(){
  digitalWrite(MOSFET_PINS[selected_liquid], HIGH);
  getLiquidLeds(selected_liquid);
  LEDs_Status = "Blink";
  DosingPhase = "2";
  DosingTime = qdq * 1104;
  ActivateStepper(1, 1);
  DosePhase2 = millis();
}

void EndDose(bool Normal){
  DeactivateStepper(0);
  digitalWrite(MOSFET_PINS[7], LOW);
  float usedDrops = 0;
  if(!Normal){
    DosingTime = millis() - DoseStartTime;
  }
  usedDrops = (DosingTime/1104) * 0.04;
  Remaining_Liquid[selected_liquid] -= usedDrops;
  writeFloatIntoEEPROM(Remain_Liquid_Addr[selected_liquid], Remaining_Liquid[selected_liquid]);
  current_screen = "Home";
  isDosing = false; 
  DisplayHomeScreen();
}
