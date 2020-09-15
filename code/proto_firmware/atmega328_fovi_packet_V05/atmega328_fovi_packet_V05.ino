/// @brief New fovi device protocol over several bytes
///
/// For documentation purposes, this is FOVI protocol 0.3
///
/// Format is z[source][address][cmd0][cmd1][cmd2][cmd3][check high][check low]s. For example,
/// source=1, address=1, cmds 3,7,12,14, 
/// checksum high is (1+1+3+7+12+14)=38/16=2
/// checksum low is (1+1+3+7+12+14)=38%16=6
/// So the numeric data will be 1,1,3,7,12,14,2,6. Characterized data would be 1,1,3,7,C,E,2,6.
/// So the packet will be z1137CE26\n.
///
/// Packet is always 10 bytes, including the \n character. 
/// -header: z
/// -source: 1 (master) or 0 (slave)
/// -address: 0,1,2,3,4,5,6,7,8,9,A,B,C,D,E, or F
/// -command 0: 0,1,2,3,4,5,6,7,8,9,A,B,C,D,E, or F
/// -command 1: 0,1,2,3,4,5,6,7,8,9,A,B,C,D,E, or F
/// -command 2: 0,1,2,3,4,5,6,7,8,9,A,B,C,D,E, or F
/// -command 3: 0,1,2,3,4,5,6,7,8,9,A,B,C,D,E, or F
/// -checksum high: (source + address + cmd0 + cmd1 + cmd2 + cmd3) / 16
/// -checksum low: (source + address + cmd0 + cmd1 + cmd2 + cmd3) % 16
/// -terminator: s 
///
/// Bonus protocol:
/// Generic commands for a generic robot. Just keywords followed by 
/// numbers of up to 3 digits.
/// 
///
/// Created 30 Dec 2019 (V03)
/// -multibyte packet (1 motor per packet)
/// -single byte check sum
/// Updated 12 Jan 2020 (V04)
/// -protocol version 0.2
/// -new packet (4 motors + motor address per packet)
/// -two byte checksum
/// -all numbers represented by ASCII chars
/// Updated 15 Feb 2020 (V05)
/// -protocol version 0.3
/// -changed terminator to 's'
/// -cleaned encoding/decoding functions
/// -changed buffer to local
/// -broke general command decoding
/// -fixed bug if header char is wrong
/// -added CTS pin
///
/// @author Mustafa Ghazi

#define CONTROL_INTERVAL_MS 1000
#define IN_PACKET_SIZE_BYTES 20
#define OUT_PACKET_SIZE_BYTES 10
#define HEADER_CHAR 'z'
#define TERMINATION_CHAR 's'
#define LOCAL_SLAVE_NODE_ADDRESS 1
// to indicate MCU is ready to receive serial data
#define CTS_PIN 2

unsigned long controlTime = 0;
char incoming = 'N';
 // [source][address][cmd0][cmd1][cmd2][cmd3][check high][check low]
uint8_t inIdx = 0, dataSize = 0;
uint8_t waitForHeaderFlag = 1; // 1: waiting for header byte, 0: waiting for data byte
uint8_t rawDataAvailableFlag = 0; // 1: raw data available, try parsing, 0: raw data not available, do not parse
uint8_t parsedDataAvailableFlag = 0; // 1: parsed data available, 0: parsedData not available

void getCommands();
void clearPktArray(char thisPkt[], int len);
void printPktArrayEncoded(char thisPkt[]);
void printPktArrayDecoded(int slaveAddress, int command0, int command1, int command2, int command3);
void encodeMasterToSlave(int slaveAddress, int command0, int command1, int command2, int command3, char outPkt[]);
void encodeSlaveToMaster(int slaveAddress, int command0, int command1, int command2, int command3, char outPkt[]);
int decode8CharMasterToSlave(int *slaveAddress, int *command0, int *command1, int *command2, int *command3, char inPkt[]);
int decode8CharSlaveToMaster(int *slaveAddress, int *command0, int *command1, int *command2, int *command3, char inPkt[]);
int foviCharToInt(char c);
char foviIntToChar(int x) ;

int counter=0;

void setup() {
  Serial.begin(9600);
  pinMode(CTS_PIN, OUTPUT);
  // tell BLE module that MCU is ready to receive serial data
  digitalWrite(CTS_PIN, LOW);
  
  int src = 16, addr1 = 16, cmd0 = 16, cmd1 = 16, cmd2 = 16, cmd3 = 16;
  

  char inPktArray[IN_PACKET_SIZE_BYTES], outPktArray[OUT_PACKET_SIZE_BYTES];

  controlTime = millis();

  while(1) {

    
    getCommands(inPktArray); // this loads single byte from serial to packeet buffer
    if(millis() - controlTime > CONTROL_INTERVAL_MS) {
      
      // parse only if raw data available
      if (rawDataAvailableFlag == 1) {
      
        if(decode8CharMasterToSlave(&addr1, &cmd0, &cmd1, &cmd2, &cmd3, inPktArray)) {
          
          // packet is complete and has valid data (except matching local slave address)
          // if slave address matches, set motors and send ack
          if (addr1 == LOCAL_SLAVE_NODE_ADDRESS) {
            clearPktArray(outPktArray, OUT_PACKET_SIZE_BYTES);
            encodeSlaveToMaster(addr1, cmd0, cmd1, cmd2, cmd3, outPktArray);
            printPktArrayEncoded(outPktArray);
            addr1 = 16; 
            
          }
        }

        rawDataAvailableFlag = 0; // clear after parsing packet bytes
      }
      
      
      // now do something...set motor commands
      
      //parseFoviCommands(&src, &addr,  cmd);
      //setFovCommands(src, addr, cmd);      
      //parseGeneralCommand();
      
      

      controlTime = millis();
    }

    delay(3);
  }

}

void loop() {
  
}

void getCommands(char arrayIncomingPacket[]){

  if (Serial.available()) {

    incoming = Serial.read();

    // three possibilities:
    // waiting for header and received a header byte 
    // OR not waiting for header (waiting for data bytes) and received a byte that is not the end of packet
    // OR not waiting for header and received a byte that is end of packet
    if ( (waitForHeaderFlag == 1) && (incoming == HEADER_CHAR) ) {
      
      waitForHeaderFlag = 0;
      incoming = 'N';
      inIdx = 0; // just in case a new packet starts without the prvious one terminating

    } else if ( (waitForHeaderFlag == 0) && (incoming != TERMINATION_CHAR) ) {
      
      if (inIdx < IN_PACKET_SIZE_BYTES) {
        arrayIncomingPacket[inIdx] = incoming;
        inIdx++;
      }
    } else if ( (waitForHeaderFlag == 0) && (incoming == TERMINATION_CHAR) ) {
      
      // TODO: 
      // packet complete! load, clear, etc
      dataSize = inIdx;
      inIdx = 0;
      waitForHeaderFlag = 1;
      rawDataAvailableFlag = 1; // let the parsing function know it can parse data
      //Serial.println("packet");
      
      
    } // END if (waitForHeaderFlag == 1)

    
  } // END if (Serial.available())

  
}


void clearPktArray(char thisPkt[], int len) {

    int i = 0;
    for (i=0; i<len; i++) {
        thisPkt[i] = '?';
    }
}


void printPktArrayEncoded(char thisPkt[]) {

    int i = 0;
    // Serial.print("encoded packet: %s", thisPkt); // has issues because no null termination string
    
    for (i=0; i<OUT_PACKET_SIZE_BYTES; i++) {
        Serial.print(thisPkt[i]);
    }
}


void printPktArrayDecoded(int slaveAddress, int command0, int command1, int command2, int command3) {

    printf("decoded packet:\n");
    printf("slave address: %d ", slaveAddress);
    printf("commands(0,1,2,3): %d,%d,%d,%d\n", command0, command1, command2, command3);

}


void encodeMasterToSlave(int slaveAddress, int command0, int command1, int command2, int command3, char outPkt[]) {

    int msgSource = 1; // source is master
    int rawSum = msgSource + slaveAddress + command0 + command1 + command2 + command3;

    outPkt[0] = HEADER_CHAR; // header
    outPkt[1] = foviIntToChar(msgSource); // source
    outPkt[2] = foviIntToChar(slaveAddress); // slave node address
    outPkt[3] = foviIntToChar(command0); // motor 0 command
    outPkt[4] = foviIntToChar(command1); // motor 1 command
    outPkt[5] = foviIntToChar(command2); // motor 2 command
    outPkt[6] = foviIntToChar(command3); // motor 3 command
    outPkt[7] = foviIntToChar(rawSum/16); // checksum high
    outPkt[8] = foviIntToChar(rawSum%16); // checksum low
    outPkt[9] = TERMINATION_CHAR; // terminator

}


void encodeSlaveToMaster(int slaveAddress, int command0, int command1, int command2, int command3, char outPkt[]) {

    int msgSource = 0; // source is slave
    int rawSum = msgSource + slaveAddress + command0 + command1 + command2 + command3;

    outPkt[0] = HEADER_CHAR; // header
    outPkt[1] = foviIntToChar(msgSource); // source
    outPkt[2] = foviIntToChar(slaveAddress); // slave node address
    outPkt[3] = foviIntToChar(command0); // motor 0 command
    outPkt[4] = foviIntToChar(command1); // motor 1 command
    outPkt[5] = foviIntToChar(command2); // motor 2 command
    outPkt[6] = foviIntToChar(command3); // motor 3 command
    outPkt[7] = foviIntToChar(rawSum/16); // checksum high
    outPkt[8] = foviIntToChar(rawSum%16); // checksum low
    outPkt[9] = TERMINATION_CHAR; // terminator

}


/// @brief Checks if packet from master is valid
///
/// Checks checksums
/// Checks if message source is master (should be 1)
/// Checks if slave address is valid (<=1)
/// Checks if data bytes are within range (<=15)
/// Does not check < 0 since foviCharToInt() function already does that
///
/// return 1 if successful, 0 if fail
///
int decode8CharMasterToSlave(int *slaveAddress, int *command0, int *command1, int *command2, int *command3, char inPkt[]) {

    int i = 0;
    int myData[8];
    int tempSum = 0;
    *slaveAddress = 63;
    *command0 = 63;
    *command1 = 63;
    *command2 = 63;
    *command3 = 63;

    // no need check if header and termination are correct
    // those do not get copied into the data buffer

    // 8 payload numbers
    for (i=0; i<8; i++) {
        myData[i] = foviCharToInt(inPkt[i]); // convert payload from ASCII char to int
    }

    // check checksums, based on the sum of data[0] through data[5]
    for (i=0; i<6; i++) {
        tempSum += myData[i];
    }
    if ( ((tempSum/16) != myData[6]) || ((tempSum%16) != myData[7]) ) {
        return 0;
    }

    // check if source is master and slave address is within range
    // no need to check < 0 since foviCharToInt() function already does that
    if( (myData[0] != 1) || (myData[1] > 1) ) {
        return 0;
    }

    // check if motor commands are within range
    // no need to check < 0 since foviCharToInt() function already does that
    if ( (myData[2] > 15) || (myData[3] > 15) || (myData[4] > 15) || (myData[5] > 15) ) {
        return 0;
    }

    // all good... pass back the slave address and commands
    *slaveAddress = myData[1];
    *command0 = myData[2];
    *command1 = myData[3];
    *command2 = myData[4];
    *command3 = myData[5];

    return 1;
}


/// @brief Checks if packet from slave is valid but does not check if it matches the local slave address
///
/// Checks checksums
/// Checks if message source is master (should be 1)
/// Checks if slave address is valid (<=1)
/// Checks if data bytes are within range (<=15)
/// Does not check < 0 since foviCharToInt() function already does that
///
/// return 1 if successful, 0 if fail
///
int decode8CharSlaveToMaster(int *slaveAddress, int *command0, int *command1, int *command2, int *command3, char inPkt[]) {

    int i = 0;
    int myData[8];
    int tempSum = 0;
    *slaveAddress = 63;
    *command0 = 63;
    *command1 = 63;
    *command2 = 63;
    *command3 = 63;

    // no need check if header and termination are correct
    // those do not get copied into the data buffer
    
    // 8 payload numbers
    for (i=0; i<8; i++) {
        myData[i] = foviCharToInt(inPkt[i]); // convert payload from ASCII char to int
    }

    // check checksums, based on the sum of data[0] through data[5]
    for (i=0; i<6; i++) {
        tempSum += myData[i];
    }
    if ( ((tempSum/16) != myData[6]) || ((tempSum%16) != myData[7]) ) {
        return 0;
    }

    // check if source is slave and slave address is within range
    // no need to check < 0 since foviCharToInt() function already does that
    if( (myData[0] != 0) || (myData[1] > 1) ) {
        return 0;
    }

    // check if motor commands are within range
    // no need to check < 0 since foviCharToInt() function already does that
    if ( (myData[2] > 15) || (myData[3] > 15) || (myData[4] > 15) || (myData[5] > 15) ) {
        return 0;
    }

    // all good... pass back the slave address and commands
    *slaveAddress = myData[1];
    *command0 = myData[2];
    *command1 = myData[3];
    *command2 = myData[4];
    *command3 = myData[5];

    return 1;
}


int foviCharToInt(char c) {

  if (c == '0') {
    return 0;
  } else if (c == '1') {
    return 1;
  } else if (c == '2') {
    return 2;
  } else if (c == '3') {
    return 3;
  } else if (c == '4') {
    return 4;
  } else if (c == '5') {
    return 5;
  } else if (c == '6') {
    return 6;
  } else if (c == '7') {
    return 7;
  } else if (c == '8') {
    return 8;
  } else if (c == '9') {
    return 9;
  } else if (c == 'A') {
    return 10;
  } else if (c == 'B') {
    return 11;
  } else if (c == 'C') {
    return 12;
  } else if (c == 'D') {
    return 13;
  } else if (c == 'E') {
    return 14;
  } else if (c == 'F') {
    return 15;
  } else  {
    // error
    return 63; // ASCII character '?'
  }
}


char foviIntToChar (int x) {

    int myChars[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

    if ((x >= 0) && (x <= 15)) {
        return myChars[x];
    } else {
        return '?'; // error
    }

}

/*
/// 1 forward
/// 2 reverse
/// 3 right
/// 4 left
/// 5 up
/// 6 down
/// 7 open
/// 8 close
/// 9 light
/// 10 tone
/// 11 dance
/// 12 sensors
/// one set with raw values?

///
int parseGeneralCommand() {

  // do not proceed if raw data available flag is not set
  if (rawDataAvailableFlag == 0) {
    return 0;
  }
  // skip the first character as it is probably a ','
  if( (arrayIncomingPacket[1] == 'f') && (arrayIncomingPacket[2] == 'o') && (arrayIncomingPacket[3] == 'r') && (arrayIncomingPacket[4] == 'w') && (arrayIncomingPacket[5] == 'a') && (arrayIncomingPacket[6] == 'r') && (arrayIncomingPacket[7] == 'd') ){
    Serial.print("forward,");
    Serial.println(threeDigitcharsToInt16Bit(9));
  } else if( (arrayIncomingPacket[1] == 'r') && (arrayIncomingPacket[2] == 'e') && (arrayIncomingPacket[3] == 'v') && (arrayIncomingPacket[4] == 'e') && (arrayIncomingPacket[5] == 'r') && (arrayIncomingPacket[6] == 's') && (arrayIncomingPacket[7] == 'e') ){
    Serial.print("reverse,");
    Serial.println(threeDigitcharsToInt16Bit(9));
  } else if( (arrayIncomingPacket[1] == 'r') && (arrayIncomingPacket[2] == 'i') && (arrayIncomingPacket[3] == 'g') && (arrayIncomingPacket[4] == 'h') && (arrayIncomingPacket[5] == 't') ) {
    Serial.print("right,");
    Serial.println(threeDigitcharsToInt16Bit(9));
  } else if( (arrayIncomingPacket[1] == 'l') && (arrayIncomingPacket[2] == 'e') && (arrayIncomingPacket[3] == 'f') && (arrayIncomingPacket[4] == 't') ) {
    Serial.print("left,");
    Serial.println(threeDigitcharsToInt16Bit(9));
  }  else if( (arrayIncomingPacket[1] == 'u') && (arrayIncomingPacket[2] == 'p') ) {
    Serial.print("up,");
    Serial.println(threeDigitcharsToInt16Bit(9));
  }  else if( (arrayIncomingPacket[1] == 'd') && (arrayIncomingPacket[2] == 'o') && (arrayIncomingPacket[3] == 'w') && (arrayIncomingPacket[4] == 'n') ) {
    Serial.print("down,");
    Serial.println(threeDigitcharsToInt16Bit(9));
  } else {
    Serial.print("Error. Unknown command.\n");
  }
  

  dataSize = 0;
  clearArrayIncomingPacket();
  rawDataAvailableFlag = 0; // clear after parsing/using raw data

  return 1;
  
}

uint16_t threeDigitcharsToInt16Bit (int idx) {

    // ASCII characters 0-9 have byte value 48-57
  
  if ( (arrayIncomingPacket[idx] >= 48) && (arrayIncomingPacket[idx]<= 57) && (arrayIncomingPacket[idx+1] >= 48) && (arrayIncomingPacket[idx+1]<= 57) && (arrayIncomingPacket[idx+2] >= 48) && (arrayIncomingPacket[idx+2]<= 57) ) {

    // 3 digit number
    return ((uint16_t)(charToInt8Bit(arrayIncomingPacket[idx]))*100 + (uint16_t)(charToInt8Bit(arrayIncomingPacket[idx+1]))*10 + (uint16_t)(charToInt8Bit(arrayIncomingPacket[idx+2])) );
  
  } else if ( (arrayIncomingPacket[idx] >= 48) && (arrayIncomingPacket[idx]<= 57) && (arrayIncomingPacket[idx+1] >= 48) && (arrayIncomingPacket[idx+1]<= 57) ) {

    // 2 digit number
    return ( (uint16_t)(charToInt8Bit(arrayIncomingPacket[idx]))*10 + (uint16_t)(charToInt8Bit(arrayIncomingPacket[idx+1])) );
  
  } else if ( (arrayIncomingPacket[idx] >= 48) && (arrayIncomingPacket[idx]<= 57) ) {

    // 1 digit number
    return ( (uint16_t)(charToInt8Bit(arrayIncomingPacket[idx])) );
    
  } else {
    Serial.print("Error: Wrong number format.\n");
    return 1023;
  }
  
}

*/
