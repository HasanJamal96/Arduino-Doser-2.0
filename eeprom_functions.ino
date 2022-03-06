void writeIntIntoEEPROM(int addr, int number){ 
  EEPROM.write(addr, number >> 8);
  EEPROM.write(addr + 1, number & 0xFF);
}

int readIntFromEEPROM(int addr){
  byte byte1 = EEPROM.read(addr);
  byte byte2 = EEPROM.read(addr + 1);
  return (byte1 << 8) + byte2;
}


void writeFloatIntoEEPROM(int addr, float number){
  EEPROM.put(addr, number);
}

float readFloatFromEEPROM(int addr){
  static float n;
  EEPROM.get(addr, n);
  return n;
}

void writeStringToEEPROM(int addr, const String &strToWrite){
  byte len = strToWrite.length();
  for (int i = 0; i < len; i++){
    EEPROM.update(addr + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addr, int len){
  char read_data[len];
  for (int i = 0; i < len; i++){
    read_data[i] = EEPROM.read(addr + i);
  }
  return String(read_data);
}

void ReadSchedulesFromEEPROM(){
  for (uint8_t x=0; x<7; x++){
    for (uint8_t i=0; i<3; i++){
      Dose_Shedules[x][i] = readStringFromEEPROM(Dose_Sched_Addr[x][i], 10);
    } 
  }
}


void ReadNamesFromEEPROM(){
  for (uint8_t x=0; x<7; x++){
    LiquidNames[x] = readStringFromEEPROM(LiquidNameAddr[x], 12);
    uint8_t index = LiquidNames[x].indexOf(';');
    LiquidNames[x] = LiquidNames[x].substring(0,index);
  }
}
