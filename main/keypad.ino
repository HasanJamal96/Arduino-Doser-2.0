const byte ROWS = 4; 
const byte COLS = 4; 

const char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

const byte rowPins[ROWS] = {46, 47, 48, 49}; 
const byte colPins[COLS] = {50, 51, 52, 53};

Keypad KP = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void SetKeypadParams(){
  KP.setDebounceTime(50);
}

char GetKey(){
  char customKey = KP.getKey();
  if(customKey)
    return customKey;
  else
    return '-';
}
