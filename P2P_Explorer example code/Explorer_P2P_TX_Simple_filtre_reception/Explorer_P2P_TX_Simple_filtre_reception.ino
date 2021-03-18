// RN2483_P2P code

// We put transmitter power to 1 and the spreading factor to 12 because in a test we saw at https://support.sodaq.com/Boards/ExpLoRer/Examples/LORAP2PGateway.pdf, 
// the signal starts faded after the third floor (starting from the ground floor) and we think that a distance of 3 floors is okay.

int trPower = 1;         // Transreceiver power  ( can be -3 to 15)
String SprFactor = "sf12";  // Spreadingsfactor     (can be sf7 to sf12)
uint8_t max_dataSize = 100; // Maximum charcount to avoid writing outside of string
unsigned long readDelay = 10000; // Time to read for messages in ms (max 4294967295 ms, 0 to disable) --> A MODIFIER PROBABLEMENT PAR RAPPORT AU PARAMETRE DONNE A LA FONCTION "rx". En fait ça c'est genre le temps pendant lequel tu attends quand tu fais "radio rx". Passé ce délai, ça renvoie une erreur de Timeout

const char CR = '\r';
const char LF = '\n';


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

  pinMode(LED_GREEN, OUTPUT);

  while (!SerialUSB && millis() < 1000);

  LoraP2P_Setup();
}

void loop() {
  
  // Some data to send
  char Data[100] = "48656c6c6f20576f726c6421";

  digitalWrite(LED_GREEN, HIGH);
  delay(5000);
  LORA_Write(Data);
  digitalWrite(LED_GREEN, LOW); // To let us know when the data is send
  delay(50);


}
