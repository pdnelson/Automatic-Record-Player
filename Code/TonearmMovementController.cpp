// This library is made specifically for automatic turntable firmware/hardware designed by Patrick Nelson.
// It may have other uses outside of the specific scenarios where it is used here, but that should
// be investigated on an individual basis by whoever stumbles upon this code.

#include "TonearmMovementController.h"

TonearmMovementController::TonearmMovementController(
  Multiplexer inputMux, 
  Stepper tonearmMotors, 
  uint8_t motorSelectPin, 
  DcMotor horizontalClutch, 
  uint8_t verticalLowerLimit, 
  uint8_t verticalUpperLimit
  ):  inputMux(inputMux), 
      tonearmMotors(tonearmMotors),
      horizontalClutch(horizontalClutch)
{
    this->motorSelectPin = motorSelectPin;
    pinMode(motorSelectPin, OUTPUT);

    this->verticalLowerLimit = verticalLowerLimit;
    this->verticalUpperLimit = verticalUpperLimit;

    this->clutchEngagementMs = 0;
    this->topMotorSpeed = 0;
    this->verticalTimeout = 0;
}

MovementResult TonearmMovementController::moveUp(uint8_t speed) {
  this->verticalMove(VerticalMovementDirection::Up, speed);
}

MovementResult TonearmMovementController::moveDown(uint8_t speed) {
  this->verticalMove(VerticalMovementDirection::Down, speed);
}

MovementResult TonearmMovementController::seekRecordEdge() {
  // TODO: Implement
}

MovementResult TonearmMovementController::horizontalHome() {
  // TODO: Implement
}

// Moves the tonearm to a specified destination.
// TODO: Delete this method when we are ready to replace it with seekRecordEdge and horizontalHome
MovementResult TonearmMovementController::moveTonearmHorizontally(uint8_t destinationSensor, unsigned int timeout, int calibration, uint8_t speed) {
    // Select horizontal movement.
    digitalWrite(this->motorSelectPin, MotorAxis::Horizontal);

    // Set values needed for movement.
    unsigned int movementStepCount = 0;
    this->tonearmMotors.setSpeed(speed);
    bool currentSensorStatus = digitalRead(destinationSensor); 

    // Determine what direction we're moving.
    HorizontalMovementDirection movementDirection = currentSensorStatus ? HorizontalMovementDirection::Counterclockwise : HorizontalMovementDirection::Clockwise;

    // Engage clutch so gears can move the tonearm.
    this->setClutchPosition(HorizontalClutchPosition::Engage);

    // Keep moving until the sensor is the opposite of what it started at.
    while(digitalRead(destinationSensor) == currentSensorStatus) {
      this->tonearmMotors.step(movementDirection);

      // If the sensor isn't hit within the expected number of steps, the movement failed.
      if(movementStepCount++ >= timeout) {
        this->setClutchPosition(HorizontalClutchPosition::Disengage);

        if(movementDirection == HorizontalMovementDirection::Clockwise)
          return MovementResult::HorizontalClockwiseDirectionError;

        else
          return MovementResult::HorizontalCounterclockwiseDirectionError;
      }
    }

    this->horizontalRelativeMove(calibration, 14, movementDirection);

    delay(200);

    this->releaseCurrentFromMotors();
    this->setClutchPosition(HorizontalClutchPosition::Disengage);

    return MovementResult::Success;
}

MovementResult TonearmMovementController::verticalMove(VerticalMovementDirection direction, uint8_t speed) {
  // Determine the destination sensor based on the direction passed.
  uint8_t destinationSensor = (direction == VerticalMovementDirection::Up) ? this->verticalUpperLimit : this->verticalLowerLimit;

  // Only perform the operation if the tonearm is not already at its destination.
  if(!this->inputMux.readDigitalValue(destinationSensor)) {
    // Select vertical movement.
    digitalWrite(this->motorSelectPin, MotorAxis::Vertical);

    // Set values needed for movement.
    unsigned int movementStepCount = 0;
    this->tonearmMotors.setSpeed(speed);

    // Move the vertical axis until it reaches its destination.
    while(!this->inputMux.readDigitalValue(destinationSensor)) {
      this->tonearmMotors.step(direction);

      // If the limit isn't hit within the expected number of steps, the movement failed.
      if(movementStepCount++ >= this->verticalTimeout) {
        this->releaseCurrentFromMotors();
        if(direction == VerticalMovementDirection::Up)
          return MovementResult::VerticalPositiveDirectionError;

        else
          return MovementResult::VerticalNegativeDirectionError;
      }
    }

    this->releaseCurrentFromMotors();
  }

  return MovementResult::Success;
}

// This method assumes that the horizontal clutch has already been engaged.
void TonearmMovementController::horizontalRelativeMove(uint16_t steps, uint8_t speed, HorizontalMovementDirection direction) {
  // Select horizontal movement.
  digitalWrite(this->motorSelectPin, MotorAxis::Horizontal);

  // Set values needed for movement.
  unsigned int movementStepCount = 0;
  this->tonearmMotors.setSpeed(speed);

  // Move the tonearm by the number of steps passed as an argument.
  while(movementStepCount++ < steps) {
    this->tonearmMotors.step(direction);
  }

  this->releaseCurrentFromMotors();
}

// This is used to release current from both motors so they aren't drawing power when not in use.
void TonearmMovementController::releaseCurrentFromMotors() {
    this->tonearmMotors.releaseMotorCurrent();
    digitalWrite(this->motorSelectPin, LOW);
}

void TonearmMovementController::setClutchPosition(HorizontalClutchPosition position) {
    this->horizontalClutch.immediateStart(position);

    if(position == HorizontalClutchPosition::Disengage) {
      delay(this->clutchEngagementMs);
    }
    else {
      // Give the clutch additional time to engage because it may not always land in the same spot when disengaging for x ms
      delay(this->clutchEngagementMs + (this->clutchEngagementMs / 2));
    }
    
    this->horizontalClutch.immediateStop();
}

void TonearmMovementController::setClutchEngagementMs(uint16_t ms) {
  this->clutchEngagementMs = ms;
}

void TonearmMovementController::setTopMotorSpeed(uint8_t topSpeed) {
  this->topMotorSpeed = topSpeed;
}

void TonearmMovementController::setVerticalTimeout(unsigned int timeout) {
  this->verticalTimeout = timeout;
}