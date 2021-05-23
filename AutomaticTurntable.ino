#include "MotorDirection.cpp"

#define SERIAL_SPEED 115200

// First stepper motor
#define STP1_IN1 2
#define STP1_IN2 3
#define STP1_IN3 4
#define STP1_IN4 5

// Second stepper motor
#define STP2_IN1 13
#define STP2_IN2 12
#define STP2_IN3 11
#define STP2_IN4 10

// Left/right buttons for clockwise/counterclockwise test movements. These exist for troubleshooting/testing purposes only.
#define LFT_BTN 6
#define RGT_BTN 7

// These LEDs light up when moving one direction or another. They exist for troubleshooting/testing purposes only.
#define MOV_LED_L 8
#define MOV_LED_R 9

// This is an internal step count for the motors, so we know how many steps it has taken
int32_t totalStepCount = 0;
int8_t stepPosition = 0;

// This is the move delay, in ms, between motor steps. The motors used in this project are 28BYJ-48 stepper motors,
// which offer a minimum delay of 2ms between steps. Additional ms were added for precision, at the cost of speed.
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
}

void loop() {
  int leftStatus = digitalRead(LFT_BTN);
  int rightStatus = digitalRead(RGT_BTN);

  // If the left button is pressed, move clockwise and light the left LED
  if(leftStatus && !rightStatus) {
    digitalWrite(MOV_LED_L, HIGH);
    digitalWrite(MOV_LED_R, LOW);
    oneStep(Clockwise, STP1_IN1, STP1_IN2, STP1_IN3, STP1_IN4);
    oneStep(Clockwise, STP2_IN1, STP2_IN2, STP2_IN3, STP2_IN4);
    delay(moveDelay);
  }

  // If the right button is pressed, move counterclockwise and light the right LED
  else if (!leftStatus && rightStatus) {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, HIGH);
    oneStep(Counterclockwise, STP1_IN1, STP1_IN2, STP1_IN3, STP1_IN4);
    oneStep(Counterclockwise, STP2_IN1, STP2_IN2, STP2_IN3, STP2_IN4);
    delay(moveDelay);
  }

  // If both or no buttons are pressed, halt movement and turn off both LEDs
  else {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, LOW);
    oneStep(NoDirection, STP1_IN1, STP1_IN2, STP1_IN3, STP1_IN4);
    oneStep(NoDirection, STP2_IN1, STP2_IN2, STP2_IN3, STP2_IN4);
  }
}

// This will move a motor one step clockwise or counterclockwise. Because of the way stepper motors work,
// we must keep track of the step count and set bits high or low depending on the current position
void oneStep(MotorDirection dir, int in1, int in2, int in3, int in4) {
  switch(stepPosition) {
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
      Serial.println("Step number exceeded limit: " + stepPosition);
  }

  // Set the step position one digit clockwise or counterclockwise
  // This is so the program knows which bit to flip high next
  if(dir == Counterclockwise) {
    stepPosition++;
    totalStepCount++;
  }
  else {
    stepPosition--;
    totalStepCount--;
  } 

  // Keep the step position between 0-3
  if(stepPosition > 3) stepPosition = 0;
  else if(stepPosition < 0) stepPosition = 3;
}