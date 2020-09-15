/// @file atmega328_fovi_headless_demo_V01
///
/// Adapted from atmega328_pwm_freq_mod_simple_V02
///
///
///
///
///
///
///
///

// PWM out of 120
// out of 3.7, 1.50, 2.25, 3.0
// 41% 61%, 81%,
// for 150, 62, 92, 138
#define LOW_INTENSITY_PWM 62
#define MEDIUM_INTENSITY_PWM 92
#define HIGH_INTENSITY_PWM 138

void setup() {

  // these use timer 1
  analogWrite(9, 128-64);
  analogWrite(10, 128+64);  
  modifyTimer1(31000);
  // these use timer 2
  analogWrite(11, 25);
  analogWrite(3, 255-25);
  modifyTimer2(31);
  setIntensityAll(0); // turn all off

}

void loop() {

  setIntensityAll(LOW_INTENSITY_PWM);
  delay(3000);
  setIntensityAll(MEDIUM_INTENSITY_PWM);
  delay(3000);
  setIntensityAll(HIGH_INTENSITY_PWM);
  delay(3000);
  setIntensityAll(0);
  delay(3000);
  while(1) {
    
  }

  

}

void setIntensityAll(uint8_t pwmVal) {

  analogWrite(9, pwmVal);
  analogWrite(10, pwmVal);
  analogWrite(11, pwmVal);
  analogWrite(3, pwmVal);
  
}

/// Warning: This will mess up timekeeping functions like delay(), millis(), micros()
void modifyTimer0(uint16_t freq) {

  
  //----- PWM frequency for D5 & D6 -----
  //Timer0 divisor = 1, 8, 64, 256, 1024
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // 62.5KHz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // 7.8KHz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // 976Hz (default)
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // 244Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // 61Hz
  
}

// @param freq PWM frequency (Hz) 31000, 3900, 490, 123, 31
void modifyTimer1(uint16_t freq){

  //----- PWM frequency for D9 & D10 -----
  //Timer1 divisor = 2, 16, 128, 512, 2048
  if (freq == 31000) {
    TCCR1B = TCCR1B & B11111000 | B00000001;    // 31KHz
  } else if (freq == 3900) {
    TCCR1B = TCCR1B & B11111000 | B00000010;    // 3.9KHz
  } else if (freq == 490) {
    TCCR1B = TCCR1B & B11111000 | B00000011;    // 490Hz (default)
  } else if (freq == 123) {
    TCCR1B = TCCR1B & B11111000 | B00000100;    // 122.5Hz
  } else if (freq == 31) {
    TCCR1B = TCCR1B & B11111000 | B00000101;    // 30.6Hz
  }
}


// @param freq PWM frequency (Hz) 31000, 3900, 980, 490, 245, 123, 31
void modifyTimer2(uint16_t freq){

  //----- PWM frequency for D3 & D11 -----
  //Timer2 divisor = 2, 16, 64, 128, 512, 2048

  if (freq == 31000) {
    TCCR2B = TCCR2B & B11111000 | B00000001;    // 31KHz
  } else if (freq == 3900) {
    TCCR2B = TCCR2B & B11111000 | B00000010;    // 3.9KHz
  }else if (freq == 980) {
    TCCR2B = TCCR2B & B11111000 | B00000011;    // 980Hz
  } else if (freq == 490) {
    TCCR2B = TCCR2B & B11111000 | B00000100;    // 490Hz (default)
  } else if (freq == 245) {
    TCCR2B = TCCR2B & B11111000 | B00000101;    // 245Hz
  } else if (freq == 123) {
    TCCR2B = TCCR2B & B11111000 | B00000110;    // 122.5Hz
  } else if (freq == 31) {
    TCCR2B = TCCR2B & B11111000 | B00000111;    // 30.6Hz
  }
}
