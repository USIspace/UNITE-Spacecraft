/*
  Reads from serial and then prints
 */

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial.println(inByte);
  }
}
