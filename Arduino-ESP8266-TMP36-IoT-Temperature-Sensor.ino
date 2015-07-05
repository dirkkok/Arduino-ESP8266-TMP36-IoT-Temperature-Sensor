// http://playground.arduino.cc/Main/Average
#include <Average.h>
#include <SoftwareSerial.h>

char serialbuffer[100];//serial buffer for request url
SoftwareSerial mySerial(10, 11);

void setup() {
  Serial.begin(9600); // Connection to PC
  mySerial.begin(9600); // Connection to ESP8266
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
}

void loop() {
  float temp = getTemperatureAverage();
  Serial.println("Temperature: " + String(temp));
  sendTemperature(temp);

  delay(10000);
}

void sendTemperature(float temperature) {
  digitalWrite(7, HIGH);
  delay(2000);

  mySerial.println("AT+RST");
  WaitForReady(2000);
  mySerial.println("AT+CWMODE=1");
  WaitForOK(2000);
  mySerial.println("AT+RST");
  WaitForReady(2000);

  mySerial.println("AT+CWJAP=\"<wifi_ssid>\",\"<wifi_password>\"");
  if (WaitForOK(5000)) {
    digitalWrite(13, HIGH); // Connection succesful
  }

  mySerial.println("AT+CIPSTART=\"TCP\",\"data.sparkfun.com\",80");
  WaitForOK(5000);
  mySerial.println("AT+CIPSEND=123");
  WaitForOK(5000);
  mySerial.print("GET /input/<public_key>?private_key=<private_key>&temp=" + String(temperature) + " HTTP/1.0\r\n");
  mySerial.print("Hostname: data.sparkfun.com\r\n\r\n");
  WaitForOK(5000);
  mySerial.println("AT+CIPCLOSE");
  WaitForOK(5000);

  digitalWrite(13, LOW);
  digitalWrite(7, LOW);
}

float getTemperatureAverage() {
  Average<float> ave(10);
  for (int i = 0; i < 10; i++) {
    ave.push(getTemperature());
    delay(500);
  }

  float total = 0.0;
  delay(50);

  float temperature = ave.mean();

  return temperature;
}

float getTemperature() {
  int sensorVal = analogRead(A0);
  float voltage = (sensorVal / 1024.0) * 5.0;
  float temperature = (voltage - .5) * 100;

  return temperature;
}

boolean WaitForOK(long timeoutamount) {
  return WaitForResponse("OK", timeoutamount);
}

boolean WaitForReady(long timeoutamount) {
  return WaitForResponse("ready", timeoutamount);
}

// Parts used from https://github.com/contractorwolf/ESP8266
boolean WaitForResponse(String response, long timeoutamount) {
  unsigned long timeout = millis() + timeoutamount;

  while (millis() <= timeout) {
    while (mySerial.available() > 0) {
      int len = mySerial.readBytesUntil('\n', serialbuffer, sizeof(serialbuffer));

      String message = String(serialbuffer).substring(0, len - 1);

      if (message == response) {
        return true;
      }
    }
  }

  return false;
}
