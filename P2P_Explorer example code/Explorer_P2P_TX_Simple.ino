

//#define DEBUG

void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(57600);
  Serial2.begin(57600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT) ;

  while (!SerialUSB && millis() < 1000);

  LoraP2P_Setup();
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // Some data to send
  char Data[100] = "48656c6c6f20576f726c6421";


  LORA_Write(Data);
  digitalWrite(LED_GREEN, LOW); // To let us know when the data is send
  delay(100);
  digitalWrite(LED_GREEN, HIGH);


}
