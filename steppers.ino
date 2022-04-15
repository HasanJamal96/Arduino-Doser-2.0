void ActivateStepper(uint8_t ID, uint8_t DIR){
  digitalWrite(SteppersEN[ID], LOW);
  if(DIR == 1)
    digitalWrite(SteppersDIR[ID], HIGH);
  else
    digitalWrite(SteppersDIR[ID], LOW);
  analogWrite(SteppersSTEP[ID], 125);
}


void DeactivateStepper(uint8_t ID){
  digitalWrite(SteppersEN[ID], HIGH);
  analogWrite(SteppersSTEP[ID], 0);
}
