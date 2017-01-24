/*
  Arduino NSL Simplex Simulator (Serial Monitor)

Receives from the serial port, prints to the serial monitor.
This program is setup to receive 39 byte packets via UART. Each packet should begin with a preamble of 0x050505. So if the first three bytes received are all 5, then the packet should be Serial printed and an ACK sent back. If the first three bytes are not all 5, then the packet should not be sent, and a NAK should be sent back.

 created 1/14/17
 by Adam Will

*/

int byte_count=0;   //counter to count the size of the incoming packet (packet should be 39 bytes; 3 preamble bytes + 36 payload)
int simplex_BUSY=2; //BUSY line for the simplex is pin 2 (HIGH = busy, LOW = ready to receive)
int PIC_RTS=3;      //PIC controlled line indicating the PIC is ready to receive a transmission (ACK or NAK)
int CTX=1;          //Arduino clear to transmit (written high when reading PIC_CRX = 1); cleared to TX when CTX=1

void setup() {
  // initialize serial port:
  Serial.begin(9600);
  pinMode(simplex_BUSY, OUTPUT);  //sets BUSY line of simplex as an output
  pinMode(PIC_RTS, INPUT);        //sets the line reading the PIC’s clear to receive line as an input
  pinMode(13, OUTPUT); 
}

void loop() {
int preamble[3]={0};    //array to hold preamble
int packet[36]={0};   //array to hold values of payload bytes
digitalWrite(simplex_BUSY,HIGH); //simplex NOT ready to receive 
digitalWrite(13,HIGH);
/* Part 1 - Arduino waits to receive 39-byte packet */

while (PIC_RTS==1);{
delay(100);
digitalWrite(simplex_BUSY,LOW); //simplex ready to receive 
digitalWrite(LED,LOW);

int i=0;
while (i<3){
if (Serial.available()) {
  preamble[i]=Serial.read(); 
  if (preamble[i]==5){
    i++;
  }
  else i=4;
  }
}

if (i==3){
/* Part 2 - Arduino receives 36-byte payload */
   //sets counter of bytes in the packet to 0
byte_count=0;
while (byte_count<36) {     //For loop stops reading UART after receiving 39 bytes
  // read from port
  if (Serial.available()) {
    packet[byte_count]=Serial.read(); 
    byte_count++;
  }
}

/* Part 3 - Arduino received 39-byte packet and is now processing it */
digitalWrite(simplex_BUSY,HIGH);  //simplex not ready to receive; stopped while checking that packet preamble is ok and during serial printing
digitalWrite(LED,HIGH);

int count9=0; //count9 allows for payload to be broken into 4 9-byte lines
byte_count=0;
Serial.println("Packet:");
while (byte_count<36){ //For loop serial prints packet as 4 9-byte lines
  if (count9<8) {
    
    Serial.print(packet[byte_count]); //prints byte with a comma after 
    Serial.print(",");  
    count9=count9+1;
    byte_count++;
  } 
  else {
    Serial.print(packet[byte_count]); //prints byte and comma, followed by a new line
    Serial.println(",");
    count9=0;
    byte_count++;
  
  }
}
}
}
}

