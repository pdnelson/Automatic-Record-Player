#define SERIAL_SPEED 115200

#define STP1_IN1 2
#define STP1_IN2 3
#define STP1_IN3 4
#define STP1_IN4 5
#define LFT_BTN 6
#define RGT_BTN 7
#define MOV_LED_L 8
#define MOV_LED_R 9

#define STP2_IN1 13
#define STP2_IN2 12
#define STP2_IN3 11
#define STP2_IN4 10

bool left = false;
bool right = true;

int stepNum = 0;
bool moving = false;

int loopCounter = 0;
bool ledOn = false;

int moveDelay = 4;

void setup() {
  pinMode(STP1_IN1, OUTPUT);
  pinMode(STP1_IN2, OUTPUT);
  pinMode(STP1_IN3, OUTPUT);
  pinMode(STP1_IN4, OUTPUT);
  
  pinMode(STP2_IN1, OUTPUT);
  pinMode(STP2_IN2, OUTPUT);
  pinMode(STP2_IN3, OUTPUT);
  pinMode(STP2_IN4, OUTPUT);
  
  pinMode(LFT_BTN, INPUT);
  pinMode(RGT_BTN, INPUT);

  pinMode(MOV_LED_L, OUTPUT);
  pinMode(MOV_LED_R, OUTPUT);


  Serial.begin(SERIAL_SPEED);
  //Serial.println("Initialized arduino with move delay of %i ms.", moveDelay);
}

void loop() {
  int leftStatus = digitalRead(LFT_BTN);
  int rightStatus = digitalRead(RGT_BTN);

  if(leftStatus && !rightStatus) {
    if(!moving) stepNum = 0;
    digitalWrite(MOV_LED_L, HIGH);
    digitalWrite(MOV_LED_R, LOW);
    oneStep(right, STP1_IN1, STP1_IN2, STP1_IN3, STP1_IN4, false);
    oneStep(right, STP2_IN1, STP2_IN2, STP2_IN3, STP2_IN4, false);
    moving = true;
    delay(moveDelay);
  }
  else if (!leftStatus && rightStatus) {
    if(!moving) stepNum = 3;
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, HIGH);
    oneStep(left, STP1_IN1, STP1_IN2, STP1_IN3, STP1_IN4, false);
    oneStep(left, STP2_IN1, STP2_IN2, STP2_IN3, STP2_IN4, false);
    moving = true;
    delay(moveDelay);
  }
  else {
    moving = false;
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, LOW);
    oneStep(left, STP1_IN1, STP1_IN2, STP1_IN3, STP1_IN4, true);
    oneStep(left, STP2_IN1, STP2_IN2, STP2_IN3, STP2_IN4, true);
  }

  /*loopCounter++;

  if(loopCounter > 20000) {
    if(ledOn) {
      digitalWrite(LED_BUILTIN, LOW);
      ledOn = false;
    }
    else {
      digitalWrite(LED_BUILTIN, HIGH);
      ledOn = true;
    }
    loopCounter = 0;
  }*/
}

void oneStep(bool dir, int in1, int in2, int in3, int in4, bool stopping) {
  if(!stopping) {
    switch(stepNum) {
      case 0:
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
        break;
      case 1:
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
        break;
      case 2:
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        break;
      case 3:
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        break;
      default:
        Serial.println("Step number exceeded limit: " + stepNum);
    }

    if(dir == left) stepNum++;
    else stepNum--;
    if(stepNum > 3) stepNum = 0;
    else if(stepNum < 0) stepNum = 3;
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }
}