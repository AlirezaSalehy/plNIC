#define RECEIVER

#include <SoftwareSerial.h>

const byte RX_BYTE = 3;
const byte TX_BYTE = 4;
SoftwareSerial ss(RX_BYTE, TX_BYTE);

const char data[] = "Hello World!";

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  #ifdef TRANSMITTER
   Serial.println("Transmitter started");
  #elif defined(RECEIVER)
    Serial.println("Receiver Started");
  #endif

  ss.begin(9600);

}

void loop() {
  #ifdef TRANSMITTER
    ss.println(data);
    Serial.println("Data is sent");
    delay(1000);
  #elif defined(RECEIVER)
    ss.listen();
    if (ss.available() > 0) {
      char data = ss.read();
      Serial.print(data);
    }
  #endif
}
