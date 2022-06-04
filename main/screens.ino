uint8_t selected_menu = 0;
uint8_t scroll_pos = 0;
uint8_t selected_liquid_menu = 0;
uint8_t last_selected_liquid = 0;
String new_vol = "";
String editing = "";
uint8_t cur_pos = 0;
uint8_t cur_pos_x = 0, cur_pos_y = 0;
String h = "", m = "", s = "", d = "", mth = "", y = "";


void ClearLCD(uint8_t Srow, uint8_t Scol, uint8_t Erow, uint8_t Ecol){
  for(uint8_t i=Srow; i<=Erow; i++){
    lcd.setCursor(Scol, i);
    for(uint8_t x=Scol; x<=Ecol; x++){
      lcd.print(" ");
    }
  }
}


void UpdateRTC(){
  ClearLCD(0,9,1,19);
  myRTC.updateTime();
  lcd.setCursor(9,0);

  lcd.print(myRTC.hours);
  lcd.print(":");
  lcd.print(myRTC.minutes);
  lcd.print(":");
  lcd.print(myRTC.seconds);
  lcd.setCursor(9,1);
  lcd.print(myRTC.dayofmonth);
  lcd.print(":");
  lcd.print(myRTC.month);
  lcd.print(":");
  lcd.print(myRTC.year);
}

void UpdateDHT(){
  if(read_dht()){
    ClearLCD(2,9,3,18);
    lcd.setCursor(9,2);
    lcd.print(temperature);
    lcd.print(" ");
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(9,3);
    lcd.print(humidity);
    lcd.print(" rH%");
  }
}

void UpdateCapacityBars(){
  ClearLCD(0,0,3,6);
  for(uint8_t i=0; i<7; i++){
    lcd.setCursor(i,3);
    for(uint8_t x=0; x<30; x++){
      if(Remaining_Liquid[i] > x*10 && Remaining_Liquid[i] <= (x*10)+10){
        if(x<7)
          lcd.write(byte(x));
        else if(x<14){
          lcd.write(byte(6));
          lcd.setCursor(i,2);
          lcd.write(byte(x-7));
        }
        else if(x<21){
          lcd.write(byte(6));
          lcd.setCursor(i,2);
          lcd.write(byte(6));
          lcd.setCursor(i,1);
          lcd.write(byte(x-14));
        }
        else if(x<30){
          lcd.write(byte(6));
          lcd.setCursor(i,2);
          lcd.write(byte(6));
          lcd.setCursor(i,1);
          lcd.write(byte(6));
          lcd.setCursor(i,0);
          lcd.write(byte(x-21));
          if(x > 27)
            lcd.write(byte(6));
        }
      }
    }
  }
}


void DisplayHomeScreen(){
  last_screen = current_screen = "Home";
  LEDs_Status = "";
  ClearLeds();
  lcd.clear();
  UpdateCapacityBars();
  UpdateRTC();
  UpdateDHT();
}

void ReadHomeScreen(){
  char key = GetKey();
  if(key != '-'){
    if(last_screen == "menu" || last_screen == "Home"){
        generateRandom = true;
        DisplayMenuScreen();
    }
  }
}

void DisplayMenuScreen(){
  lcd.clear();
  last_screen = current_screen = "Menu";
  lcd.setCursor(0,selected_menu);
  lcd.print('>');
  lcd.setCursor(1,0);
  lcd.print("Time & Date");
  lcd.setCursor(1,1);
  lcd.print("Schedule");
  lcd.setCursor(1,2);
  lcd.print("Quick Dose");
  lcd.setCursor(1,3);
  lcd.print("Flush");
}


void ReadMenuScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      LEDs_Status = "Fade";
      ClearLeds();
      if(selected_menu == 0){
        cur_pos_x =  0;
        cur_pos_y = 6;
        SetLeds(255,255);
        setRTCBeforeEdit();
        DisplayRTCScreen();
      }
      else if(selected_menu == 1){
        selected_liquid = 0;
        scroll_pos = 0;
        DisplayScheduleScreen();
      }
      else if(selected_menu == 2){
        selected_liquid = 0;
        DisplayScheduleScreen();
      }
      else
        DisplayFluchScreen();
    }
    else if(key == 'A'){
      selected_menu -= 1;
      if(selected_menu > 200)
        selected_menu = 3;
      DisplayMenuScreen();
    }
    else if(key == 'B'){
      selected_menu += 1;
      if(selected_menu > 3)
        selected_menu = 0;
      DisplayMenuScreen();
    }
    else if(key == '*'){
      selected_menu = 0;
      DisplayHomeScreen();
    }
  }
}


void setRTCBeforeEdit(){
  h = "", m = "", s = "", d = "", mth = "", y = "";
  if(myRTC.hours == 0) h = "00";
  else h = String(myRTC.hours);
  if(h.length() < 2) '0' + h;
    
  if(myRTC.minutes == 0) m = "00";
  else m = String(myRTC.minutes);
  if(m.length() < 2) '0' + m;
    
  if(myRTC.seconds == 0) s = "00";
  else s = String(myRTC.seconds);
  if(s.length() < 2) '0' + s;

  if(myRTC.dayofmonth == 0) d = "00";
  else d = String(myRTC.dayofmonth);
  if(d.length() < 2) '0' + d;
    
  if(myRTC.month == 0) mth = "00";
  else mth = String(myRTC.month);
  if(mth.length() < 2) '0' + mth;
    
  if(myRTC.year == 0) y = "00";
  else y = String(myRTC.year);

  
}


void DisplayRTCScreen(){
  last_screen = current_screen = "TD";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: " + h + ":" + m + ":" + s);
  lcd.setCursor(0, 1);
  lcd.print("Date: " + d + ":" + mth + ":" + y);
  lcd.cursor();
  lcd.setCursor(cur_pos_y, cur_pos_x);
}


void ReadRTCScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      int myear = mth.substring(2,4).toInt();
      int mmonth = mth.toInt();
      int dow = ((myear /4) + d.toInt() + mth_key[mmonth-1] + 4 + myear) % 4;
      myRTC.setDS1302Time(s.toInt(), m.toInt(), h.toInt(), dow, d.toInt(), mmonth, y.toInt());
      myRTC.updateTime();
      setTime(h.toInt(), m.toInt(), s.toInt(), d.toInt(), mmonth,  y.toInt());
      lcd.noCursor();
      DisplayMenuScreen();
    }
    else if(key == 'A'){
      cur_pos_x -= 1;
      if(cur_pos_x > 200)
        cur_pos_x = 1;
      DisplayRTCScreen();
    }
    else if(key == 'B'){
      cur_pos_x += 1;
      if(cur_pos_x > 1)
        cur_pos_x = 0;
      DisplayRTCScreen();
    }
    else if(key == 'C'){
      cur_pos_y += 1;
      if(cur_pos_x == 0){
        if(cur_pos_y > 13)
          cur_pos_y = 6;
      }
      else{
        if(cur_pos_y > 15)
          cur_pos_y = 6;
      }
      if(cur_pos_y == 8 || cur_pos_y == 11)
        cur_pos_y += 1;
      DisplayRTCScreen();
    }
    else if(key == 'D'){
      cur_pos_y -= 1;
      if(cur_pos_y < 6){
        if(cur_pos_x == 0)
          cur_pos_y = 13;
        else
          cur_pos_y = 15;
      }
      if(cur_pos_y == 8 || cur_pos_y == 11)
          cur_pos_y -= 1;
      DisplayRTCScreen();
    }
    else if(key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0'){
      int num = key - '0';
      if(cur_pos_x == 0){
        if(cur_pos_y == 6){
          if(num < 3){
            h = String(num) + h[1];
          }
        }
        else if(cur_pos_y == 7){
          if((h[0] - '0') < 2){
            h = h[0] + String(num);
          }
          else if(num < 5){
            h = h[0] + String(num);
          }              
        }
        else if(cur_pos_y == 9){
          if(num < 6){
            m = String(num) + m[1];
          }
        }
        else if(cur_pos_y == 10){
          m = m[0] + String(num);
        }
        else if(cur_pos_y == 12){
          if(num < 6)
            s = String(num) + s[1];
        }
        else if(cur_pos_y == 13){
          s = s[0] + String(num);
        }
      }
      if(cur_pos_x == 1){
        if(cur_pos_y == 6){
          if(num < 4){
            d = String(num) + d[1];
          }
        }
        else if(cur_pos_y == 7){
          d = d[0] + String(num);
        }
        else if(cur_pos_y == 9){
          if(num < 2){
            mth = String(num) + mth[1];
          }
        }
        else if(cur_pos_y == 10){
          if(mth[0]-'0' > 1){
            if(num < 3)
              mth = mth[0] + String(num);
          }
          else
            mth = mth[0] + String(num);
        }
        else if(cur_pos_y == 12)
          y = String(num) + String(y[1]-'0') + String(y[2]-'0') + String(y[3]-'0');
        else if(cur_pos_y == 13)
          y = String(y[0]-'0') + String(num) + String(y[2]-'0') + String(y[3]-'0');
        else if(cur_pos_y == 14)
          y = String(y[0]-'0') + String(y[1]-'0') + String(num) + String(y[3]-'0');
        else if(cur_pos_y == 15)
          y = String(y[0]-'0') + String(y[1]-'0') + String(y[2]-'0') + String(num);
      }
      DisplayRTCScreen();
    }
    else if(key == '*'){
      LEDs_Status = "";
      lcd.noCursor();
      DisplayMenuScreen();
    }
  }
}


void DisplayScheduleScreen(){
  lcd.clear();
  last_screen = current_screen = "Schedule";
  lcd.setCursor(0,selected_liquid);
  lcd.print('>');
  for(int i=scroll_pos; i<scroll_pos+4; i++){
    lcd.setCursor(1,i-scroll_pos);
    lcd.print(LiquidNames[i]);
  }
}


void getLiquidLeds(uint8_t SL){
  if(SL == 0){
    Bled1 = 3;
    Bled2 = 4;
  }
  else if(SL == 1){
    Bled1 = 5;
    Bled2 = 6;
  }
  else if(SL == 2){
    Bled1 = 7;
    Bled2 = 8;
  }
  else if(SL == 3){
    Bled1 = 9;
    Bled2 = 10;
  }
  else if(SL == 4){
    Bled1 = 11;
    Bled2 = 12;
  }
  else if(SL == 5){
    Bled1 = 13;
    Bled2 = 14;
  }
  else if(SL == 6){
    Bled1 = 1;
    Bled2 = 2;
  }
}


void ReadScheduleScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      last_selected_liquid = selected_liquid+scroll_pos;
      LEDs_Status = "Blink";
      getLiquidLeds(last_selected_liquid);
     
      if(selected_menu == 1)
        DisplayLiquidScreen();
      else
        DisplayQuickDoseScreen();
    }
    else if(key == 'A'){
      selected_liquid -= 1;
      if(selected_liquid > 200){
        selected_liquid = 0;
        scroll_pos -= 1;
        if(scroll_pos > 200){
          selected_liquid = 3;
          scroll_pos = 3;
        }
      }
      DisplayScheduleScreen();
    }
    else if(key == 'B'){
      selected_liquid += 1;
      if(selected_liquid > 3){
        selected_liquid = 3;
        scroll_pos += 1;
        if(scroll_pos > 3){
          scroll_pos = 0;
          selected_liquid = 0;
        }
      }
      DisplayScheduleScreen();
    }
    else if(key == '*'){
      DisplayMenuScreen();
    }  
  }
}


void DisplayLiquidScreen(){
  last_screen = current_screen = "Liquid";
  lcd.clear();
  lcd.setCursor(0,selected_liquid_menu);
  lcd.print('>');
  lcd.setCursor(1,0);
  lcd.print("Rename " + LiquidNames[last_selected_liquid]);
  lcd.setCursor(1,1);
  lcd.print("Edit Total");
  lcd.setCursor(1,2);
  lcd.print("Show Remaining");
  lcd.setCursor(1,3);
  lcd.print("Dosing Schedules");
}


String NewLiquidName = "";
uint8_t selected_schedule = 0;
uint8_t taps = 0;
uint8_t r_cursor_pos = 5;
unsigned long lastTap = 0;
int fourTapDuration = 2000;
int threeTapDuration = 1500;
int twoTapDuration = 800;
int oneTapDuration = 500;
char lastKey = '-';
bool caps = true;

void ReadLiquidScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      if(selected_liquid_menu == 0){
        NewLiquidName = "";
        r_cursor_pos = 5;
        lcd.cursor();
        DisplayRenameScreen();
      }
      else if(selected_liquid_menu == 1){
        cur_pos = 11 + new_vol.length();
        EditVolume();
      }
      else if(selected_liquid_menu == 2){
        cur_pos = 11 + new_vol.length();
        DisplayRemainingVolume();
      }
      else if(selected_liquid_menu == 3){
        selected_schedule = 0;
        DisplayDosingScreen();
      }
      selected_liquid_menu = 0;
      
    }
    else if(key == 'A'){
      selected_liquid_menu -= 1;
      if(selected_liquid_menu > 200)
        selected_liquid_menu = 3;
      DisplayLiquidScreen();
    }
    else if(key == 'B'){
      selected_liquid_menu += 1;
      if(selected_liquid_menu > 3)
        selected_liquid_menu = 0;
      DisplayLiquidScreen();
    }
    else if(key == '*'){
      LEDs_Status = "";
      ClearLeds();
      selected_liquid_menu = 0;
      DisplayScheduleScreen();
    } 
  }
}


void DisplayRenameScreen(){
  last_screen = current_screen = "Rename";
  lcd.clear();
  lcd.setCursor(6,0);
  lcd.print("Renaming");
  lcd.setCursor(0,1);
  lcd.print("Old: " + LiquidNames[last_selected_liquid]);
  lcd.setCursor(0,3);
  if(caps)
    lcd.print("Caps: ON");
  else
    lcd.print("Caps: off");
  lcd.setCursor(0,2);
  lcd.print("New: " + NewLiquidName);
}


void ReadRenameScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0'){
      if(key != lastKey){
        lastKey = key;
        taps = 1;
        lastTap = millis();
      }
      else if(millis - lastTap > 100){
        taps += 1;
        if(taps > 4)
          taps = 1;
        lastTap = millis();
      }
    }
    else if(key == '#'){
      lcd.noCursor();
      LiquidNames[last_selected_liquid] = NewLiquidName;
      NewLiquidName += ";";
      writeStringToEEPROM(LiquidNameAddr[last_selected_liquid], NewLiquidName);
      DisplayLiquidScreen();
    }
    else if(key == 'A'){
      if(caps)
        caps = false;
      else
        caps = true;
      DisplayRenameScreen();
    }
    else if(key == 'B'){
      NewLiquidName.remove(NewLiquidName.length()-1);
      DisplayRenameScreen();
    }
    else if(key == '*'){
      lastKey = '-';
      taps = 0;
      lcd.noCursor();
      DisplayLiquidScreen();
    }
  }
  if(taps > 0){
    if(taps == 1){
      if(millis() - lastTap > oneTapDuration){
        UpdateNameDisplay();
      }
    }
    else if(taps == 2){
      if(millis() - lastTap > twoTapDuration){
        UpdateNameDisplay();
      }
    }
    else if(taps == 3){
      if(millis() - lastTap > threeTapDuration){
        UpdateNameDisplay();
      }
    }
    else if(taps == 4){
      if(millis() - lastTap > fourTapDuration){
        UpdateNameDisplay();
      }
    }
  }
}

void UpdateNameDisplay(){
  if(NewLiquidName.length() < 13 && r_cursor_pos < 18){
    int index = lastKey-'0';
    String alphas = AlphabetMap[index-1];
    char nl;
    if(caps)
      NewLiquidName += alphas[taps-1];
    else{
      if(index != 9){
        if(taps < 3){
          nl = alphas[taps-1] + 32;
        }
        else
          nl = alphas[taps-1];
      }
      else{
        if(taps < 2){
          nl = alphas[taps-1] + 32;
        }
        else
          nl = alphas[taps-1];
      }
      NewLiquidName += nl;
    }
  }
  lastKey = '-';
  taps = 0;
  DisplayRenameScreen();
}


void EditVolume(){
  last_screen = current_screen = "EditTotal";
  lcd.clear();
  uint8_t len = LiquidNames[last_selected_liquid].length();
  lcd.setCursor(10-int(len/2),1);
  lcd.print(LiquidNames[last_selected_liquid]);
  lcd.setCursor(0,0);
  lcd.print("Enter Total Volume");
  lcd.setCursor(0,2);
  lcd.print("Old value: " + String(Total_Liquid[last_selected_liquid]));
  lcd.setCursor(0,3);
  lcd.cursor();
  lcd.print("New Value: ");
  lcd.setCursor(11,3);
  lcd.print(new_vol);
  lcd.setCursor(cur_pos,3);
}

void ReadEditVolume(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      Total_Liquid[last_selected_liquid] = new_vol.toFloat();
      Remaining_Liquid[last_selected_liquid] = new_vol.toFloat();
      writeFloatIntoEEPROM(Total_Liquid_Addr[last_selected_liquid], Total_Liquid[last_selected_liquid]);
      writeFloatIntoEEPROM(Remain_Liquid_Addr[last_selected_liquid], Remaining_Liquid[last_selected_liquid]);
      new_vol = "";
      lcd.noCursor();
      DisplayLiquidScreen();
    }
    else if(key == 'C'){
      if(cur_pos < 11 + new_vol.length()){
        cur_pos += 1;
        EditVolume();
      }
    }
    else if(key == 'D'){
      if(cur_pos > 11){
        cur_pos -= 1;
        EditVolume();
      }
    }
    else if(key == '*'){
      last_selected_liquid = 0;
      lcd.noCursor();
      new_vol = "";
      DisplayLiquidScreen();
    }
    else if(key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0'){
      uint8_t x = cur_pos-11;
      String nv = new_vol.substring(0,x) + key + new_vol.substring(x+1,-1);
      int maxi_val;
      if(last_selected_liquid == 6)
        maxi_val = 301;
      else
        maxi_val = 101;
      if(editing == "Total"){
        if(nv.toFloat() < maxi_val){
          new_vol = nv;
          cur_pos += 1;
          EditVolume();
        }
      }
      else{
        float maxi_vol = 100.0;
        if(last_selected_liquid > 5)
          maxi_vol = 300.00;
        if(nv.toFloat() > 0 && (nv.toFloat() <= maxi_vol)){
          new_vol = nv;
          cur_pos += 1;
          EditVolume();
        }
      }
    }
  }
}


void DisplayRemainingVolume(){
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Remaining Volume");
  uint8_t len = LiquidNames[last_selected_liquid].length();
  lcd.setCursor(10-int(len/2),1);
  lcd.print(LiquidNames[last_selected_liquid]);
  lcd.setCursor(0,2);
  lcd.print("Volume: " + String(Remaining_Liquid[last_selected_liquid]));
  
}

void ReadRemainingVolume(){
  char key = GetKey();
  if(key != '-'){
    if(key == '*'){
      selected_liquid_menu = 0;
      DisplayLiquidScreen();
    }
  }
}

void DisplayDosingScreen(){
  last_screen = current_screen = "Dosing";
  lcd.clear();
  uint8_t len = LiquidNames[last_selected_liquid].length();
  lcd.setCursor(10-int(len/2),0);
  lcd.print(LiquidNames[last_selected_liquid]);
  lcd.setCursor(0,selected_schedule+1);
  lcd.print('>');
  lcd.setCursor(1,1);
  lcd.print("Schedule 1");
  lcd.setCursor(1,2);
  lcd.print("Schedule 2");
  lcd.setCursor(1,3);
  lcd.print("Schedule 3");
}


void ReadDosingScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      DisplayEditDose();
    }
    else if(key == 'A'){
      selected_schedule -= 1;
      if(selected_schedule > 200)
        selected_schedule = 2;
      DisplayDosingScreen();
    }

    else if(key == 'B'){
      selected_schedule += 1;
      if(selected_schedule > 2)
        selected_schedule = 0;
      DisplayDosingScreen();
    }
    else if(key == '*'){
      selected_schedule = 0;
      DisplayLiquidScreen();
    }
  }
}


uint8_t selected_schedule_menu = 0;

void DisplayEditDose(){
  last_screen = current_screen = "Dosing2";
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print(LiquidNames[last_selected_liquid]);
  lcd.setCursor(15,0);
  lcd.print("S:" + String(selected_schedule+1));
  lcd.setCursor(0,selected_schedule_menu+1);
  lcd.print('>');
  lcd.setCursor(1,1);
  lcd.print("Currnet Schedule");
  lcd.setCursor(1,2);
  lcd.print("New Schedule");
  lcd.setCursor(1,3);
  lcd.print("Delete Schedule");
}


String NS = "";
uint8_t sche_cur_pos = 0;
String shed_drops = "";
uint8_t x_shed_pos = 2;
uint8_t drop_crs = 0;
const uint8_t dropLen = 3;
char NST = '0';


void ReadEditDose(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      NS = Dose_Shedules[last_selected_liquid][selected_schedule];
      if(selected_schedule_menu != 0){
        lcd.cursor();
        shed_drops = String(Drops[last_selected_liquid][selected_schedule]);
        NST = Schedule_Type[last_selected_liquid][selected_schedule];
        while(shed_drops.length() < 3)
          shed_drops = "0" + shed_drops;
      }
      sche_cur_pos = 2;
      x_shed_pos = 2;
      drop_crs = 7;
      DisplayEditSchedule();
    }
    else if(key == 'A'){
      selected_schedule_menu -= 1;
      if(selected_schedule_menu > 200)
        selected_schedule_menu = 2;
      DisplayEditDose();
    }
    else if(key == 'B'){
      selected_schedule_menu += 1;
      if(selected_schedule_menu > 2)
        selected_schedule_menu = 0;
      DisplayEditDose();
    }
    else if(key == '*'){
      selected_schedule = 0;
      selected_schedule_menu = 0;
      DisplayDosingScreen();
    }
  }
}



void DisplayEditSchedule(){
  last_screen = current_screen = "Dosing3";
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(LiquidNames[last_selected_liquid]);
  lcd.setCursor(17,0);
  lcd.print("S:" + String(selected_schedule+1));
  if(selected_schedule_menu == 0){
    lcd.setCursor(2,1);
    lcd.print("Current Schedule");
    lcd.setCursor(0,2);
    lcd.print("Drops: ");
    lcd.print(Drops[last_selected_liquid][selected_schedule]);
    lcd.setCursor(12,2);
    lcd.print("Type: ");
    NST = Schedule_Type[last_selected_liquid][selected_schedule];
    lcd.print(NST);
    lcd.setCursor(0,3);
    String sched = Dose_Shedules[last_selected_liquid][selected_schedule];
    lcd.print("H:" + sched.substring(0,2) + " M:" + sched.substring(3,5) + " S:" + sched.substring(6,8) + " DOW:" + sched.substring(9,10));
  }
  else if(selected_schedule_menu == 1){
    lcd.setCursor(3,1);
    lcd.print("New Schedule");
    lcd.setCursor(0,2);
    lcd.print("Drops: " + shed_drops);
    lcd.setCursor(12,2);
    lcd.print("Type: ");
    lcd.print(NST);
    lcd.setCursor(0,3);
    lcd.print("H:" + NS.substring(0,2) + " M:" + NS.substring(3,5) + " S:" + NS.substring(6,8) + " DOW:" + NS.substring(9,10));
    if(x_shed_pos == 3)
      lcd.setCursor(sche_cur_pos,3);
    else
      lcd.setCursor(drop_crs,2);
  }
  else if(selected_schedule_menu == 2){
    lcd.setCursor(7,1);
    lcd.print("Deleted");
    selected_schedule_menu = 0;
    lcd.noCursor();
    if(AlarmIDs[last_selected_liquid][selected_schedule] != 255){
      Alarm.disable(AlarmIDs[last_selected_liquid][selected_schedule]);
      AlarmIDs[last_selected_liquid][selected_schedule] = 255;
      Dose_Shedules[last_selected_liquid][selected_schedule] = "00,00,00:0";
      Drops[last_selected_liquid][selected_schedule] = 0;
      writeStringToEEPROM(Dose_Sched_Addr[last_selected_liquid][selected_schedule], "00,00,00:0");
      writeIntIntoEEPROM(Drops_Addr[last_selected_liquid][selected_schedule], 0);
      Schedule_Type[last_selected_liquid][selected_schedule] = '0';
      EEPROM.write(schedule_Type_Addr[last_selected_liquid][selected_schedule], '0');
    }
    Alarm.delay(1500);
    DisplayEditDose();
  }
}


void UpdateScheduleCursor(char which_way){
  if(which_way == 'C'){
    if(x_shed_pos == 3){
      if(sche_cur_pos == 2 || sche_cur_pos == 7 || sche_cur_pos == 12)
        sche_cur_pos += 1;
      else if(sche_cur_pos == 3 || sche_cur_pos == 8)
        sche_cur_pos += 4;
      else if(sche_cur_pos == 13)
        sche_cur_pos += 6;
      else if(sche_cur_pos == 19)
        sche_cur_pos = 2;
      lcd.setCursor(sche_cur_pos,3);
    }
    else{
      drop_crs += 1;
      if(drop_crs > 18)
        drop_crs = 7;
      else if(drop_crs > 9)
        drop_crs = 18;
      lcd.setCursor(drop_crs,2);
    }
  }
  else if(which_way == 'D'){
    if(x_shed_pos == 3){
      if(sche_cur_pos == 7 || sche_cur_pos == 12)
        sche_cur_pos -= 4;
      else if(sche_cur_pos == 3 || sche_cur_pos == 8 || sche_cur_pos == 13)
        sche_cur_pos -= 1;
      else if(sche_cur_pos == 19)
        sche_cur_pos -= 6;
      else if(sche_cur_pos == 2)
        sche_cur_pos = 19;
      lcd.setCursor(sche_cur_pos,3);
    }
    else{
      drop_crs -= 1;
      if(drop_crs < 7)
        drop_crs = 18;
      else if(drop_crs < 18 && drop_crs > 15)
        drop_crs = 9;
      lcd.setCursor(drop_crs,2);
    }
  }
}

void ReadDisplayEditSchedule(){
  char key = GetKey();
  if(key != '-'){
    if(selected_schedule_menu != 0){
      if(key == '#'){
        lcd.noCursor();
        Dose_Shedules[last_selected_liquid][selected_schedule] = NS;
        writeStringToEEPROM(Dose_Sched_Addr[last_selected_liquid][selected_schedule], NS);
        writeIntIntoEEPROM(Drops_Addr[last_selected_liquid][selected_schedule], shed_drops.toInt());
        int s_hrs = Dose_Shedules[last_selected_liquid][selected_schedule].substring(0,2).toInt();
        int s_min = Dose_Shedules[last_selected_liquid][selected_schedule].substring(3,5).toInt();
        int s_sec = Dose_Shedules[last_selected_liquid][selected_schedule].substring(6,8).toInt();
        uint8_t s_dow = Dose_Shedules[last_selected_liquid][selected_schedule].substring(9,10).toInt()-1;
        Schedule_Type[last_selected_liquid][selected_schedule] = NST;
        Drops[last_selected_liquid][selected_schedule] = shed_drops.toInt();
        if(s_dow > 7)
          s_dow = 0;
        if(AlarmIDs[last_selected_liquid][selected_schedule] != -1)
          Alarm.disable(AlarmIDs[last_selected_liquid][selected_schedule]);
        if(NST == '0')
          AlarmIDs[last_selected_liquid][selected_schedule] = Alarm.alarmOnce(weekDays[s_dow], s_hrs, s_min, s_sec, AlarmFunction[last_selected_liquid][selected_schedule]);
        else if(NST == '1')
          AlarmIDs[last_selected_liquid][selected_schedule] = Alarm.alarmRepeat(weekDays[s_dow], s_hrs, s_min, s_sec, AlarmFunction[last_selected_liquid][selected_schedule]);
        else if(NST == '2')
          AlarmIDs[last_selected_liquid][selected_schedule] = Alarm.alarmRepeat(s_hrs, s_min, s_sec, AlarmFunction[last_selected_liquid][selected_schedule]);
        
        ClearLCD(2,0,3,19);
        lcd.setCursor(7,2);
        if(AlarmIDs[last_selected_liquid][selected_schedule] == 255)
          lcd.print("Failed");
        else
          lcd.print("Saved");
        selected_schedule_menu = 0;
        Alarm.delay(2000);
        DisplayEditDose();
      }
      else if(key == 'A'){
        x_shed_pos -= 1;
        if(x_shed_pos < 2)
          x_shed_pos = 3;
        DisplayEditSchedule();
      }
      else if(key == 'B'){
        x_shed_pos += 1;
        if(x_shed_pos > 3)
          x_shed_pos = 2;
        DisplayEditSchedule();
      }
      else if(key == 'C'){
        UpdateScheduleCursor('C');
      }
      else if(key == 'D'){
        UpdateScheduleCursor('D');
      }
      else if(key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0'){
        if(x_shed_pos == 3){
          if(x_shed_pos == 3){
            if(sche_cur_pos == 2){
              if(key < '3')
                NS.setCharAt(0, key);
            }
            else if(sche_cur_pos == 3){
              if(NS.charAt(0) < '2')
                NS.setCharAt(1, key);
              else if(key < '5')
                NS.setCharAt(1, key);
            }
            else if(sche_cur_pos == 7){
              if(key < '6'){
                NS.setCharAt(3, key);
              }
            }
            else if(sche_cur_pos == 8){
              NS.setCharAt(4, key);
            }
            else if(sche_cur_pos == 12){
              if(key < '6')
                NS.setCharAt(6, key);
            }
            else if(sche_cur_pos == 13){
              NS.setCharAt(7, key);
            }
            else if(sche_cur_pos == 19){
              if(key < '8' && key > '0')
                NS.setCharAt(9, key);
            }
          }
        }
        else{
          if(drop_crs == 18){
            int k = key - '0';
            if(k < 3){
              NST = key;
            }
          }
          else{
            shed_drops.setCharAt(drop_crs-7, key);
            UpdateScheduleCursor('C');
          }
        }
        DisplayEditSchedule();
      }
    }
    if(key == '*'){
      selected_schedule_menu = 0;
      lcd.noCursor();
      DisplayEditDose();
    }
  }
}

uint8_t quik_menu_select = 0;
bool dosing_quick = false;
String QDQ = "";


void DisplayQuickDoseScreen(){
  last_screen = current_screen = "Quick";
  lcd.clear();
  lcd.setCursor(0,quik_menu_select+2);
  lcd.print('>');
  lcd.setCursor(10-(LiquidNames[last_selected_liquid].length()/2),0);
  lcd.print(LiquidNames[last_selected_liquid]);
  if(dosing_quick){
    lcd.setCursor(0,2);
    lcd.print("QuickDose in process");
  }
  else{
    lcd.setCursor(1,1);
    lcd.print("Drops set: ");
    lcd.print(QDQ.toInt());
    lcd.setCursor(1,2);
    lcd.print("Set Dosing quantity");
    lcd.setCursor(1,3);
    lcd.print("Start Quick Dose");
    
  }
}



void ReadQuickDoseScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      if(quik_menu_select == 0){
        QDQ = "   ";
        drop_crs = 7;
        lcd.cursor();
        EditDosingQuantity();
      }
      else{
        if(isScheduleRunning){
          lcd.clear();
          lcd.setCursor(2,0);
          lcd.print("Unable to start");
          lcd.setCursor(7,1);
          lcd.print("Dosing");
          lcd.setCursor(2,2);
          lcd.print("Schedule Running");
          Alarm.delay(2000);
          DisplayQuickDoseScreen();
          return;
        }
        DosingPhase = "2";
        DosingLiquid = last_selected_liquid;
        activeStepper = 0;
        if(last_selected_liquid != 6)
          activeStepper = 1;
        if(last_selected_liquid < 3 || last_selected_liquid == 6){
          StartDose(last_selected_liquid, 0);
        }
        else{
          Max_Dosing_Duration -= DC_MOTOR_DURATION;
          StartPhase2();
        }
        ProgressScreen();
      }
    }
    else if(key == 'A'){
      quik_menu_select -= 1;
      if(quik_menu_select > 250)
        quik_menu_select = 1;
      DisplayQuickDoseScreen();
    }
    else if(key == 'B'){
      quik_menu_select += 1;
      if(quik_menu_select > 1)
        quik_menu_select = 0;
      DisplayQuickDoseScreen();
    }
    else if(key == '*'){
      LEDs_Status = "";
      ClearLeds();
      scroll_pos = quik_menu_select = selected_liquid = 0;
      DisplayScheduleScreen();
    }
  }
}

void EditDosingQuantity(){
  last_screen = current_screen = "DosingQnty";
  lcd.clear();
  lcd.setCursor(10-(LiquidNames[last_selected_liquid].length()/2),0);
  lcd.print(LiquidNames[last_selected_liquid]);
  lcd.setCursor(0,1);
  lcd.print("Drops: " + QDQ);
  lcd.setCursor(drop_crs,1);
}

void ReadDosingQuantity(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      quik_menu_select = 0;
      qdq = QDQ.toInt();
      DisplayQuickDoseScreen();
      lcd.noCursor();
    }
    else if(key == 'C'){
      drop_crs += 1;
      if(drop_crs > 9)
        drop_crs = 7;
      lcd.setCursor(drop_crs,1);
    }
    else if(key == 'D'){
      drop_crs -= 1;
      if(drop_crs < 7)
        drop_crs = 9;
      lcd.setCursor(drop_crs,1);
    }
    else if(key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0'){
      QDQ.setCharAt(drop_crs-7, key);
      drop_crs += 1;
      if(drop_crs > 9)
        drop_crs = 7;
      EditDosingQuantity();
    }
    else if(key == '*'){
      quik_menu_select = 0;
      lcd.noCursor();
      DisplayQuickDoseScreen();
    }
  }
}


uint8_t flush_menu_select = 0;

void DisplayFluchScreen(){
  last_screen = current_screen = "Flush";
  lcd.clear();
  lcd.setCursor(8,0);
  lcd.print("Flush");
  lcd.setCursor(0,flush_menu_select+1);
  lcd.print('>');
  lcd.setCursor(1,0);
  lcd.print("Small Line");
  lcd.setCursor(1,1);
  lcd.print("Large Line");
}


void ReadFluchScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '#'){
      ClearLeds();
      LEDs_Status = "Chase";
      if(flush_menu_select == 0)
        activeStepper = 1;
      else
        activeStepper = 0;
      isFlush = true;
      Remaining_Dosing_Duration = Max_Dosing_Duration = FLUSH_CW + FLUSH_CCW;
      ProgressScreen();
      StartFlushingSequence();
    }
    else if(key == 'A'){
      flush_menu_select -= 1;
      if(flush_menu_select > 250)
        flush_menu_select = 1;
      DisplayFluchScreen();
    }
    else if(key == 'B'){
      flush_menu_select += 1;
      if(flush_menu_select > 1)
        flush_menu_select = 0;
      DisplayFluchScreen();
    }
    else if(key == '*'){
      LEDs_Status = "";
      ClearLeds();
      scroll_pos = quik_menu_select = 0;
      DisplayMenuScreen();
    }
  }
}

void ProgressScreen(){
  last_screen = current_screen = "Progress";
  lcd.clear();
  if(isDosing){
    lcd.setCursor(1,0);
    lcd.print("Dosing in progress");
  }
  else{
    lcd.setCursor(2,0);
    lcd.print("Flush in progress");
  }
  lcd.setCursor(0,2);
  lcd.print("Press ESC to Cancel");
}

void updateProgressBar(){
  /*ClearLCD(1,1,1,12);
  lcd.setCursor(8,1);
  uint32_t prog = millis() - DoseStartTime;
  lcd.print(map(prog, 0, Max_Dosing_Duration, 0, 100));
  lcd.print(" %");*/
}

void ReadProgressScreen(){
  char key = GetKey();
  if(key != '-'){
    if(key == '*'){
      quik_menu_select = 0;
      selected_menu = last_selected_liquid = 0;
      QDQ = "";
      EndDose(false);
    }
  }
}
