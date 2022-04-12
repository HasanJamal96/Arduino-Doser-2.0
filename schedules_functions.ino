void Dosing(){
  if(isDosing){
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
        Serial.print("Phase 2 completed after ");
        Serial.print(millis()-DosePhase2);
        Serial.println(" ms");
        Serial.println("Phase 3 started clear");
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
  DosingTime = qdq * 1104;
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
  Serial.println("Flushing started");
}


void Flushing(){
 if(DosingPhase == "4"){
    if(millis() - DosePhase4 >= 36000){
      ActivateStepper(activeStepper, 0);
      DosingPhase = "5";
      DosePhase5 = millis();
      Serial.println("Phase 5 started");
    }
  }
  else if(DosingPhase == "5"){
    if(millis() - DosePhase5 >= 72000){
      EndDose(true);
    }
  }
}

void EndDose(bool Normal){
  DeactivateStepper(activeStepper);
  digitalWrite(MOSFET_PINS[7], LOW);
  float usedDrops = 0;
  current_screen = "Home";
  isDosing = false; 
  DosingPhase = "-1";
  if(!isFlush){
    if(!Normal){
      DosingTime = millis() - DoseStartTime;
    }
    usedDrops = (DosingTime/1104) * 0.04;
    Remaining_Liquid[DosingLiquid] -= usedDrops;
    writeFloatIntoEEPROM(Remain_Liquid_Addr[DosingLiquid], Remaining_Liquid[DosingLiquid]);
  }
  DisplayHomeScreen();
}
