// This library is made specifically for automatic turntable firmware/hardware designed by Patrick Nelson.
// It may have other uses outside of the specific scenarios where it is used here, but that should
// be investigated on an individual basis by whoever stumbles upon this code.

#include "TonearmMovementController.h"

TonearmMovementController::TonearmMovementController(Multiplexer inputMux, uint8_t motorPin1, uint8_t motorPin2, uint8_t motorPin3, uint8_t motorPin4, Stepper tonearmMotors, uint8_t motorSelectPin, uint8_t horizontalSolenoidPin, uint8_t verticalLowerLimit, uint8_t verticalUpperLimit): inputMux(inputMux), tonearmMotors(tonearmMotors) {
    this->motorPin1 = motorPin1;
    this->motorPin2 = motorPin2;
    this->motorPin3 = motorPin3;
    this->motorPin4 = motorPin4;
    
    this->motorSelectPin = motorSelectPin;
    pinMode(motorSelectPin, OUTPUT);

    this->horizontalSolenoidPin = horizontalSolenoidPin;
    pinMode(horizontalSolenoidPin, OUTPUT);

    this->verticalLowerLimit = verticalLowerLimit;
    this->verticalUpperLimit = verticalUpperLimit;
}

// Moves the tonearm to a specified destination.
MovementResult TonearmMovementController::moveTonearmHorizontally(uint8_t destinationSensor, unsigned int timeout, int calibration, uint8_t speed) {
    this->tonearmMotors.setSpeed(speed);
    unsigned int movementStepCount = 0;
    digitalWrite(this->motorSelectPin, MotorAxis::Horizontal);

    bool currentSensorStatus = this->inputMux.readDigitalValue(destinationSensor); 
    TonearmMovementDirection movementDirection = currentSensorStatus ? TonearmMovementDirection::Clockwise : TonearmMovementDirection::Counterclockwise;

    digitalWrite(this->horizontalSolenoidPin, HIGH);

    // Keep moving until the sensor is the opposite of what it started at
    while(this->inputMux.readDigitalValue(destinationSensor) == currentSensorStatus) {
      this->tonearmMotors.step(movementDirection);

      // If the sensor isn't hit in the expected time, the movement failed.
      if(movementStepCount++ >= timeout) {
        digitalWrite(this->horizontalSolenoidPin, LOW);

        if(movementDirection == TonearmMovementDirection::Clockwise)
          return MovementResult::HorizontalClockwiseDirectionError;

        else
          return MovementResult::HorizontalCounterclockwiseDirectionError;
      }
    }

    this->horizontalRelativeMove(calibration, speed); // Move tonearm additional steps to account for calibration set by rear potentiometers

    this->releaseCurrentFromMotors();

    return MovementResult::Success;
}

MovementResult TonearmMovementController::moveTonearmVertically(uint8_t destinationSensor, unsigned int timeout, uint8_t speed) {

  TonearmMovementDirection movementDirection = TonearmMovementDirection::NoDirection;

  // Only perform the operation if the tonearm is not already at its destination.
  if(!this->inputMux.readDigitalValue(destinationSensor)) {
    if(destinationSensor == this->verticalLowerLimit) {
      movementDirection = TonearmMovementDirection::Down; // Must move down to reach the lower limit.
    }
    else if(destinationSensor == this->verticalUpperLimit) {
      movementDirection = TonearmMovementDirection::Up; // Must move up to reach the upper limit.
    }

    this->tonearmMotors.setSpeed(speed);
    unsigned int movementStepCount = 0;
    digitalWrite(this->motorSelectPin, MotorAxis::Vertical);

    // Move the vertical axis until it reaches its destination
    while(!this->inputMux.readDigitalValue(destinationSensor)) {
      this->tonearmMotors.step(movementDirection);

      // Timeout so that if the motor gets stuck, it does not get damaged.
      if(movementStepCount++ >= timeout) {
        this->releaseCurrentFromMotors();
        if(movementDirection == TonearmMovementDirection::Up)
          return MovementResult::VerticalPositiveDirectionError;

        else
          return MovementResult::VerticalNegativeDirectionError;
      }
    }

    this->releaseCurrentFromMotors();
  }

  return MovementResult::Success;
}

void TonearmMovementController::horizontalRelativeMove(int steps, uint8_t speed) {
  unsigned int movementStepCount = 0;

  digitalWrite(this->motorSelectPin, MotorAxis::Horizontal);
  this->tonearmMotors.setSpeed(speed);

  TonearmMovementDirection movementDirection = (steps > 0) ? TonearmMovementDirection::Clockwise : TonearmMovementDirection::Counterclockwise;
  steps = abs(steps);

  while(movementStepCount++ < steps) {
    this->tonearmMotors.step(movementDirection);
  }

  this->releaseCurrentFromMotors();
}

// This is used to release current from both motors so they aren't drawing power when not in use.
void TonearmMovementController::releaseCurrentFromMotors() {
    digitalWrite(this->motorPin1, LOW);
    digitalWrite(this->motorPin2, LOW);
    digitalWrite(this->motorPin3, LOW);
    digitalWrite(this->motorPin4, LOW);

    digitalWrite(this->motorSelectPin, LOW);
}
