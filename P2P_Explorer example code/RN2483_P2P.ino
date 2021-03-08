
int8_t trPower = 1;         // Transreceiver power  ( can be -3 to 15)
String SprFactor = "sf12";  // Spreadingsfactor     (can be sf7 to sf12)
uint8_t max_dataSize = 100; // Maximum charcount to avoid writing outside of string
unsigned long readDelay = 60000; // Time to read for messages in ms (max 4294967295 ms, 0 to disable)

const char CR = '\r';
const char LF = '\n';


// Configuring the RN2483 for P2P
void LoraP2P_Setup()
{

  Serial2.print("sys reset\r\n");
  delay(200);
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
  Serial2.print("mac pause\r\n");
  delay(100);

  FlushSerialBufferIn();
}


// Send Data array (in HEX)
void LORA_Write(char* Data)
{
  Serial2.print("radio tx ");
  Serial2.print(Data);
  Serial2.print("\r\n");
  Serial2.flush();

  waitTillMessageGone();

}

// Waits until the data transmit is done
void waitTillMessageGone()
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


// Setting up the receiver to read for incomming messages
void StartLoraRead()
{
  Serial2.print("radio rx 0\r\n");
  delay(100);

  FlushSerialBufferIn();
}

//////////////////////////////////////
// Read message from P2P TX module  //
// Returns 1 if there is a message  //
// Returns 2 if there is no message //
//////////////////////////////////////
int LORA_Read(char* Data)
{
  int messageFlag = 0;
  String dataStr = "radio_rx  ";
  String errorStr = "radio_err";
  String Buffer = "";

  StartLoraRead();

  while (messageFlag == 0) // As long as there is no message
  {
    while (!Serial2.available());
    
    delay(50);  // Some time for the buffer to fill

    // Read message from RN2483 LORA chip
    while (Serial2.available() > 0 && Serial2.peek() != LF)
    {
      Buffer += (char)Serial2.read();
    }

    // If there is an incoming message
    if (Buffer.startsWith(dataStr, 0)) // if there is a message in the buffer
    {
      int i = 10;  // Incoming data starts at the 11th character

      // Seperate message from string till end of datastring
      while (Buffer[i] != CR && i - 10 < max_dataSize)
      {
        Data[i - 10] = Buffer[i];
        i++;
      }
      messageFlag = 1; // Message received
    }
    else if (Buffer.startsWith(errorStr, 0))
    {
      messageFlag = 2; // Read error or Watchdogtimer timeout
    }
  }

#ifdef DEBUG
  SerialUSB.println(Buffer);
#endif

  return (messageFlag);
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

