#include <Stepper.h>

#define SERIAL_SPEED 115200

const int stepsPerRevolution = 2048;

// The vertical stepper motor controls the up and down movements of the tonearm, such as lifting the stylus off of the 
// record or setting it down
#define STEPPER_VERTICAL_PIN1 2
#define STEPPER_VERTICAL_PIN2 3
#define STEPPER_VERTICAL_PIN3 4
#define STEPPER_VERTICAL_PIN4 5
Stepper VerticalTonearmMotor = Stepper(stepsPerRevolution, STEPPER_VERTICAL_PIN1, STEPPER_VERTICAL_PIN3, STEPPER_VERTICAL_PIN2, STEPPER_VERTICAL_PIN4);

// The horizontal stepper motor controls the left and right movements of the tonearm, such as positioning it at any
// horizontal axis so the vertical movement can place the tonearm rests at the correct location
#define STEPPER_HORIZONTAL_PIN1 10
#define STEPPER_HORIZONTAL_PIN2 11
#define STEPPER_HORIZONTAL_PIN3 12
#define STEPPER_HORIZONTAL_PIN4 13
Stepper HorizontalTonearmMotor = Stepper(stepsPerRevolution, STEPPER_HORIZONTAL_PIN1, STEPPER_HORIZONTAL_PIN3, STEPPER_HORIZONTAL_PIN2, STEPPER_HORIZONTAL_PIN4);

// Left/right buttons for clockwise/counterclockwise test movements. These exist for troubleshooting/testing purposes only.
#define LEFT_BUTTON 6
#define RIGHT_BUTTON 7

// These LEDs light up when moving one direction or another. They exist for troubleshooting/testing purposes only.
#define MOV_LED_L 8
#define MOV_LED_R 9

// This is an internal step count for the motors, so we know how many steps it has taken
int32_t verticalStepCount = 0;
int32_t horizontalStepCount = 0;

// The motors used in this project are 28BYJ-48 stepper motors, which I've found to cap at 11 RPM 
// before becoming too unreliable. 8 or 9 I've found to be a good balance for speed and reliability at 5v DC
int movementRPM = 8;

void setup() {
  pinMode(STEPPER_VERTICAL_PIN1, OUTPUT);
  pinMode(STEPPER_VERTICAL_PIN2, OUTPUT);
  pinMode(STEPPER_VERTICAL_PIN3, OUTPUT);
  pinMode(STEPPER_VERTICAL_PIN4, OUTPUT);
  VerticalTonearmMotor.setSpeed(movementRPM);
  
  pinMode(STEPPER_HORIZONTAL_PIN1, OUTPUT);
  pinMode(STEPPER_HORIZONTAL_PIN2, OUTPUT);
  pinMode(STEPPER_HORIZONTAL_PIN3, OUTPUT);
  pinMode(STEPPER_HORIZONTAL_PIN4, OUTPUT);
  HorizontalTonearmMotor.setSpeed(movementRPM);
  
  pinMode(LEFT_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);

  pinMode(MOV_LED_L, OUTPUT);
  pinMode(MOV_LED_R, OUTPUT);

  Serial.begin(SERIAL_SPEED);

  homeBothAxes();
}

void loop() {
  bool leftButtonStatus = digitalRead(LEFT_BUTTON);
  bool rightButtonStatus = digitalRead(RIGHT_BUTTON);

  // If the left button is pressed, move clockwise and light the left LED
  if(leftButtonStatus && !rightButtonStatus) {
    digitalWrite(MOV_LED_L, HIGH);
    digitalWrite(MOV_LED_R, LOW);

    VerticalTonearmMotor.step(1);
    verticalStepCount++;

    HorizontalTonearmMotor.step(1);
    horizontalStepCount++;
  }

  // If the right button is pressed, move counterclockwise and light the right LED
  else if (!leftButtonStatus && rightButtonStatus) {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, HIGH);

    VerticalTonearmMotor.step(-1);
    verticalStepCount--;

    HorizontalTonearmMotor.step(-1);
    horizontalStepCount--;
  }
  
  // No movement, so shut the LEDs off
  else {
    digitalWrite(MOV_LED_L, LOW);
    digitalWrite(MOV_LED_R, LOW);
  }
}

// This handles homing the tonearm, which must be done every time the application is started
void homeBothAxes() {

  // Now that we are homed, the step count can be reset to 0 for each axis
  verticalStepCount = 0;
  horizontalStepCount = 0;
}