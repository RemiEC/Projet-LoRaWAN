// RN2483_P2P code
#include <AES.h>
#include "./printf.h"
#include "stdlib.h"

AES aes ;

byte *key = (unsigned char*)"0123456789010123";

byte plain[30];
int plainLength = sizeof(plain)-1;  // don't count the trailing /0 of the string !
int padedLength = plainLength + N_BLOCK - plainLength % N_BLOCK;
byte cipher[20];

//real iv = iv x2 ex: 01234567 = 0123456701234567
unsigned long long int my_iv = 36753562;

// We put transmitter power to 1 and the spreading factor to 12 because in a test we saw at https://support.sodaq.com/Boards/ExpLoRer/Examples/LORAP2PGateway.pdf,
// the signal starts faded after the third floor (starting from the ground floor) and we think that a distance of 3 floors is okay.

int trPower = 1;         // Transreceiver power  ( can be -3 to 15)
String SprFactor = "sf12";  // Spreadingsfactor     (can be sf7 to sf12)
uint8_t max_dataSize = 100; // Maximum charcount to avoid writing outside of string
unsigned long readDelay = 60000; // Time to read for messages in ms (max 4294967295 ms, 0 to disable) --> A MODIFIER PROBABLEMENT PAR RAPPORT AU PARAMETRE DONNE A LA FONCTION "rx"

const char CR = '\r';
const char LF = '\n';

int devNonce = 0;

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
void LORA_Write_String(String Data)
{  
  Serial2.print("radio tx ");
  Serial2.print(Data);
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


// Authentification (includes D1 or D2) - DevNonce - (Heure/date)
String header_function(){
  devNonce++;
  
  //Reset devNonce for managing bits 
  if(devNonce == 255){
    devNonce = 0; 
  }
  //String date = String(__DATE__).substring(0,6);
  //String time = String(__TIME__).substring(0,5);

  //'auth' proper to each device - set 'auth' to 242 for second device
  String auth = "241";

  //Return the complete header
  return auth + ' ' + String(devNonce);
}

//Payload
//Transform the temperature read into bytes
String payload_function(){
  String payload;
  uint16_t temp = getTemperature();
  payload = String(temp);
  return payload;
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

void prekey (int bits)
{
  String k = String(devNonce, HEX);
  long count = strtol(k.c_str(), NULL, 16);

  String temp_string = String(getTemperature());
  int temp1 = temp_string.substring(0,2).toInt();
  int temp2 = temp_string.substring(2,4).toInt();
  
  byte plain[] = { 0xF1, count, temp1, temp2 };

  SerialUSB.print("\n\nPLAIN :");
  for(int i = 0; i < sizeof(plain); i++){
    SerialUSB.print(String(plain[i])); 
  }
  SerialUSB.println();
  
  aes.iv_inc();
  byte iv [N_BLOCK];
  byte plain_p[padedLength];
  byte check[padedLength];
    
  aes.set_IV(my_iv);
  aes.get_IV(iv);
  aes.do_aes_encrypt(plain, plainLength, cipher, key, bits, iv);
  
  aes.set_IV(my_iv);
  aes.get_IV(iv);  
  aes.do_aes_decrypt(cipher,padedLength,check,key,bits,iv);
  
  SerialUSB.print("\nCIPHER:");
  for(int k = 0; k < sizeof(plain); k++){
    SerialUSB.print(cipher[k]); 
  }
  
  SerialUSB.println("\n============================================================\n");
}

void prekey_test ()
{
  prekey (128) ;
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

void loop() {
  
  // Data to send
  String Data = header_function() + ' ' + payload_function();

  prekey_test();

  // Convert Data into HEX
  String Data_HEX;
  for (int i = 0; i < Data.length(); i++)
  {
    Data_HEX += String(Data[i], HEX);  
  }

  String Data_HEX_CIPHER;
  for (int i = 0; i < sizeof(cipher); i++)
  {
    Data_HEX_CIPHER += String(cipher[i], HEX);
  }
  
  SerialUSB.println(Data_HEX);
  
  // Send the data
  digitalWrite(LED_RED, HIGH);
  delay(5000);
  LORA_Write_String(Data_HEX);
  LORA_Write_String(Data_HEX_CIPHER);
  digitalWrite(LED_RED, LOW); // To let us know when the data is send
  delay(50);
}
