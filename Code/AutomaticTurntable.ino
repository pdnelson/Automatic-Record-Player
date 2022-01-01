#include <Stepper.h>
#include <Multiplexer.h>
#include "headers/AutomaticTurntable.h"
#include "enums/MultiplexerInput.h"
#include "enums/ErrorCode.h"
#include "enums/AutoManualSwitchPosition.h"
#include "TonearmMovementController.h"

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

TonearmMovementController tonearmController = TonearmMovementController(
  mux,
  MOTOR_PIN1,
  MOTOR_PIN2,
  MOTOR_PIN3,
  MOTOR_PIN4,
  TonearmMotor,
  MOTOR_AXIS_SELECTOR,
  HORIZONTAL_GEARING_SOLENOID,
  MultiplexerInput::VerticalLowerLimit,
  MultiplexerInput::VerticalUpperLimit
);

// The motors used in this project are 28BYJ-48 stepper motors, which I've found to cap at 11 RPM 
// before becoming too unreliable. 8 or 9 I've found to be a good balance for speed and reliability at 5v DC.
#define DEFAULT_MOVEMENT_RPM 8

// These are timeouts used for error checking, so the hardware doesn't damage itself.
// Essentially, if the steps exceed this number and the motor has not yet reached its
// destination, an error has occurred.
#define MOVEMENT_TIMEOUT_STEPS 1000

// All of these fields are used to calculate the speed that the turntable is spinning.
Adafruit_7segment speedDisplay = Adafruit_7segment();
double currSpeed = 0.0;
double lastSpeed = 0.0;
unsigned long currMillis = millis();
unsigned long lastMillis = millis();
bool currSpeedSensorStatus;
bool lastSpeedSensorStatus;

// This is used to tell us if the user viewed the sensor calibration from the 7-segment display or 
// the turntable speed
bool lastDisplayOption;
bool currDisplayOption;

// Calibration potentiometers are used to move the tonearm a few steps past the sensor, so that the user has an easy way
// to fine-tune where exactly it should be set down (or picked up)
#define CALIBRATION_POT_7IN A0
#define CALIBRATION_POT_10IN A1
#define CALIBRATION_POT_12IN A2

void setup() {
  Serial.begin(SERIAL_SPEED);

  pinMode(MOVEMENT_STATUS_LED, OUTPUT);
  pinMode(PAUSE_STATUS_LED, OUTPUT);

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

  lastDisplayOption = mux.readDigitalValue(MultiplexerInput::DisplayCalibrationValue);
  currDisplayOption = lastDisplayOption;

  ErrorCode currentMovementStatus = ErrorCode::None;

  // If the turntable is turned on to "automatic," then home the whole tonearm if it is not already home.
  if(mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) == AutoManualSwitchPosition::Automatic && 
    !mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) {
    ErrorCode currentMovementStatus = homeRoutine();
  }

  // Otherwise, we only want to home the vertical axis if it is not already homed, which will drop the tonearm in its current location.
  else if(!mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
      currentMovementStatus = pauseOrUnpause();
  }

  if(currentMovementStatus != ErrorCode::Success && currentMovementStatus != ErrorCode::None) {
    setErrorState(currentMovementStatus);
  }
}

void loop() {
  // We always want to make sure the solenoid is not being powered when a command is not executing. There are some bugs that are mostly out of my control
  // that may cause the solenoid to become HIGH, for example, unplugging the USB from the Arduino can sometimes alter the state of the software.
  digitalWrite(HORIZONTAL_GEARING_SOLENOID, LOW);

  ErrorCode currentMovementStatus = ErrorCode::None;

  if(mux.readDigitalValue(MultiplexerInput::PauseButton)) {
    currentMovementStatus = pauseOrUnpause();
  }
  
  else if(mux.readDigitalValue(MultiplexerInput::PlayHomeButton) ||
   (!mux.readDigitalValue(MultiplexerInput::HorizontalPickupOpticalSensor) && 
     mux.readDigitalValue(MultiplexerInput::AutoManualSwitch))) {

    // If the tonearm is past the location of the home sensor, then this button will home it. Otherwise, it will execute
    // the play routine.
    if(mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) 
      currentMovementStatus = playRoutine();
    else 
      currentMovementStatus = homeRoutine();
  }

  if(currentMovementStatus != ErrorCode::Success && currentMovementStatus != ErrorCode::None) {
    setErrorState(currentMovementStatus);
  }

  // If the calibration button is being pressed, display the current value of the active potentiometer
  if(currDisplayOption = mux.readDigitalValue(MultiplexerInput::DisplayCalibrationValue)) {
    
    // Must cast this to double because, for some reason, since every other value I've been sending to the display
    // has been doubles, it only lets me write double values. Is it a bug on my end? I'm not sure, but this works.
    speedDisplay.print((double)getActiveSensorCalibration());
    speedDisplay.writeDisplay();
  }

  // Otherwise, display the turntable speed
  else {
    calculateTurntableSpeedAndPrintToDisplay();
  }
  lastDisplayOption = currDisplayOption;
}

ErrorCode homeRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);
  digitalWrite(PAUSE_STATUS_LED, LOW);

  if(!tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM))
    return ErrorCode::VerticalPickupError;

  // -200 calibration to push the tonearm past the home sensor, into the homing mount
  if(!tonearmController.moveTonearmHorizontally(MultiplexerInput::HorizontalHomeOpticalSensor, MOVEMENT_TIMEOUT_STEPS, -200, 7))
    return ErrorCode::HorizontalHomeError;

  if(!tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM))
    return ErrorCode::VerticalHomeError;

  tonearmController.horizontalRelativeMove(200, DEFAULT_MOVEMENT_RPM);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);

  return ErrorCode::Success;
}

// This is the pause routine that will lift up the tonearm from the record until the user "unpauses" by pressing the
// pause button again
ErrorCode pauseOrUnpause() {
  digitalWrite(MOVEMENT_STATUS_LED, LOW);
  digitalWrite(PAUSE_STATUS_LED, HIGH);

  // If the vertical lower limit is pressed (i.e., the tonearm is vertically homed), then move it up
  if(mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
    if(!tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM))
      return ErrorCode::VerticalPickupError;
  }

  // Otherwise, just move it down and then shut off the LED
  else {
    uint8_t tonearmSetRpm = 0;

    // If the tonearm is hovering over home position, then just go down at default speed
    if(mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) {
      tonearmSetRpm = DEFAULT_MOVEMENT_RPM;
    }

    // Otherwise, set it down carefully
    else tonearmSetRpm = 3;

    if(!tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, tonearmSetRpm))
      return ErrorCode::VerticalHomeError;

    digitalWrite(PAUSE_STATUS_LED, LOW);
  }

  return ErrorCode::Success;
}

ErrorCode playRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);
  digitalWrite(PAUSE_STATUS_LED, LOW);

  if(!tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM))
    return ErrorCode::VerticalPickupError;

  if(!tonearmController.moveTonearmHorizontally(getActivePlaySensor(), MOVEMENT_TIMEOUT_STEPS, getActiveSensorCalibration(), 4))
    return ErrorCode::PlayError;

  if(!tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, 4))
    return ErrorCode::VerticalHomeError;

  tonearmController.horizontalRelativeMove(100, DEFAULT_MOVEMENT_RPM);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);
  
  return ErrorCode::Success;
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
unsigned int getActiveSensorCalibration() {
  int calibration = 0;

    // If only RecordSizeSelector1 is HIGH (or BOTH RecordSelector1 and 2 are HIGH), we are using the 7" sensor
  if(mux.readDigitalValue(MultiplexerInput::RecordSizeSelector1))
    calibration = analogRead(CALIBRATION_POT_7IN); 

  // If only RecordSizeSelector2 is HIGH, we are using the 12" sensor
  else if (mux.readDigitalValue(MultiplexerInput::RecordSizeSelector2))
    calibration = analogRead(CALIBRATION_POT_12IN);

  // If NEITHER RecordSizeSelectors are HIGH, we are using the 10" sensor
  else calibration = analogRead(CALIBRATION_POT_10IN);

  // I only have 10k pots, which ranges between 615-1023, and I only want to allow
  // values between 0 and 50
  calibration = (calibration - 615) * 50 / 408;

  if(calibration < 0) 
    calibration = 0;

  else if(calibration > 50) 
    calibration = 50;

  return calibration;
}

// This function will calculate the speed of the turntable 8 times per rotation.
void calculateTurntableSpeedAndPrintToDisplay() {
  currSpeedSensorStatus = mux.readDigitalValue(MultiplexerInput::TurntableSpeedSensor);
  
  if(currSpeedSensorStatus != lastSpeedSensorStatus) {
    currMillis = millis();
    currSpeed = 60000 / (double)((currMillis - lastMillis) << 3); // Calculate RPM 8 times per rotation
    
    // Only re-write the display if the display will change
    if(abs(lastSpeed - currSpeed) > 0.009) {
      speedDisplay.print(currSpeed, 2);
      speedDisplay.writeDisplay();
    }

    lastSpeed = currSpeed;
    lastMillis = currMillis;
  }

  // If 1 second elapses without a sensor change, we can assume that the turntable has stopped.
  // Also, re-write the display as 0 if the user releases the display button (changing from displaying calibration steps back to RPM)
  else if((millis() - currMillis > 1000 && lastSpeed > 0.0) || currDisplayOption != lastDisplayOption) {
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
