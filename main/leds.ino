void ClearLeds(){
  digitalWrite(LEDlatch, LOW);
  shiftOut(LEDdata, LEDclock, LSBFIRST, 00000000);
  shiftOut(LEDdata, LEDclock, LSBFIRST, 00000000);
  digitalWrite(LEDlatch, HIGH);
}

void SetLeds(byte LowLeds, byte HighLeds){
  digitalWrite(LEDoe, LOW);
  digitalWrite(LEDlatch, LOW);
  shiftOut(LEDdata, LEDclock, MSBFIRST, HighLeds);
  shiftOut(LEDdata, LEDclock, MSBFIRST, LowLeds);
  digitalWrite(LEDlatch, HIGH);
}


void BlinkLeds(bool state){
  if(state){
    byte l = 0;
    byte h = 0;
  
    for(uint8_t i=0; i<8; i++){
      if((Bled1-1) < 8){
        if((Bled1-1) == i)
          bitSet(l, i);
      }
      else{
        if((Bled1-1) == i+8)
          bitSet(h, i);
      }
      if((Bled2-1) < 8){
        if((Bled2-1) == i)
          bitSet(l, i);
      }
      else{
        if((Bled2-1) == i+8)
          bitSet(h, i);
      }
    }
    SetLeds(l,h);
  }
  else{
    ClearLeds();
  }
}

void FadeLeds(){
  if(Fade_Pwm > 250)
    fade_state = true;
  else if(Fade_Pwm < 5){
    fade_state = false;
    generateRandom = true;
  }

  if(!fade_state)
    Fade_Pwm += 5;
  else
    Fade_Pwm -= 5;
  analogWrite(LEDoe, Fade_Pwm);
}

uint8_t bit_set = 0;
uint8_t bit_set_x = 0;
uint8_t complete_sec = 0;
byte chase_l = 0;
byte chase_h = 0;


void ChaseLeds(){
  digitalWrite(LEDoe, LOW);
  chase_l = 0;
  chase_h = 0;
  if(bit_set < 8){
    bitSet(chase_l, bit_set);
    bitSet(chase_h, 7-bit_set);
    bit_set += 1;
  }
  else{
    bit_set = chase_l = 0;
    chase_h = 0;
  }
  SetLeds(chase_l, chase_h);
}


void Chase2(){
  digitalWrite(LEDoe, LOW);
  if(bit_set < 8){
    bitSet(chase_l, bit_set);
    bit_set_x = chase_h = 0;
    bit_set += 1;
  }
  else{
    if(bit_set_x < 8){
      bitSet(chase_h, bit_set_x);
      bit_set_x += 1;
    }
    else{
      bit_set = 0;
      bit_set_x = 0;
      chase_l = 0;
    }
  }
  SetLeds(chase_l, chase_h);
}


void LedsLoopCode(){
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
