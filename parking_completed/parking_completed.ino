#include <pt.h>
#include <Servo.h>

#define PT_DELAY(pt, ms, ts) \
    ts = millis(); \
    PT_WAIT_WHILE(pt, millis()-ts < (ms));

// PIN_SENSOR
#define PIN_LED 12
#define PIN_SERVO 3
#define PIN_ULTRA_ECHO 11
#define PIN_ULTRA_TRIG 12

// SERVO STATUS
#define CLOSE 0
#define OPEN 1

// LED & LCD STATUS
#define FREE 0
#define RESERVE 1
#define PARK 2

// READ WRITE VALUE
long duration = 0;  // Ultrasonic
long distance = 0;  // Ultrasonic
String val = "F";
String car_ID = "";

Servo myservo;

int servoStatus = CLOSE;
int ledStatus = FREE;

struct pt pt_taskLED;
struct pt pt_taskSERVO;
struct pt pt_taskULTRA;
struct pt pt_taskSendSerial;

///////////////////////////////////////////////////////
void setValue(){
  car_ID = val.substring(1,7);
  switch(val.charAt(0)) {
    case 'R':
      ledStatus = RESERVE;
    break;
    case 'P':
      ledStatus = PARK;
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
PT_THREAD(taskULTRA(struct pt* pt))
{
  static uint32_t ts;

  PT_BEGIN(pt);

  while (1)
  {
    digitalWrite(PIN_ULTRA_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_ULTRA_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRA_TRIG, LOW);
    duration = pulseIn(PIN_ULTRA_ECHO, HIGH);
    distance = (duration/2) / 29.1;
    PT_DELAY(pt, 150, ts);
    Serial.print(distance);
  }

  PT_END(pt);
}

///////////////////////////////////////////////////////
PT_THREAD(taskSERVO(struct pt* pt))
{
  static uint32_t ts;

  PT_BEGIN(pt);

  while (1)
  {
    if(val.charAt(0) == 'R'){
      myservo.write(90);
    }
    else if(val.charAt(0) == 'P'){
      myservo.write(0);
      if(distance < 5){
          myservo.write(90);
      }
    }
    PT_DELAY(pt, 150, ts);
  }

  PT_END(pt);
}

///////////////////////////////////////////////////////
void setup()
{
  myservo.attach(PIN_SERVO);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_ULTRA_ECHO, INPUT);
  pinMode(PIN_ULTRA_TRIG, OUTPUT);

  PT_INIT(&pt_taskLED);
  PT_INIT(&pt_taskULTRA);
  PT_INIT(&pt_taskSERVO);

  Serial1.begin(115200);
  Serial.begin(115200);
}

///////////////////////////////////////////////////////
void loop()
{
  taskLED(&pt_taskLED);
  taskULTRA(&pt_taskULTRA);
  taskSERVO(&pt_taskSERVO);

  serialEvent();
  taskSendSerial(&pt_taskSendSerial);
}
