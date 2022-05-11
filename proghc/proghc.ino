Uart SerialLog(1, 13, 12);

void setup() {
  Serial.begin(9600);
  SerialLog.begin(9600);
}

void loop() {
  if(SerialLog.available())Serial.write(SerialLog.read());
  if(Serial.available())SerialLog.write(Serial.read());
}
