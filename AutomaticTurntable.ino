#include "MotorDirection.cpp"

#define SERIAL_SPEED 115200

// First stepper motor
#define STEPPER_1_PIN1 2
#define STEPPER_1_PIN2 3
#define STEPPER_1_PIN3 4
#define STEPPER_1_PIN4 5

// Second stepper motor
#define STEPPER_2_PIN1 13
#define STEPPER_2_PIN2 12
#define STEPPER_2_PIN3 11
#define STEPPER_2_PIN4 10

// Left/right buttons for clockwise/counterclockwise test movements. These exist for troubleshooting/testing purposes only.
#define LEFT_BUTTON 6
#define RIGHT_BUTTON 7

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
  pinMode(STEPPER_1_PIN1, OUTPUT);
  pinMode(STEPPER_1_PIN2, OUTPUT);
  pinMode(STEPPER_1_PIN3, OUTPUT);
  pinMode(STEPPER_1_PIN4, OUTPUT);
  
  pinMode(STEPPER_2_PIN1, OUTPUT);
  pinMode(STEPPER_2_PIN2, OUTPUT);
  pinMode(STEPPER_2_PIN3, OUTPUT);
  pinMode(STEPPER_2_PIN4, OUTPUT);
  
  pinMode(LEFT_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);

  pinMode(MOV_LED_L, OUTPUT);
  pinMode(MOV_LED_R, OUTPUT);


  Serial.begin(SERIAL_SPEED);
}

void loop() {
  int leftButtonStatus = digitalRead(LEFT_BUTTON);
  int rightButtonStatus = digitalRead(RIGHT_BUTTON);

  // If the left button is pressed, move clockwise and light the left LED
  if(leftButtonStatus && !rightButtonStatus) {
    digitalWrite(MOV_LED_L, HIGH);
    digitalWrite(MOV_LED_R, LOW);
    moveMotorOneStep(Clockwise, STEPPER_1_PIN1, STEPPER_1_PIN2, STEPPER_1_PIN3, STEPPER_1_PIN4);
    moveMotorOneStep(Clockwise, STEPPER_2_PIN1, STEPPER_2_PIN2, STEPPER_2_PIN3, STEPPER_2_PIN4);
    delay(moveDelay);
  }

  // If the right button is pressed, move counterclockwise and light the right LED
  else if (!leftButtonStatus && rightButtonStatus) {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, HIGH);
    moveMotorOneStep(Counterclockwise, STEPPER_1_PIN1, STEPPER_1_PIN2, STEPPER_1_PIN3, STEPPER_1_PIN4);
    moveMotorOneStep(Counterclockwise, STEPPER_2_PIN1, STEPPER_2_PIN2, STEPPER_2_PIN3, STEPPER_2_PIN4);
    delay(moveDelay);
  }

  // If both or no buttons are pressed, halt movement and turn off both LEDs
  else {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, LOW);
    moveMotorOneStep(NoDirection, STEPPER_1_PIN1, STEPPER_1_PIN2, STEPPER_1_PIN3, STEPPER_1_PIN4);
    moveMotorOneStep(NoDirection, STEPPER_2_PIN1, STEPPER_2_PIN2, STEPPER_2_PIN3, STEPPER_2_PIN4);
  }
}

// This will move a motor one step clockwise or counterclockwise. Because of the way stepper motors work,
// we must keep track of the step count and set bits high or low depending on the current position
void moveMotorOneStep(MotorDirection dir, int in1, int in2, int in3, int in4) {
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
  else if(dir == Clockwise) {
    stepPosition--;
    totalStepCount--;
  } 

  // Keep the step position between 0-3
  if(stepPosition > 3) stepPosition = 0;
  else if(stepPosition < 0) stepPosition = 3;
}