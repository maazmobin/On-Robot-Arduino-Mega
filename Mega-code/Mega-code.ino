
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String inputString1 = "";         // a string to hold incoming data
boolean stringComplete1 = false;  // whether the string is complete
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
Serial1.begin(115200);
//digitalWrite(13,LOW);
  inputString.reserve(200);
  inputString1.reserve(200);
}

void loop() {
  
    if (stringComplete) 
    {
      if (inputString.startsWith("MOTOR"))
      {inputString= inputString.substring(6);
      Serial1.println(inputString);
      }
    // clear the string:
    inputString = "";
    stringComplete = false;  
    }
      if (stringComplete1) 
    {
      Serial.println(inputString1);  
    // clear the string:
    inputString1 = "";
    stringComplete1 = false;  
    }
 }
  // put your main code here, to run repeatedly:


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;   
    }
  }
}
void serialEvent1(){
  while (Serial1.available()) {
    char inChar1 = (char)Serial1.read();
    inputString1 += inChar1;
    if (inChar1 == '\n') {
      stringComplete1 = true;   
    }
  }
}
