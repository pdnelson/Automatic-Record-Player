#include <Stepper.h>
#include <Multiplexer.h>
#include "MultiplexerInput.h"
#include "ErrorCode.h"
#include "MotorAxis.h"
#include "TonearmMovementDirection.h"

#define SERIAL_SPEED 115200

#define STEPS_PER_REVOLUTION 2048

// These motor pins channel into a quad 2-channel demultiplexer, so that either the vertical or horizontal motors receive
// the voltages. Only one of these motors will ever be moving at once
#define MOTOR_PIN1 2
#define MOTOR_PIN2 4
#define MOTOR_PIN3 7
#define MOTOR_PIN4 8
Stepper TonearmMotor = Stepper(STEPS_PER_REVOLUTION, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

// This is the pin used to select which motor we are moving, using the demultiplexer.
#define MOTOR_AXIS_SELECTOR 9

// This is used to engage the horizontal gears for movement. This is needed so that the gears aren't engaged
// when a record is playing or any other times, otherwise the record would not be able to move the tonearm
// very well...
#define HORIZONTAL_GEARING_SOLENOID 10

// Indicator lights so we can tell what the turntable is currently doing.
#define MOVEMENT_STATUS_LED 11
#define PAUSE_STATUS_LED 12

// These are the selector pins for the multiplexer that is used to handle all inputs.
#define MUX_OUTPUT 13
#define MUX_SELECTOR_A A0
#define MUX_SELECTOR_B A1
#define MUX_SELECTOR_C A2
Multiplexer mux = Multiplexer(MUX_OUTPUT, MUX_SELECTOR_A, MUX_SELECTOR_B, MUX_SELECTOR_C);

// The motors used in this project are 28BYJ-48 stepper motors, which I've found to cap at 11 RPM 
// before becoming too unreliable. 8 or 9 I've found to be a good balance for speed and reliability at 5v DC.
#define MOVEMENT_RPM 8

// These are timeouts used for error checking, so the hardware doesn't damage itself.
// Essentially, if the steps exceed this number and the motor has not yet reached its
// destination, an error has occurred.
#define PLAY_TIMEOUT_STEPS 1500 // Number of steps until movement to the horizontal "play" sensor should time out.
#define HOME_TIMEOUT_STEPS 1500 // Number of steps until movement to the horizontal "home" sensor should time out.
#define VERTICAL_TIMEOUT_STEPS 1000 // Number of steps until a vertical movement should time out.
#define STEPS_TO_RELIEVE_LIMIT 250 // Number of steps it takes to un-click a limit switch.

// Step counts used for error checking. We will have an idea of how many steps a movement should take,
// so here we are keeping track of those so we know it doesn't exceed the limits defined above.
unsigned int movementStepCount;

// Movement variables
TonearmMovementDirection movementDirection;
bool currentSensorStatus = false;

void setup() {
  Serial.begin(SERIAL_SPEED);

  TonearmMotor.setSpeed(MOVEMENT_RPM);

  pinMode(MOTOR_AXIS_SELECTOR, OUTPUT);

  mux.setDelayMicroseconds(10);

  pinMode(HORIZONTAL_GEARING_SOLENOID, OUTPUT);

  pinMode(MOVEMENT_STATUS_LED, OUTPUT);
  pinMode(PAUSE_STATUS_LED, OUTPUT);

  // If the turntable is turned on to "automatic," then home the whole tonearm if it is not already home.
  if(mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) && 
    !mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) {
    homeTonearm();
  }

  // Otherwise, we only want to home the vertical axis, which will drop the tonearm in its current location.
  else {
    if(!moveTonearmToSensor(MotorAxis::Vertical, MultiplexerInput::VerticalLowerLimit, 8, VERTICAL_TIMEOUT_STEPS))
      setErrorState(ErrorCode::VerticalHomeError);
  }
}

// This sits and waits for any of the command buttons to be pressed.
// As soon as a button is pressed, the corresponding command routine is executed.
void loop() {
  if(mux.readDigitalValue(MultiplexerInput::PauseButton)) {
    pauseAndWaitUntilUnpaused();
  }
  
  if(mux.readDigitalValue(MultiplexerInput::PlayHomeButton) ||
   (!mux.readDigitalValue(MultiplexerInput::HorizontalPickupOpticalSensor) && 
     mux.readDigitalValue(MultiplexerInput::AutoManualSwitch))) {

    // If the tonearm is past the location of the play sensor, then this button will home it. Otherwise, it will execute
    // the play routine.
    if(mux.readDigitalValue(MultiplexerInput::HorizontalPlayOpticalSensor)) 
      playRoutine();
    else 
      homeTonearm();
  }
}

void homeTonearm() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);

  if(!moveTonearmToSensor(MotorAxis::Vertical, MultiplexerInput::VerticalUpperLimit, 8, VERTICAL_TIMEOUT_STEPS))
    setErrorState(ErrorCode::VerticalPickupError);

  if(!moveTonearmToSensor(MotorAxis::Horizontal, MultiplexerInput::HorizontalHomeOpticalSensor, 7, HOME_TIMEOUT_STEPS))
    setErrorState(ErrorCode::HorizontalHomeError);

  if(!moveTonearmToSensor(MotorAxis::Vertical, MultiplexerInput::VerticalLowerLimit, 8, VERTICAL_TIMEOUT_STEPS))
    setErrorState(ErrorCode::VerticalHomeError);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);
}

// This is the pause routine that will lift up the tonearm from the record until the user "unpauses" by pressing the
// pause button again
void pauseAndWaitUntilUnpaused() {
  digitalWrite(PAUSE_STATUS_LED, HIGH);

  if(!moveTonearmToSensor(MotorAxis::Vertical, MultiplexerInput::VerticalUpperLimit, 8, VERTICAL_TIMEOUT_STEPS))
    setErrorState(ErrorCode::VerticalPickupError);

  // Wait for the user to unpause.
  while(!mux.readDigitalValue(MultiplexerInput::PauseButton)) {
    delay(1);
  }

  if(!moveTonearmToSensor(MotorAxis::Vertical, MultiplexerInput::VerticalLowerLimit, 8, VERTICAL_TIMEOUT_STEPS))
    setErrorState(ErrorCode::VerticalHomeError);

  digitalWrite(PAUSE_STATUS_LED, LOW);
}

void playRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);

  if(!moveTonearmToSensor(MotorAxis::Vertical, MultiplexerInput::VerticalUpperLimit, 8, VERTICAL_TIMEOUT_STEPS))
    setErrorState(ErrorCode::VerticalPickupError);

  if(!moveTonearmToSensor(MotorAxis::Horizontal, MultiplexerInput::HorizontalPlayOpticalSensor, 4, PLAY_TIMEOUT_STEPS))
    setErrorState(ErrorCode::PlayError);

  if(!moveTonearmToSensor(MotorAxis::Vertical, MultiplexerInput::VerticalLowerLimit, 8, VERTICAL_TIMEOUT_STEPS))
    setErrorState(ErrorCode::VerticalHomeError);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);
}

// Moves the tonearm to a specified destination.
bool moveTonearmToSensor(MotorAxis axis, MultiplexerInput destinationSensor, uint8_t speed, unsigned int timeout) {
    digitalWrite(MOTOR_AXIS_SELECTOR, axis);

    movementStepCount = 0;
    currentSensorStatus = mux.readDigitalValue(destinationSensor); 

    TonearmMotor.setSpeed(speed);

    if(axis == MotorAxis::Horizontal) {
      digitalWrite(HORIZONTAL_GEARING_SOLENOID, HIGH);
      movementDirection = currentSensorStatus ? TonearmMovementDirection::Counterclockwise : TonearmMovementDirection::Clockwise;
    }
    else {
      // Because the tonearm would have to move in opposite directions to set each limit switch "high," we must tell it to move
      // up or down depending on the destination sensor.
      if(destinationSensor == MultiplexerInput::VerticalLowerLimit)
        movementDirection = currentSensorStatus ? TonearmMovementDirection::Positive : TonearmMovementDirection::Negative;
      else
        movementDirection = currentSensorStatus ? TonearmMovementDirection::Negative : TonearmMovementDirection::Positive;
    }

    // Keep moving until the sensor is the opposite of what it started at
    while(mux.readDigitalValue(destinationSensor) == currentSensorStatus) {
      TonearmMotor.step(movementDirection);

      // If the sensor isn't hit in the expected time, the movement failed.
      if(movementStepCount++ >= timeout) {
        // TO DO: Push the motor a few steps in the opposite direction to un-lock the gears
        releaseCurrentFromMotors();
        digitalWrite(HORIZONTAL_GEARING_SOLENOID, LOW);
        return false;
      }
    }
    
    releaseCurrentFromMotors();

    // If it is a horizontal movement, make sure all movement has ceased before releasing the horizontal solenoid
    if(axis == MotorAxis::Horizontal) {
      delay(500);
      digitalWrite(HORIZONTAL_GEARING_SOLENOID, LOW);
    }

    return true;
}

// This is used to release current from both motors so they aren't drawing power when not in use.
void releaseCurrentFromMotors() {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);

    digitalWrite(MOTOR_AXIS_SELECTOR, LOW);
}

// This stops all movement and sets the turntable in an error state to prevent damage.
// This will be called if a motor stall has been detected.
// The user will have to restart the turntable if this occurs.
void setErrorState(ErrorCode errorCode) {
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