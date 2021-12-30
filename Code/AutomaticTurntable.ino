#include <Stepper.h>
#include <Multiplexer.h>
#include "headers/AutomaticTurntable.h"
#include "enums/MultiplexerInput.h"
#include "enums/ErrorCode.h"
#include "enums/MotorAxis.h"
#include "enums/TonearmMovementDirection.h"
#include "enums/AutoManualSwitchPosition.h"

// Used for 7-segment display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define SERIAL_SPEED 115200

#define STEPS_PER_REVOLUTION 2048

// These motor pins channel into a quad 2-channel demultiplexer, so that either the vertical or horizontal motors receive
// the voltages. Only one of these motors will ever be moving at once
#define MOTOR_PIN1 7
#define MOTOR_PIN2 8
#define MOTOR_PIN3 9
#define MOTOR_PIN4 10
Stepper TonearmMotor = Stepper(STEPS_PER_REVOLUTION, MOTOR_PIN4, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN1);

// This is the pin used to select which motor we are moving, using the demultiplexer.
#define MOTOR_AXIS_SELECTOR 11

// This is used to engage the horizontal gears for movement. This is needed so that the gears aren't engaged
// when a record is playing or any other times, otherwise the record would not be able to move the tonearm
// very well...
#define HORIZONTAL_GEARING_SOLENOID A7

// Indicator lights so we can tell what the turntable is currently doing.
#define MOVEMENT_STATUS_LED 12
#define PAUSE_STATUS_LED 13

// These are the selector pins for the multiplexer that is used to handle all inputs.
#define MUX_OUTPUT 6
#define MUX_SELECTOR_A 2
#define MUX_SELECTOR_B 3
#define MUX_SELECTOR_C 4
#define MUX_SELECTOR_D 5
Multiplexer mux = Multiplexer(MUX_OUTPUT, MUX_SELECTOR_A, MUX_SELECTOR_B, MUX_SELECTOR_C, MUX_SELECTOR_D);

// The motors used in this project are 28BYJ-48 stepper motors, which I've found to cap at 11 RPM 
// before becoming too unreliable. 8 or 9 I've found to be a good balance for speed and reliability at 5v DC.
#define DEFAULT_MOVEMENT_RPM 8
uint8_t tonearmMovementRpm = DEFAULT_MOVEMENT_RPM;

// These are timeouts used for error checking, so the hardware doesn't damage itself.
// Essentially, if the steps exceed this number and the motor has not yet reached its
// destination, an error has occurred.
#define MOVEMENT_TIMEOUT_STEPS 1000

// Step counts used for error checking. We will have an idea of how many steps a movement should take,
// so here we are keeping track of those so we know it doesn't exceed the limits defined above.
unsigned int movementStepCount;

// The direction the tonearm is currently moving.
TonearmMovementDirection movementDirection;

// Whether the current sensor that the tonearm is moving toward is HIGH or LOW.
bool currentSensorStatus = false;

// Whether the last movement succeeded or failed (and how).
ErrorCode currentMovementStatus;

// All of these fields are used to calculate the speed that the turntable is spinning.
Adafruit_7segment speedDisplay = Adafruit_7segment();
double currSpeed = 0.0;
double lastSpeed = 0.0;
unsigned long currMillis = millis();
unsigned long lastMillis = millis();
bool currSpeedSensorStatus;
bool lastSpeedSensorStatus;

// Calibration potentiometers are used to move the tonearm a few steps past the sensor, so that the user has an easy way
// to fine-tune where exactly it should be set down (or picked up)
#define CALIBRATION_POT_7IN A0
#define CALIBRATION_POT_10IN A1
#define CALIBRATION_POT_12IN A2
#define CALIBRATION_POT_HOME A3
#define CALIBRATION_POT_PICKUP A6

int currentHorizontalCalibration = 0;
MultiplexerInput currentPlaySensor;

void setup() {
  Serial.begin(SERIAL_SPEED);

  pinMode(MOTOR_AXIS_SELECTOR, OUTPUT);
  pinMode(HORIZONTAL_GEARING_SOLENOID, OUTPUT);
  pinMode(MOVEMENT_STATUS_LED, OUTPUT);
  pinMode(PAUSE_STATUS_LED, OUTPUT);
  pinMode(CALIBRATION_POT_7IN, INPUT);
  pinMode(CALIBRATION_POT_10IN, INPUT);
  pinMode(CALIBRATION_POT_12IN, INPUT);
  pinMode(CALIBRATION_POT_HOME, INPUT);
  pinMode(CALIBRATION_POT_PICKUP, INPUT);

  mux.setDelayMicroseconds(10);

  speedDisplay.begin(0x70);
  speedDisplay.print("JHI");
  speedDisplay.writeDisplay();

  // Began startup light show
  delay(100);
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);
  delay(100);
  digitalWrite(PAUSE_STATUS_LED, HIGH);
  delay(100);
  digitalWrite(PAUSE_STATUS_LED, LOW);
  delay(100);
  digitalWrite(MOVEMENT_STATUS_LED, LOW);
  // End startup light show

  lastSpeedSensorStatus = mux.readDigitalValue(MultiplexerInput::TurntableSpeedSensor);
  currSpeedSensorStatus = lastSpeedSensorStatus;

  currentPlaySensor = getActivePlaySensor();

  // If the turntable is turned on to "automatic," then home the whole tonearm if it is not already home.
  if(mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) == AutoManualSwitchPosition::Automatic && 
    !mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) {
    currentMovementStatus = homeRoutine();

    if(currentMovementStatus != ErrorCode::Success) {
      setErrorState(currentMovementStatus);
    }
  }

  // Otherwise, we only want to home the vertical axis if it is not already homed, which will drop the tonearm in its current location.
  else if(!mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
    if(!moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS))
      setErrorState(ErrorCode::VerticalHomeError);
  }
}

// This sits and waits for any of the command buttons to be pressed.
// As soon as a button is pressed, the corresponding command routine is executed.
void loop() {
  if(mux.readDigitalValue(MultiplexerInput::PauseButton)) {
    currentMovementStatus = pauseAndWaitUntilUnpaused();

    if(currentMovementStatus != ErrorCode::Success) {
      setErrorState(currentMovementStatus);
    }
  }
  
  if(mux.readDigitalValue(MultiplexerInput::PlayHomeButton) ||
   (!mux.readDigitalValue(MultiplexerInput::HorizontalPickupOpticalSensor) && 
     mux.readDigitalValue(MultiplexerInput::AutoManualSwitch))) {

    // If the tonearm is past the location of the home sensor, then this button will home it. Otherwise, it will execute
    // the play routine.
    if(mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) 
      currentMovementStatus = playRoutine();
    else 
      currentMovementStatus = homeRoutine();

    if(currentMovementStatus != ErrorCode::Success) {
      setErrorState(currentMovementStatus);
    }
  }
  calculateTurntableSpeed();
}

ErrorCode homeRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);

  currentHorizontalCalibration = -100; //getHorizontalSensorCalibration(MultiplexerInput::HorizontalHomeOpticalSensor);

  if(!moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS))
    return ErrorCode::VerticalPickupError;

  if(!moveTonearmHorizontally(MultiplexerInput::HorizontalHomeOpticalSensor, MOVEMENT_TIMEOUT_STEPS, currentHorizontalCalibration, 7))
    return ErrorCode::HorizontalHomeError;

  if(!moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS))
    return ErrorCode::VerticalHomeError;

  horizontalRelativeMove(200);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);

  return ErrorCode::Success;
}

// This is the pause routine that will lift up the tonearm from the record until the user "unpauses" by pressing the
// pause button again
ErrorCode pauseAndWaitUntilUnpaused() {
  digitalWrite(PAUSE_STATUS_LED, HIGH);

  // Only "pause" if the tonearm is not already paused.
  if(!mux.readDigitalValue(MultiplexerInput::VerticalUpperLimit)) {
    if(!moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS))
      return ErrorCode::VerticalPickupError;

    // Wait for the user to unpause.
    while(!mux.readDigitalValue(MultiplexerInput::PauseButton)) {
      delay(1);
    }
  }

  // If the tonearm is hovering over home position, then just go down at default speed
  if(mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) {
    tonearmMovementRpm = DEFAULT_MOVEMENT_RPM;
  }

  // Otherwise, set it down carefully
  else tonearmMovementRpm = 3;

  if(!moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, tonearmMovementRpm))
    return ErrorCode::VerticalHomeError;

  digitalWrite(PAUSE_STATUS_LED, LOW);

  return ErrorCode::Success;
}

ErrorCode playRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);

  currentPlaySensor = getActivePlaySensor();
  currentHorizontalCalibration = 0; //getHorizontalSensorCalibration(currentPlaySensor);

  if(!moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS))
    return ErrorCode::VerticalPickupError;

  if(!moveTonearmHorizontally(currentPlaySensor, MOVEMENT_TIMEOUT_STEPS, currentHorizontalCalibration, 4))
    return ErrorCode::PlayError;

  if(!moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, 4))
    return ErrorCode::VerticalHomeError;

  horizontalRelativeMove(100, 8);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);
  
  return ErrorCode::Success;
}

// Moves the tonearm to a specified destination.
bool moveTonearmHorizontally(MultiplexerInput destinationSensor, unsigned int timeout, int calibration, uint8_t speed) {
    // Only make the movement if it is a valid destination sensor
    if(!(destinationSensor == MultiplexerInput::HorizontalHomeOpticalSensor || 
        destinationSensor == MultiplexerInput::HorizontalPlay7InchOpticalSensor || 
        destinationSensor == MultiplexerInput::HorizontalPlay10InchOpticalSensor || 
        destinationSensor == MultiplexerInput::HorizontalPlay12InchOpticalSensor
    )) {
      Serial.println("Programming error: Invalid sensor sent to moveTonearmHorizontally. This function only expects a horizontal sensor.");
      return false;
    }

    TonearmMotor.setSpeed(speed);
    movementStepCount = 0;
    digitalWrite(MOTOR_AXIS_SELECTOR, MotorAxis::Horizontal);

    currentSensorStatus = mux.readDigitalValue(destinationSensor); 
    movementDirection = currentSensorStatus ? TonearmMovementDirection::Clockwise : TonearmMovementDirection::Counterclockwise;

    digitalWrite(HORIZONTAL_GEARING_SOLENOID, HIGH);

    // Keep moving until the sensor is the opposite of what it started at
    while(mux.readDigitalValue(destinationSensor) == currentSensorStatus) {
      TonearmMotor.step(movementDirection);

      // If the sensor isn't hit in the expected time, the movement failed.
      if(movementStepCount++ >= timeout) {
        horizontalRelativeMove(200); // Move clockwise to unlock horizontal gears
        digitalWrite(HORIZONTAL_GEARING_SOLENOID, LOW);
        return false;
      }
    }

    horizontalRelativeMove(calibration, speed); // Move tonearm additional steps to account for calibration set by rear potentiometers
    delay(250); // Verify movement has truly ceased before releasing the solenoid
    digitalWrite(HORIZONTAL_GEARING_SOLENOID, LOW);

    releaseCurrentFromMotors();

    return true;
}

bool moveTonearmVertically(MultiplexerInput destinationSensor, unsigned int timeout, uint8_t speed = DEFAULT_MOVEMENT_RPM) {

  // Only perform the operation if the tonearm is not already at its destination.
  if(!mux.readDigitalValue(destinationSensor)) {
    if(destinationSensor == MultiplexerInput::VerticalLowerLimit) {
      movementDirection = TonearmMovementDirection::Negative; // Must move in the negative direction (down) to reach the lower limit.
    }
    else if(destinationSensor == MultiplexerInput::VerticalUpperLimit) {
      movementDirection = TonearmMovementDirection::Positive; // Must move in the positive direction (up) to reach the upper limit.
    }
    else {
      Serial.println("Programming error: Invalid Multiplexer Input passed to moveTonearmVertically.");
      return false;
    }

    TonearmMotor.setSpeed(speed);
    movementStepCount = 0;
    digitalWrite(MOTOR_AXIS_SELECTOR, MotorAxis::Vertical);

    // Move the vertical axis until it reaches its destination
    while(!mux.readDigitalValue(destinationSensor)) {
      TonearmMotor.step(movementDirection);

      // Timeout so that if the motor gets stuck, it does not get damaged.
      if(movementStepCount++ >= timeout) {
        return false;
      }
    }

    releaseCurrentFromMotors();
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

// Move the tonearm horizontally by the given step count. A positive value will move the tonearm clockwise, and a negative value
// counter-clockwise. This is a blind movement, meaning there is no check at the end that the tonearm successfully moved all
// steps. This is only intended to be used for calibration offsets, and moving a few steps to unlock gears.
void horizontalRelativeMove(int steps, uint8_t speed = DEFAULT_MOVEMENT_RPM) {
  movementStepCount = 0;

  digitalWrite(MOTOR_AXIS_SELECTOR, MotorAxis::Horizontal);
  TonearmMotor.setSpeed(speed);

  movementDirection = (steps > 0) ? TonearmMovementDirection::Clockwise : TonearmMovementDirection::Counterclockwise;
  steps = abs(steps);

  while(movementStepCount++ < steps) {
    TonearmMotor.step(movementDirection);
  }

  releaseCurrentFromMotors();
}

// This uses RecordSizeSelector1 and RecordSizeSelector2 to determine the record size the user
// currently has selected.
MultiplexerInput getActivePlaySensor() {

  // If only RecordSizeSelector1 is HIGH, we are using the 7" sensor
  if(mux.readDigitalValue(MultiplexerInput::RecordSizeSelector1))
    return MultiplexerInput::HorizontalPlay7InchOpticalSensor;

  // If only RecordSizeSelector2 is HIGH, we are using the 12" sensor
  else if (mux.readDigitalValue(MultiplexerInput::RecordSizeSelector2))
    return MultiplexerInput::HorizontalPlay12InchOpticalSensor;

  // If NEITHER RecordSizeSelectors are HIGH, we are using the 10" sensor
  return MultiplexerInput::HorizontalPlay10InchOpticalSensor;
}

// Returns the calibration step offset for the given sensor.
// The returned value will be the number of steps (clockwise or counterclockwise) that the horizontal motor should move.
// Pickup calibration return value is expressed in number of seconds that should be waited before the homing routine is
// executed at the end of a record.
unsigned int getHorizontalSensorCalibration(MultiplexerInput sensor) {
  if(sensor == MultiplexerInput::HorizontalPlay7InchOpticalSensor)
    return analogRead(CALIBRATION_POT_7IN);

  else if(sensor == MultiplexerInput::HorizontalPlay10InchOpticalSensor)
    return analogRead(CALIBRATION_POT_10IN);

  else if(sensor == MultiplexerInput::HorizontalPlay12InchOpticalSensor)
    return analogRead(CALIBRATION_POT_12IN);

  else if(sensor == MultiplexerInput::HorizontalHomeOpticalSensor)
    return analogRead(CALIBRATION_POT_HOME);

  else if(sensor == MultiplexerInput::HorizontalPickupOpticalSensor)
    return analogRead(CALIBRATION_POT_PICKUP);

  else
    return 0;
}

// This function will calculate the speed of the turntable 8 times per rotation.
void calculateTurntableSpeed() {
  currSpeedSensorStatus = mux.readDigitalValue(MultiplexerInput::TurntableSpeedSensor);
  
  if(currSpeedSensorStatus != lastSpeedSensorStatus) {
    currMillis = millis();
    currSpeed = 60000 / (double)((currMillis - lastMillis) << 3);
    
    // Only re-write the display if the display will change
    if(abs(lastSpeed - currSpeed) > 0.009) {
      speedDisplay.print(currSpeed, 2);
      speedDisplay.writeDisplay();
    }

    lastSpeed = currSpeed;
    lastMillis = currMillis;
  }

  // If 1 second elapses without a sensor change, we can assume that the turntable has stopped.
  else if(millis() - currMillis > 1000 && lastSpeed > 0.0) {
    speedDisplay.print(0.0);
    speedDisplay.writeDisplay();
    lastSpeed = 0.0;
  }

  lastSpeedSensorStatus = currSpeedSensorStatus;
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
      break;

    // Fast-blink pause LED.
    case ErrorCode::VerticalPickupError:
      blinkLed(PAUSE_STATUS_LED, 150);
      break;

    // Slow-blink movement LED.
    case ErrorCode::HorizontalHomeError:
      blinkLed(MOVEMENT_STATUS_LED, 1000);
      break;

    // Fast-blink movement LED.
    case ErrorCode::PlayError:
      blinkLed(MOVEMENT_STATUS_LED, 150);
      break;
  }
}

// Blinks a specific LED indefinitely.
// For use only with error codes.
void blinkLed(int led, int interval) {

  // Only blink until a button is pressed.
  // TODO: Make this not require holding the button until the interval has elapsed.
  while(!mux.readDigitalValue(MultiplexerInput::PauseButton) && !mux.readDigitalValue(MultiplexerInput::PlayHomeButton)) {
    digitalWrite(led, LOW);
    delay(interval);
    digitalWrite(led, HIGH);
    delay(interval);
  }

  digitalWrite(led, LOW);
}