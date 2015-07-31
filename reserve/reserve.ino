#include <pt.h>

#define PT_DELAY(pt, ms, ts) \
    ts = millis(); \
    PT_WAIT_WHILE(pt, millis()-ts < (ms));

#define PIN_LED 12

#define FREE 0
#define RESERVE 1

String val = "F";
String car_ID = "";

int ledStatus = FREE;

struct pt pt_taskLED;
struct pt pt_taskSendSerial;

///////////////////////////////////////////////////////
void setValue(){
  car_ID = val.substring(1,7);
  switch(val.charAt(0)) {
    case 'F':
	  ledStatus = RESERVE;
    break;
  }
}

///////////////////////////////////////////////////////
void serialEvent() {
  if (Serial1.available() > 0) {
    val = Serial1.readStringUntil('\r');
    Serial.print("value Recieve : ");
    Serial.println(val);
    Serial1.flush();
    setValue();
  }
}

///////////////////////////////////////////////////////
void sendSerial(){
  String sendData = String(val.charAt(0));
  sendData += car_ID;
  Serial1.print(sendData);
  Serial1.print('\r');
  Serial.print(sendData);
  Serial.print('\r');
}

///////////////////////////////////////////////////////
PT_THREAD(taskSendSerial(struct pt* pt))
{
  static uint32_t ts;

  PT_BEGIN(pt);

  while (1)
  {
    sendSerial();
    PT_DELAY(pt, 600, ts);
  }

  PT_END(pt);
}


///////////////////////////////////////////////////////
PT_THREAD(taskLED(struct pt* pt))
{
  static uint32_t ts;

  PT_BEGIN(pt);

  while (1)
  {
    digitalWrite(PIN_LED, ledStatus);
    PT_DELAY(pt, 150, ts);
  }

  PT_END(pt);
}

///////////////////////////////////////////////////////
void setup()
{
  Serial1.begin(115200);
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  PT_INIT(&pt_taskLED);
}

///////////////////////////////////////////////////////
void loop()
{
  taskLED(&pt_taskLED);

  serialEvent();
  taskSendSerial(&pt_taskSendSerial);
}
