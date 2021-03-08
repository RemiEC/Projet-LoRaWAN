
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
  digitalWrite(LED_GREEN, HIGH);
  char Data[100] = "";  // Array to store the message in

  if (LORA_Read(Data) == 1)
  {
    digitalWrite(LED_GREEN, LOW); // Light up LED if there is a message
    SerialUSB.println(Data);
    delay(50);
  }
}









