#include <SPI.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

// RF24 radio(CE,CSN);
RF24 radio(49,53);

const uint64_t pipes[2] = { 0xDEDEDEDEE8LL, 0xDEDEDEDEE4LL };

boolean stringComplete = false;
static int dataBufferIndex = 0;
boolean stringOverflow = false;
char charOverflow = 0;

String SendPayload = "";
String RecvPayload = "";
char serialBuffer[50] = "";


void setup(void) {
  Serial1.begin(115200);
  Serial.begin(115200);
  printf_begin();
  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads

 // radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setDataRate(RF24_250KBPS);
  //radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(70);
  
  radio.enableDynamicPayloads();
  radio.setRetries(0,15);
  radio.setCRCLength(RF24_CRC_16);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);  
  
  radio.startListening();
  radio.printDetails();

  Serial.println();
  Serial.println("RF Chat V01.0");    
  
  delay(500);
  
  ///SendPayload.reserve(100);
 /// RecvPayload.reserve(100);
}

void loop(void) {
  nRF_receive();
  serial_receive();
} // end loop()

void serialEvent1() {
  while (Serial1.available()) {
    // get the new byte:
    char inChar = (char)Serial1.read();
    // add it to the inputString:
    SendPayload += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
} // end serialEvent()

void nRF_receive(void) {
  int len = 0;
  if ( radio.available() ) {
    len = radio.getDynamicPayloadSize();
    radio.read(&serialBuffer,len);
   // delay(5);
    serialBuffer[len] = 0; // null terminate string
    
    Serial.print("R:");
    Serial.print(serialBuffer);
    Serial.println();
    RecvPayload = String(serialBuffer);
    if (RecvPayload.startsWith("MOTOR"))
      {
        RecvPayload= RecvPayload.substring(6);
      Serial1.println(RecvPayload);
      }
    // clear the string:
    RecvPayload = "";
    serialBuffer[0] = 0;  // Clear the buffers
  }  

} // end nRF_receive()

void serial_receive(void){
  if (stringComplete) { 
        // swap TX & Rx addr for writing
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(0,pipes[0]);  
        radio.stopListening();
        //bool a = radio.write(SendPayload.c_str(),SendPayload.length());
        char SendPayload1[31] = "maaz";
         // Serial.println("sss");

        bool a = radio.write(&SendPayload,sizeof(SendPayload));
       // radio.write(&SendPayload,strlen(SendPayload));
        if(a)
        {
          Serial.print("S:");
          Serial.print(SendPayload1);          
        Serial.println();
        
        }
        stringComplete = false;
        SendPayload = "";

             
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
        radio.startListening();  
 
  }
}
