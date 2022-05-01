void StartDose(uint8_t L, uint8_t by){
  Serial.println("Dosing sequence start");
  Accel_DC = true;
  ClearLeds();
  dc_speed = 0;
  DosingPhase = "1";
  LEDs_Status = "Chase2";
  if(L < 3)
    which_dc = L;
  else if(L  == 6)
    which_dc = 3;
  if(by == 0){
    DosingTime = qdq * one_drop_time;
    isDosing = true;
  }
  else{
    DosingTime = Drops[Running_Schedule_Liquid][Running_Schedule] * one_drop_time;
  }
  if(L < 3 || L == 6)
    Max_Dosing_Duration = DC_MOTOR_DURATION + PRIME_TIME + CLEAR_TIME + FLUSH_CW + FLUSH_CCW + DosingTime;
  else
    Max_Dosing_Duration = PRIME_TIME + CLEAR_TIME + FLUSH_CW + FLUSH_CCW + DosingTime;
  Remaining_Dosing_Duration = Max_Dosing_Duration;
  DoseStartTime = millis();
}

void Dosing(){
  if(isDosing){
    Remaining_Dosing_Duration = Max_Dosing_Duration - (millis() - DoseStartTime);
    if(DosingPhase == "1"){
      if(Accel_DC){
        if(millis() - last_accel >= ACCEL_AFTER){
          AccelerateDCMotor();
          last_accel = millis();
        }
      }
      else if(millis() - DoseStartTime >= DC_MOTOR_DURATION){
        digitalWrite(DC_EN_Pins[which_dc], 0);
        Serial.println("DC Stopped");
        StartPhase2();
      }
    }
    else if(DosingPhase == "2"){
      if(millis() - DosePhase2 >= PRIME_TIME + DosingTime){
        ActivateStepper(activeStepper, 0);
        DosingPhase = "3";
        DosePhase3 = millis();
        Serial.println("Clear");
      }
    }
    else  if(DosingPhase == "3"){
      if(millis() - DosePhase3 >= CLEAR_TIME){
        DeactivateStepper(1);
        digitalWrite(MOSFET_PINS[DosingLiquid], LOW);
        StartFlushingSequence();
      }
    }
    else
        Flushing();
  }
}


void AccelerateDCMotor(){
  if(dc_speed < 155)
    dc_speed += 5;
  else
    Accel_DC = false;
  analogWrite(DC_EN_Pins[which_dc], dc_speed);
  Serial.print("Accelerating DC: ");
  Serial.println(which_dc + 1);
}

void StartPhase2(){
  digitalWrite(MOSFET_PINS[DosingLiquid], HIGH);
  getLiquidLeds(DosingLiquid);
  LEDs_Status = "Blink";
  DosingPhase = "2";
  DosePhase2 = millis();
  Serial.println("Prime and Dosing started");
  ActivateStepper(activeStepper, 1);
}


void StartFlushingSequence(){
  DosingPhase = "4";
  ClearLeds();
  LEDs_Status = "Chase";
  ActivateStepper(activeStepper, 1);
  DosePhase4 = millis();
  digitalWrite(MOSFET_PINS[7], HIGH);
  Serial.println("Flushing CW");
}


void Flushing(){
 if(DosingPhase == "4"){
    if(millis() - DosePhase4 >= FLUSH_CW){
      ActivateStepper(activeStepper, 0);
      DosingPhase = "5";
      DosePhase5 = millis();
      Serial.println("Flushing CCW");
    }
  }
  else if(DosingPhase == "5"){
    if(millis() - DosePhase5 >= FLUSH_CCW){
      EndDose(true);
    }
  }
}

void EndDose(bool Normal){
  Serial.println("End Dosing");
  DeactivateStepper(activeStepper);
  digitalWrite(MOSFET_PINS[7], LOW);
  float usedDrops = 0;
  current_screen = "Home";
  isDosing = false; 
  DosingPhase = "-1";
  if(!isFlush){
    if(!Normal){
      if(DosingLiquid < 3 || DosingLiquid == 6){
        DosingTime = millis() - DoseStartTime - DC_MOTOR_DURATION - PRIME_TIME;
        if(DosingTime > 0){
          usedDrops = (DosingTime/one_drop_time) * one_drop_ml;
        }
      }
    }
    else
      usedDrops = (DosingTime/one_drop_time) * one_drop_ml;
    Remaining_Liquid[DosingLiquid] -= usedDrops;
    writeFloatIntoEEPROM(Remain_Liquid_Addr[DosingLiquid], Remaining_Liquid[DosingLiquid]);
  }
  DisplayHomeScreen();
}



void L1_S1(){
  if(!isScheduleRunning){
    Running_Schedule = 0;
    Running_Schedule_Liquid = 1;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 1 of Liquid 1");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L1_S2(){
  if(!isScheduleRunning){
    Running_Schedule = 1;
    Running_Schedule_Liquid = 1;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 2 of Liquid 1");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L1_S3(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 1;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 3 of Liquid 1");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}


void L2_S1(){
  if(!isScheduleRunning){
    Running_Schedule = 0;
    Running_Schedule_Liquid = 1;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 1 of Liquid 2");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L2_S2(){
  if(!isScheduleRunning){
    Running_Schedule = 1;
    Running_Schedule_Liquid = 1;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 2 of Liquid 2");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L2_S3(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 1;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 3 of Liquid 2");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}


void L3_S1(){
  if(!isScheduleRunning){
    Running_Schedule = 0;
    Running_Schedule_Liquid = 2;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 1 of Liquid 3");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L3_S2(){
  if(!isScheduleRunning){
    Running_Schedule = 1;
    Running_Schedule_Liquid = 2;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 2 of Liquid 3");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L3_S3(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 2;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 2 of Liquid 3");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}


void L4_S1(){
  if(!isScheduleRunning){
    Running_Schedule = 0;
    Running_Schedule_Liquid = 3;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 1 of Liquid 4");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L4_S2(){
  if(!isScheduleRunning){
    Running_Schedule = 1;
    Running_Schedule_Liquid = 3;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 2 of Liquid 4");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L4_S3(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 3;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 3 of Liquid 4");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}


void L5_S1(){
  if(!isScheduleRunning){
    Running_Schedule = 0;
    Running_Schedule_Liquid = 4;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 1 of Liquid 5");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L5_S2(){
  if(!isScheduleRunning){
    Running_Schedule = 1;
    Running_Schedule_Liquid = 4;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 2 of Liquid 5");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L5_S3(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 4;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 3 of Liquid 5");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}


void L6_S1(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 5;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 1 of Liquid 6");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L6_S2(){
  if(!isScheduleRunning){
    Running_Schedule = 1;
    Running_Schedule_Liquid = 5;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 2 of Liquid 6");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L6_S3(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 5;
    StartSchedule = true;
    activeStepper = 1;
    Serial.println("Starting Schedule 3 of Liquid 6");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}


void L7_S1(){
  if(!isScheduleRunning){
    Running_Schedule = 0;
    Running_Schedule_Liquid = 6;
    StartSchedule = true;
    activeStepper = 0;
    Serial.println("Starting Schedule 1 of Liquid 7");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L7_S2(){
  if(!isScheduleRunning){
    Running_Schedule = 1;
    Running_Schedule_Liquid = 6;
    StartSchedule = true;
    activeStepper = 0;
    Serial.println("Starting Schedule 2 of Liquid 7");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}
void L7_S3(){
  if(!isScheduleRunning){
    Running_Schedule = 2;
    Running_Schedule_Liquid = 6;
    StartSchedule = true;
    activeStepper = 0;
    Serial.println("Starting Schedule 3 of Liquid 7");
  }
  else{
    Serial.println("Schedule alreeady running");
  }
}

function AlarmFunction[7][3] = {L1_S1, L1_S2, L1_S3,
                      L2_S1, L2_S2, L2_S3,
                      L3_S1, L3_S2, L3_S3,
                      L4_S1, L4_S2, L4_S3,
                      L5_S1, L5_S2, L5_S3,
                      L6_S1, L6_S2, L6_S3,
                      L7_S1, L7_S2, L7_S3,
                      };

void AttatchSchedules(){
  for (uint8_t x=0; x<7; x++){
    for (uint8_t i=0; i<3; i++){
      if(Dose_Shedules[x][i] != "00,00,00:0"){
        uint8_t s_hrs = Dose_Shedules[x][i].substring(0,2).toInt();
        uint8_t s_min = Dose_Shedules[x][i].substring(3,5).toInt();
        uint8_t s_sec = Dose_Shedules[x][i].substring(6,8).toInt();
        uint8_t s_dow = Dose_Shedules[x][i].substring(9,10).toInt();
        AlarmIDs[x][i] = Alarm.alarmRepeat(weekDays[s_dow], s_hrs, s_min, s_sec, AlarmFunction[x][i]);
      }
      else
        AlarmIDs[x][i] = 255;
    }
  }
  return;
}
