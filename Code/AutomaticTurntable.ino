#include <Stepper.h>

#define SERIAL_SPEED 115200

const int stepsPerRevolution = 2048;

// First stepper motor
#define STEPPER_1_PIN1 2
#define STEPPER_1_PIN2 3
#define STEPPER_1_PIN3 4
#define STEPPER_1_PIN4 5
Stepper motor1 = Stepper(stepsPerRevolution, STEPPER_1_PIN1, STEPPER_1_PIN3, STEPPER_1_PIN2, STEPPER_1_PIN4);

// Second stepper motor
#define STEPPER_2_PIN1 13
#define STEPPER_2_PIN2 12
#define STEPPER_2_PIN3 11
#define STEPPER_2_PIN4 10
Stepper motor2 = Stepper(stepsPerRevolution, STEPPER_2_PIN1, STEPPER_2_PIN3, STEPPER_2_PIN2, STEPPER_2_PIN4);

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
int movementRPM = 8;

void setup() {
  pinMode(STEPPER_1_PIN1, OUTPUT);
  pinMode(STEPPER_1_PIN2, OUTPUT);
  pinMode(STEPPER_1_PIN3, OUTPUT);
  pinMode(STEPPER_1_PIN4, OUTPUT);
  motor1.setSpeed(movementRPM);
  
  pinMode(STEPPER_2_PIN1, OUTPUT);
  pinMode(STEPPER_2_PIN2, OUTPUT);
  pinMode(STEPPER_2_PIN3, OUTPUT);
  pinMode(STEPPER_2_PIN4, OUTPUT);
  motor2.setSpeed(movementRPM);
  
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
    motor1.step(1);
    motor2.step(1);
  }

  // If the right button is pressed, move counterclockwise and light the right LED
  else if (!leftButtonStatus && rightButtonStatus) {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, HIGH);
    motor1.step(-1);
    motor2.step(-1);
  }
  
  // No movement, so shut the LEDs off
  else {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, LOW);
  }
}