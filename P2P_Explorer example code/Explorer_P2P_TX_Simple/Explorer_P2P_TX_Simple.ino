// RN2483_P2P code

// We put transmitter power to 1 and the spreading factor to 12 because in a test we saw at https://support.sodaq.com/Boards/ExpLoRer/Examples/LORAP2PGateway.pdf,
// the signal starts faded after the third floor (starting from the ground floor) and we think that a distance of 3 floors is okay.

int trPower = 1;         // Transreceiver power  ( can be -3 to 15)
String SprFactor = "sf12";  // Spreadingsfactor     (can be sf7 to sf12)
uint8_t max_dataSize = 100; // Maximum charcount to avoid writing outside of string
unsigned long readDelay = 60000; // Time to read for messages in ms (max 4294967295 ms, 0 to disable) --> A MODIFIER PROBABLEMENT PAR RAPPORT AU PARAMETRE DONNE A LA FONCTION "rx"

const char CR = '\r';
const char LF = '\n';

//'auth' proper to each device - set 'auth' to 242 for second device
byte auth = 241;
int devNonce = 0;
byte payload_temp[2];

// Configuring the RN2483 for P2P
void LoraP2P_Setup()
{

  Serial2.print("sys reset\r\n");
  delay(200);
  Serial2.print("radio set mod lora\r\n"); // to set our communication to lora mode (in opposition to FSK mode)
  delay(100);
  Serial2.print("radio set freq 865000000\r\n"); // we set the freq in order to be sure of the used one and doing so, we put it at a different frequence than the default one (868000000) so that we can filter a bit
  delay(100);
  Serial2.print("radio set pwr ");
  Serial2.print(trPower);
  Serial2.print("\r\n");
  delay(100);
  Serial2.print("radio set sf ");
  Serial2.print(SprFactor);
  Serial2.print("\r\n");
  delay(100);
  Serial2.print("radio set wdt ");
  Serial2.print(readDelay);
  Serial2.print("\r\n");
  delay(100);
  Serial2.print("mac pause\r\n"); // we put the mac functions in pause because we do not use the LoRaWAN protocol
  delay(100);

  FlushSerialBufferIn();
}


// Send Data array (in HEX)
void LORA_Write(char* Data)
{
  Serial2.print("radio tx ");
  Serial2.print(Data);
  Serial2.print("\r\n");
  Serial2.flush(); // --> a checker si nécessaire

  waitTillMessageGone();

}

// Send Data array (in HEX)
void LORA_Write_byte(byte* Data)
{  
  Serial2.print("radio tx ");
  Serial2.print("baba");
  //Serial2.print(Data[1]);
  //Serial2.print(Data[2]);
  //Serial2.print(Data[3]);
  Serial2.print("\r\n");
  //Serial2.flush(); // --> a checker si nécessaire

  waitTillMessageGone();

}

// Waits until the data transmit is done
void waitTillMessageGone() // --> SUREMENT A MODIFIER CAR QUELQUES ELEMENTS INUTILES
{
  while (!Serial2.available());
  delay(10);
  while (Serial2.available() > 0)
    Serial2.read();

  while (!Serial2.available());
  delay(10);
  while (Serial2.available() > 0)
  {
#ifdef DEBUG
    SerialUSB.write(Serial2.read());
#else
    Serial2.read();
#endif
  }
}

// Flushes any message available
void FlushSerialBufferIn()
{
  while (Serial2.available() > 0)
  {
#ifdef DEBUG
    SerialUSB.write(Serial2.read());
#else
    Serial2.read();
#endif
  }
}


// end code with all the necessary functions


void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(57600);
  Serial2.begin(57600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  while (!SerialUSB && millis() < 1000);

  LoraP2P_Setup();
}


// Authentification (includes D1 or D2) - DevNonce - (Heure/date)
void header(){
  devNonce++;
  
  //Reset devNonce for managing bits 
  if(devNonce == 255){
    devNonce = 0; 
  }
  //convert in to bits
  byte byte_devNonce = byte(devNonce); 

  //We can add date and time - to CRUSH JAMMING
  /*
  //If we can fetch date and time
  if(DateTime.available()) {
    unsigned long hd = DateTime.now();
  }
  */
}

//Payload
//Transform the temperature read into bytes
void payload(){
  uint16_t temp = getTemperature();
  
  payload_temp[0] = highByte(temp);
  payload_temp[1] = lowByte(temp);
}

//Get the temperature from the Sodaq sensor
uint16_t getTemperature(){
  //get the voltage in mV
  float mVolts = (float) analogRead(TEMP_SENSOR) * 3300.0 / 1023.0;
  // --> to get the temperature in decimal, the formula is "(mVolts - 500) / 10". Therefore, by doing "(mVolts - 500) * 10", it's like multiplying "(mVolts - 500) / 10" by 100.  
  int temp = (mVolts - 500) * 10;
  //We get a temp like such : 24.58C = 2458
  return int(temp);
}


void loop() {

  // Some data to send
  //char Data[100] = "48656c6c6f20576f726c6421";
  
  byte message[4];
  message[0] = auth;
  message[1] = byte(devNonce);
  message[2] = payload_temp[0];
  message[3] = payload_temp[1];

  digitalWrite(LED_RED, HIGH);
  delay(5000);
  LORA_Write_byte(message);
  digitalWrite(LED_RED, LOW); // To let us know when the data is send
  delay(50);

}
