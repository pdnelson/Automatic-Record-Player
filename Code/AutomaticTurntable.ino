#include <Stepper.h>
#include "ErrorCode.h"
#include "MovementPosition.h"
#include "MotorAxis.h"

#define STEPS_PER_REVOLUTION 2048

// The vertical stepper motor controls the up and down movements of the tonearm, such as lifting the stylus off of the 
// record or setting it down.
#define STEPPER_VERTICAL_PIN1 0
#define STEPPER_VERTICAL_PIN2 1
#define STEPPER_VERTICAL_PIN3 2
#define STEPPER_VERTICAL_PIN4 3
Stepper VerticalTonearmMotor = Stepper(STEPS_PER_REVOLUTION, STEPPER_VERTICAL_PIN1, STEPPER_VERTICAL_PIN3, STEPPER_VERTICAL_PIN2, STEPPER_VERTICAL_PIN4);

// The horizontal stepper motor controls the left and right movements of the tonearm, such as positioning it at a
// horizontal axis so the vertical movement can place the tonearm at the correct location.
#define STEPPER_HORIZONTAL_PIN1 4
#define STEPPER_HORIZONTAL_PIN2 5
#define STEPPER_HORIZONTAL_PIN3 6
#define STEPPER_HORIZONTAL_PIN4 7
Stepper HorizontalTonearmMotor = Stepper(STEPS_PER_REVOLUTION, STEPPER_HORIZONTAL_PIN1, STEPPER_HORIZONTAL_PIN3, STEPPER_HORIZONTAL_PIN2, STEPPER_HORIZONTAL_PIN4);

// Buttons that the user can press to execute certain movements.
#define HOME_BUTTON 8
#define PLAY_BUTTON 10
#define PAUSE_BUTTON 12

// Indicator lights so we can tell what the turntable is currently doing.
#define MOVEMENT_STATUS_LED 11
#define PAUSE_STATUS_LED 13

// Positioning sensors for the vertical tonearm movement. The lower limit switch designates "home" for the tonearm's vertical position.
// The upper sensor is the "pause" position, and will allow the tonearm to go higher to engage with the gearing that moves horizontally.
#define VERTICAL_PICKUP_LIMIT A0
#define VERTICAL_HOME_LIMIT A1

// Positioning sensors for the horizontal tonearm movement. Each one represents a different point that the horizontal tonearm could be in,
// and will allow the various funcrtions to move it in the correct direction depending on where it currently is.
#define HORIZONTAL_HOME_SENSOR A2 // Tells the tonearm where "home" is horizontally.
#define HORIZONTAL_PLAY_SENSOR A3 // Tells the tonearm where to drop the stylus down on the record.
#define HORIZONTAL_PICKUP_SENSOR A4 // Tells the tonearm where the end of the record is.

// This is used to engage the horizontal gears for movement. This is needed so that the gears aren't engaged
// when a record is playing or any other times, otherwise the record would not be able to move the tonearm
// very well...
#define HORIZONTAL_GEARING_SOLENOID 9

// This tells us whether the user flipped the 3-way switch to "automatic" or "manual."
// Automatic will automatically home the turntable at the end of the record, while manual
// will not. Even with manual selected, the "home," "pause" and "play" buttons will still work.
#define AUTO_OR_MANUAL_SWITCH A5 // Auto = high; manual = low

// The motors used in this project are 28BYJ-48 stepper motors, which I've found to cap at 11 RPM 
// before becoming too unreliable. 8 or 9 I've found to be a good balance for speed and reliability at 5v DC.
#define MOVEMENT_RPM 8

// These are timeouts used for error checking, so the hardware doesn't damage itself.
// Essentially, if the steps exceed this number and the motor has not yet reached its
// destination, an error has occurred.
#define PLAY_TIMEOUT_STEPS 50
#define HOME_TIMEOUT_STEPS 50 // TODO: Determine values for these fields.
#define PAUSE_TIMEOUT_STEPS 1000 // Number of steps until a vertical movement should time out.
#define STEPS_TO_RELIEVE_LIMIT 250 // Number of steps it takes to un-click a limit switch.

#define TONEARM_DOWN_SPEED 5 // The speed at which the tonarm gets set down.
#define TONEARM_LIFT_SPEED 7 // The speed at which the tonearm gets lifted up.
#define TONEARM_HORIZONTAL_SPEED 8 // The speed at which the tonearm moves horizontally.

// Step counts used for error checking. We will have an idea of how many steps a movement should take,
// so here we are keeping track of those so we know it doesn't exceed the limits defined above.
int movementStepCount = 0;

void setup() {
  pinMode(STEPPER_VERTICAL_PIN1, OUTPUT);
  pinMode(STEPPER_VERTICAL_PIN2, OUTPUT);
  pinMode(STEPPER_VERTICAL_PIN3, OUTPUT);
  pinMode(STEPPER_VERTICAL_PIN4, OUTPUT);
  VerticalTonearmMotor.setSpeed(MOVEMENT_RPM);
  
  pinMode(STEPPER_HORIZONTAL_PIN1, OUTPUT);
  pinMode(STEPPER_HORIZONTAL_PIN2, OUTPUT);
  pinMode(STEPPER_HORIZONTAL_PIN3, OUTPUT);
  pinMode(STEPPER_HORIZONTAL_PIN4, OUTPUT);
  HorizontalTonearmMotor.setSpeed(MOVEMENT_RPM);
  
  pinMode(HOME_BUTTON, INPUT);
  pinMode(PLAY_BUTTON, INPUT);
  pinMode(PAUSE_BUTTON, INPUT);

  pinMode(MOVEMENT_STATUS_LED, OUTPUT);
  pinMode(PAUSE_STATUS_LED, OUTPUT);

  pinMode(HORIZONTAL_GEARING_SOLENOID, OUTPUT);

  // If the turntable is turned on to "automatic," then home the whole tonearm.
  if(digitalRead(AUTO_OR_MANUAL_SWITCH)) {
    homeTonearm();
  }

  // Otherwise, we only want to home the vertical axis, which will drop the tonearm in its current location.
  else {
    performTonearmMovement(MovementPosition::LowerLimit, TONEARM_DOWN_SPEED);
  }
}

// This sits and waits for any of the command buttons to be pressed.
// As soon as a button is pressed, the corresponding command routine is executed.
void loop() {
  if(digitalRead(PLAY_BUTTON)) {
    playRoutine();
  }

  if(digitalRead(PAUSE_BUTTON)) {
    pauseAndWaitUntilUnpaused();
  }

  if(digitalRead(HOME_BUTTON) || digitalRead(HORIZONTAL_PICKUP_SENSOR)) {
    homeTonearm();
  }
}

void homeTonearm() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);

  performTonearmMovement(MovementPosition::UpperLimit, TONEARM_LIFT_SPEED);

  performTonearmMovement(MovementPosition::HomePosition, TONEARM_HORIZONTAL_SPEED);

  performTonearmMovement(MovementPosition::LowerLimit, TONEARM_DOWN_SPEED);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);
}

// This is the pause routine that will lift up the tonearm from the record until the user "unpauses" by pressing the
// pause button again
void pauseAndWaitUntilUnpaused() {
  digitalWrite(PAUSE_STATUS_LED, HIGH);

  performTonearmMovement(MovementPosition::UpperLimit, TONEARM_LIFT_SPEED);

  // Wait for the user to unpause.
  while(!digitalRead(PAUSE_BUTTON)) {
    delay(1);
  }

  performTonearmMovement(MovementPosition::LowerLimit, TONEARM_DOWN_SPEED);

  digitalWrite(PAUSE_STATUS_LED, LOW);
}

void playRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);

  performTonearmMovement(MovementPosition::UpperLimit, TONEARM_LIFT_SPEED);

  performTonearmMovement(MovementPosition::PlayPosition, TONEARM_HORIZONTAL_SPEED);

  performTonearmMovement(MovementPosition::LowerLimit, TONEARM_DOWN_SPEED);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);
}

// Performs the tonearm movement, whether that be vertical or horizontal.
void performTonearmMovement(MovementPosition destination, int speed) {

    // We always want to make sure the vertical step count starts at 0 so there aren't leftover steps
    // from a previous function and the turntable is set to an error state early.
    movementStepCount = 0;

    switch(destination) {

      case MovementPosition::LowerLimit:
        VerticalTonearmMotor.setSpeed(speed);
        while(!digitalRead(VERTICAL_HOME_LIMIT))
        {
          VerticalTonearmMotor.step(-1);

          // If the limit switch isn't hit in the expected time, or the wrong limit switch is hit, error the turntable.
          if(movementStepCount++ >= PAUSE_TIMEOUT_STEPS || (digitalRead(VERTICAL_PICKUP_LIMIT) && movementStepCount > STEPS_TO_RELIEVE_LIMIT)) {
            setErrorState(ErrorCode::VerticalHomeError);
          }
        }
        releaseCurrentFromMotor(MotorAxis::Vertical);
        break;

      case MovementPosition::UpperLimit:
        VerticalTonearmMotor.setSpeed(speed);
        while(!digitalRead(VERTICAL_PICKUP_LIMIT)) {
          VerticalTonearmMotor.step(1);
          
          // If the limit switch isn't hit in the expected time, or the wrong limit switch is hit, error the turntable.
          if(movementStepCount++ >= PAUSE_TIMEOUT_STEPS || (digitalRead(VERTICAL_HOME_LIMIT) && movementStepCount > STEPS_TO_RELIEVE_LIMIT)) {
            setErrorState(ErrorCode::VerticalPickupError);
          }
        }
        releaseCurrentFromMotor(MotorAxis::Vertical);
        break;

      case MovementPosition::PlayPosition:
        digitalWrite(HORIZONTAL_GEARING_SOLENOID, HIGH);
        HorizontalTonearmMotor.setSpeed(speed);

        int movementDirection = 1;
        bool currentSensorStatus = digitalRead(HORIZONTAL_PLAY_SENSOR);
        if(currentSensorStatus) movementDirection = -1; // TODO: Verify this direction is correct

        // Keep moving until the sensor is the opposite of what it started at
        while(digitalRead(HORIZONTAL_PLAY_SENSOR) == currentSensorStatus) {
          HorizontalTonearmMotor.step(movementDirection);

          // If the sensor isn't hit in the expected time, error the turntable.
          if(movementStepCount++ >= PLAY_TIMEOUT_STEPS) {
            setErrorState(ErrorCode::PlayError);
          }
        }
        
        releaseCurrentFromMotor(MotorAxis::Horizontal);
        break;

      case MovementPosition::HomePosition:
        digitalWrite(HORIZONTAL_GEARING_SOLENOID, HIGH);
        HorizontalTonearmMotor.setSpeed(speed);

        int movementDirection = 1;
        bool currentSensorStatus = digitalRead(HORIZONTAL_HOME_SENSOR);
        if(currentSensorStatus) movementDirection = -1; // TODO: Verify this direction is correct

        // Keep moving until the sensor is the opposite of what it started at
        while(digitalRead(HORIZONTAL_HOME_SENSOR) == currentSensorStatus) {
          HorizontalTonearmMotor.step(movementDirection);

          // If the sensor isn't hit in the expected time, error the turntable.
          if(movementStepCount++ >= HOME_TIMEOUT_STEPS) {
            setErrorState(ErrorCode::HorizontalHomeError);
          }
        }

        releaseCurrentFromMotor(MotorAxis::Horizontal);
        break;
    }
}

// This is used to release current from one or both motors so they aren't drawing power when not in use.
void releaseCurrentFromMotor(MotorAxis axis) {

  if(axis == MotorAxis::Vertical || axis == MotorAxis::VerticalAndHorizontal) {
    digitalWrite(STEPPER_VERTICAL_PIN1, LOW);
    digitalWrite(STEPPER_VERTICAL_PIN2, LOW);
    digitalWrite(STEPPER_VERTICAL_PIN3, LOW);
    digitalWrite(STEPPER_VERTICAL_PIN4, LOW);
  }

  if(axis == MotorAxis::Horizontal || axis == MotorAxis::VerticalAndHorizontal) {
    digitalWrite(STEPPER_HORIZONTAL_PIN1, LOW);
    digitalWrite(STEPPER_HORIZONTAL_PIN2, LOW);
    digitalWrite(STEPPER_HORIZONTAL_PIN3, LOW);
    digitalWrite(STEPPER_HORIZONTAL_PIN4, LOW);
    digitalWrite(HORIZONTAL_GEARING_SOLENOID, LOW);
  }
}

// This stops all movement and sets the turntable in an error state to prevent damage.
// This will be called if a motor stall has been detected.
// The user will have to restart the turntable if this occurs.
void setErrorState(ErrorCode errorCode) {

  // Turn off all motors and LEDs.
  releaseCurrentFromMotor(MotorAxis::VerticalAndHorizontal);
  digitalWrite(PAUSE_STATUS_LED, LOW);
  digitalWrite(MOVEMENT_STATUS_LED, LOW);

  switch(errorCode) {

    // Slow-blink pause LED.
    case ErrorCode::VerticalHomeError:
      blinkLed(PAUSE_STATUS_LED, 1000);

    // Fast-blink pause LED.
    case ErrorCode::VerticalPickupError:
      blinkLed(PAUSE_STATUS_LED, 150);

    // Slow-blink movement LED.
    case ErrorCode::HorizontalHomeError:
      blinkLed(MOVEMENT_STATUS_LED, 1000);

    // Fast-blink movement LED.
    case ErrorCode::PlayError:
      blinkLed(MOVEMENT_STATUS_LED, 150);
  }
}

// Blinks a specific LED indefinitely.
// For use only with error codes.
void blinkLed(int led, int interval) {
  while(true) {
    digitalWrite(led, LOW);
    delay(interval);
    digitalWrite(led, HIGH);
    delay(interval);
  }
}