/*
  Reads from serial and then prints
*/


void setup() {

  Serial.begin(9600);
}

void loop() {

  if (Serial.available()) {
    int nByte = Serial.read();
    Serial.println(nByte);
  }
}

