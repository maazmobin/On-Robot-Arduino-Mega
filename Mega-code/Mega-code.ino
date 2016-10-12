#include <quaternionFilters.h>
#include <MPU9250.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

// RF24 radio(CE,CSN);
RF24 radio(49,53);

const uint64_t pipes[2] = { 0xDEDEDEDEE8LL, 0xDEDEDEDEE4LL };

boolean stringComplete = false;  // whether the string is complete
static int dataBufferIndex = 0;
boolean stringOverflow = false;
char charOverflow = 0;

String SendPayload = "";
char RecvPayload[31] = "";
char serialBuffer[31] = "";
String IMU_data="";

                                    //BROADCAST IMU DATA After 50ms
unsigned long prevIMUtime = 0;
int IMUinterval = 100; //100ms

                                    //Declearing IMU

#include <MPU9250.h>   //https://github.com/sparkfun/SparkFun_MPU-9250_Breakout_Arduino_Library
#define IMU_SerialDebug false  // Set to true to get Serial output for debugging
MPU9250 myIMU;
int dataTurn=0;

void setup(void) {

                            //FOR IMU
    Wire.begin();
  // TWBR = 12;  // 400 kbit/sec I2C speed
    myIMU.initMPU9250();
    myIMU.initAK8963(myIMU.magCalibration);
                            //END IMU
 
  Serial.begin(115200);
  Serial1.begin(115200);
  printf_begin();
  radio.begin();
  
  radio.setDataRate(RF24_1MBPS);
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
}

void loop(void) {
  
  nRF_receive();
  serial_receive();
  broadcastIMU();
  
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
}
void nRF_receive(void) {
  int len = 0;
  if ( radio.available() ) {
        len = radio.getDynamicPayloadSize();
        radio.read(&RecvPayload,len);
        //delay(5);
  
    RecvPayload[len] = 0; // null terminate string
    
 //   Serial.print("R:");
  Serial.print(RecvPayload);
    Serial.println();
    String a=String(RecvPayload);
     if (a.startsWith("MOTOR"))
      {
       a= a.substring(6);
      Serial1.println(a);
      }
    RecvPayload[0] = 0;  // Clear the buffers
  }  

} // end nRF_receive()

void serial_receive(void){
  
  if (stringComplete) { 
       // strcat(SendPayload,serialBuffer);      
        // swap TX & Rx addr for writing
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(0,pipes[0]);  
        radio.stopListening();
        char c[31]="";
      SendPayload.toCharArray(c,SendPayload.length());
        delay(2); //Delay is necessary, else transmission stops
        radio.write(c,sizeof(c));  
        Serial.println(c);          
        stringComplete = false;
       // Serial.println();
        // restore TX & Rx addr for reading  
             
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
         
        radio.startListening();  
        SendPayload="";
        dataBufferIndex = 0;
  } // endif
}// end serial_receive()
void broadcastIMU(void)
{
  unsigned long currentIMUmillis = millis();
  if (currentIMUmillis - prevIMUtime >= IMUinterval) {
    prevIMUtime = currentIMUmillis;
    IMU_data="";
  if(dataTurn==0)
  {
    dataTurn++;
     myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();
    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0]*myIMU.aRes; // - accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes; // - accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2]*myIMU.aRes; // - accelBias[2];
	IMU_data = "acc," + String(myIMU.ax*1000) +  "," + String(myIMU.ay*1000) + "," + String(myIMU.az*1000);
	if (IMU_SerialDebug)
	{
  Serial.println(IMU_data);
	}
    }
  else if(dataTurn==1)
  {
    dataTurn++;
    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values
    myIMU.getGres();
    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0]*myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1]*myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2]*myIMU.gRes;
    IMU_data="gyro,"+ String(myIMU.gx)+","+String(myIMU.gy)+","+String(myIMU.gz);
	if (IMU_SerialDebug)
	{
  Serial.println(IMU_data);
	}
    }
  else if(dataTurn==2)
  {
    dataTurn++;
     myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values
    myIMU.getMres();
    // User environmental x-axis correction in milliGauss, should be
    // automatically calculated
    myIMU.magbias[0] = +470.;
    // User environmental x-axis correction in milliGauss TODO axis??
    myIMU.magbias[1] = +120.;
    // User environmental x-axis correction in milliGauss
    myIMU.magbias[2] = +125.;
    
    myIMU.mx = (float)myIMU.magCount[0]*myIMU.mRes*myIMU.magCalibration[0] -
               myIMU.magbias[0];
    myIMU.my = (float)myIMU.magCount[1]*myIMU.mRes*myIMU.magCalibration[1] -
               myIMU.magbias[1];
    myIMU.mz = (float)myIMU.magCount[2]*myIMU.mRes*myIMU.magCalibration[2] -
               myIMU.magbias[2];
    IMU_data="mag,"+ String(int(myIMU.mx))+","+String(int(myIMU.my))+","+String(int(myIMU.mz));
	if (IMU_SerialDebug)
	{
		Serial.println(IMU_data);
	}
    }
  else if(dataTurn==3)
  {
    dataTurn=0;
    myIMU.tempCount = myIMU.readTempData();  // Read the adc values
        myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
        IMU_data="temp,"+String(myIMU.temperature);
		if (IMU_SerialDebug)
		{
			Serial.println(IMU_data);
		}
    }
     radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(0,pipes[0]);  
        radio.stopListening();
        char c[31]="";
      IMU_data.toCharArray(c,IMU_data.length());
        delay(2); //Delay is necessary, else transmission stops
        radio.write(c,sizeof(c));  
        Serial.println(c);          
        stringComplete = false;  
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
        radio.startListening(); 
  }

  
}

