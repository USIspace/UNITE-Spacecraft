/*
  Reads from serial and then prints
*/


int busy = 8;
void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  pinMode(busy, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() {

  digitalWrite(busy, LOW);
  digitalWrite(13,LOW);
  //while(!Serial.available()) {}

  if (Serial.available()) {

    digitalWrite(busy, HIGH);
    digitalWrite(13, HIGH);
    
    int inByte = Serial.read();
    Serial.println(inByte);

    for (int i = 0; i < 35; i++) {
      if (Serial.available()) {
        int inByte = Serial.read();
        Serial.println(inByte);
      }
    }

  }
}
