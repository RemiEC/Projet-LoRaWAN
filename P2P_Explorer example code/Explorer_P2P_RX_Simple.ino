#include <Thread.h>

// RN2483_P2P code

// We put transmitter power to 1 and the spreading factor to 12 because in a test we saw at https://support.sodaq.com/Boards/ExpLoRer/Examples/LORAP2PGateway.pdf, 
// the signal starts faded after the third floor (starting from the ground floor) and we think that a distance of 3 floors is okay.

int trPower = 1;         // Transreceiver power  ( can be -3 to 15)
String SprFactor = "sf12";  // Spreadingsfactor     (can be sf7 to sf12)
uint8_t max_dataSize = 100; // Maximum charcount to avoid writing outside of string
unsigned long readDelay = 60000; // Time to read for messages in ms (max 4294967295 ms, 0 to disable) --> A MODIFIER PROBABLEMENT PAR RAPPORT AU PARAMETRE DONNE A LA FONCTION "rx"


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

  // Setting up the receiver to read for incomming messages
  // Ici on est en continuous reception (parce qu'il y a le 0) mais on devrait mettre une limite 
  // Cette limite sera une limite de taille donc il faudra déterminer la taille maximum d'un message pour sécuriser un peu 
  Serial2.print("radio rx 0\r\n");
  delay(100);
  FlushSerialBufferIn(); // --> A VOIR SI ON DOIT ENLEVER (FAIRE TEST SANS)

  while (messageFlag == 0) // As long as there is no message
  {
    while (!Serial2.available()); // while there is nothing to read
    
    delay(50);  // Some time for the buffer to fill

    // Read message from RN2483 LORA chip
    while (Serial2.available() > 0 && Serial2.peek() != '\n')
    {
      Buffer += (char)Serial2.read();
    }

    // If there is an incoming message
    if (Buffer.startsWith(dataStr)) // if there is a message in the buffer
    {
      int i = 10;  // Incoming data starts at the 11th character --> look at the length of dataStr 

      // Seperate message from string till end of datastring
      while (Buffer[i] != '\r' && i - 10 < max_dataSize)
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







void LORA_Read_Test()
{
  int messageFlag = 0;
  String dataStr = "radio_rx  ";
  String errorStr = "radio_err";
  String Buffer = "";
  String KeyAuth = "48656c";
  int counter_KeyAuth = 0; 
  char Data[100] = "";  // Array to store the message in

  // Setting up the receiver to read for incomming messages
  // Ici on est en continuous reception (parce qu'il y a le 0) mais on devrait mettre une limite 
  // Cette limite sera une limite de taille donc il faudra déterminer la taille maximum d'un message pour sécuriser un peu 
  Serial2.print("radio rx 0\r\n");
  delay(100);
  FlushSerialBufferIn(); // --> A VOIR SI ON DOIT ENLEVER (FAIRE TEST SANS)

  while (messageFlag == 0) // As long as there is no message
  {
    while (!Serial2.available()); // while there is nothing to read
    
    delay(50);  // Some time for the buffer to fill

    // Read message from RN2483 LORA chip
    while (Serial2.available() > 0 && Serial2.peek() != '\n')
    {
      Buffer += (char)Serial2.read();
    }

    // If there is an incoming message
    if (Buffer.startsWith(dataStr)) // if there is a message in the buffer
    {
      int i = 10;  // Incoming data starts at the 11th character --> look at the length of dataStr 

      // Seperate message from string till end of datastring
      while (Buffer[i] != '\r' && i - 10 < max_dataSize)
      {
        if (Buffer[i] != KeyAuth[counter_KeyAuth])
        {
          // Close thread
        }
        counter_KeyAuth++;
        
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
}



// end code with all the necessary functions



void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(57600);
  Serial2.begin(57600);

  while (!SerialUSB && millis() < 1000);

  LoraP2P_Setup();
}

void loop() {
  
  // On dirait que le HIGH et le LOW sont inversés
  
  //digitalWrite(LED_GREEN, HIGH);
  //char Data[100] = "";  // Array to store the message in


  // A MODIFIER POUR AVOIR DES STATEMENTS PERSONNALISES, JE PENSE QU'ACTUELLEMENT LA CONDITION DU IF EST NULLE
  //if (LORA_Read(Data) == 1)
  //{
   // digitalWrite(LED_GREEN, LOW); // Light up LED if there is a message
    //SerialUSB.println(Data);
    //delay(50);
  //}

  if (Serial2.available() > 0)
  {
    // LANCER THREAD POUR LOAD READ
    Thread myThread = Thread();
    myThread.onRun(LORA_Read_Test);
  }
}