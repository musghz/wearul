/// @brief Read the whoami register for the LSM9DS1 accel/gyro
///
/// Need to desolder the ADDR jumper. Use desoldering braid and lots
/// of flux!
///
/// Created 7 Nov 2019
/// 
/// @author Mustafa Ghazi


#include <SPI.h> 
#define LSM6DS3_WHO_AM_I_REG_ADDR    0x0F
#define LSM6DS3_WHO_AM_I_REG_VAL     0x69
#define CS_PIN 8

void setup() {

  uint8_t myDat = 0;
  Serial.begin(115200);
  delay(3000);
  Serial.println("Setting up SPI for LSM6DS3");
  setupSPI(CS_PIN);  
  delay(1000);
  Serial.println("Attempting to connect to LSM6DS3");
  
  myDat = readOneByte(CS_PIN, LSM6DS3_WHO_AM_I_REG_ADDR);
  if (myDat == LSM6DS3_WHO_AM_I_REG_VAL) {
    Serial.print("LSM6DS3 detected on pin: ");
    Serial.println(CS_PIN);
  } else {
    Serial.print("LSM6DS3 not detected on pin: ");
    Serial.println(CS_PIN);
    
  }

  Serial.print("WHO_AM_I register value is: 0x");
  Serial.println(myDat, HEX);
  
  
  


}

void loop() {
  // put your main code here, to run repeatedly:

}

void setupSPI(int csPin) {  
  
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  SPI.begin();
  
}

uint8_t readOneByte(int csPin, uint8_t addr) {

  uint8_t myVal = 0;

  // Maximum SPI frequency is 10MHz, could divide by 2 here:
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // mode 0 for teensy 3.1
  //SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3)); // mode 3 for atmega328
  
  digitalWrite(csPin, LOW);
  SPI.transfer(addr | 0x80); // highest bit is 1 for read, 0 for write
  myVal = SPI.transfer(0x00);
  digitalWrite(csPin, HIGH);

  SPI.endTransaction();
  return myVal;
}

