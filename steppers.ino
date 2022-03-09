

int8_t activeStepper = -1

void ActivateStepper(uint8_t ID, uint8_t DIR){
  digitalWrite(SteppersEN[ID], HIGH);
  activeStepper = ID;
  if(DIR == 1){
    digitalWrite(SteppersDIR[ID], HIGH);
  }
  else
    digitalWrite(SteppersDIR[ID], LOW);
}


void DeactivateStepper(uint8_t ID){
  digitalWrite(SteppersEN[ID], LOW);
  activeStepper = -1
}
