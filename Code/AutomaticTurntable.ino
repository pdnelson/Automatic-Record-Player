#include <Stepper.h>
#include <Multiplexer.h>
#include "headers/AutomaticTurntable.h"
#include "enums/MultiplexerInput.h"
#include "enums/MovementResult.h"
#include "enums/AutoManualSwitchPosition.h"
#include "TonearmMovementController.h"

// Used for 7-segment display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

//#define SERIAL_SPEED 115200

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

// The 7-segment display is used for the following:
// - Displaying the speed that the turntable is spinning.
// - If the user is pressing the calibration button, displaying the current sensor's calibration value.
// - Error codes if a movement fails.
Adafruit_7segment sevSeg = Adafruit_7segment();

// These fields are so we aren't writing to the 7-segment display so often
double lastSevSegValue = 0.0;
double lastMillisSevSeg = millis();

// All of these fields are used to calculate the speed that the turntable is spinning.
#define SPEED_SENSOR 1
volatile unsigned long currMillisSpeed = millis();
volatile unsigned long lastMillisSpeed = currMillisSpeed;
volatile double currSpeed;

// Calibration potentiometers are used to move the tonearm a few steps past the sensor, so that the user has an easy way
// to fine-tune where exactly it should be set down (or picked up)
#define CALIBRATION_POT_7IN A0
#define CALIBRATION_POT_10IN A1
#define CALIBRATION_POT_12IN A2

void setup() {
  //Serial.begin(SERIAL_SPEED);

  pinMode(MOVEMENT_STATUS_LED, OUTPUT);
  pinMode(PAUSE_STATUS_LED, OUTPUT);

  pinMode(SPEED_SENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(SPEED_SENSOR), calculateTurntableSpeed, RISING);

  mux.setDelayMicroseconds(10);

  sevSeg.begin(0x70);
  sevSeg.print("JHI");
  sevSeg.writeDisplay();

  // Begin startup light show
  delay(100);
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);
  delay(100);
  digitalWrite(PAUSE_STATUS_LED, HIGH);
  delay(100);
  digitalWrite(PAUSE_STATUS_LED, LOW);
  delay(100);
  digitalWrite(MOVEMENT_STATUS_LED, LOW);
  // End startup light show

  MovementResult currentMovementStatus = MovementResult::None;

  // If the turntable is turned on to "automatic," then home the whole tonearm if it is not already home.
  if(mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) == AutoManualSwitchPosition::Automatic && 
    !mux.readDigitalValue(MultiplexerInput::HorizontalHomeOpticalSensor)) {
    MovementResult currentMovementStatus = homeRoutine();
  }

  // Otherwise, we only want to home the vertical axis if it is not already homed, which will drop the tonearm in its current location.
  else if(!mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
      currentMovementStatus = pauseOrUnpause();
  }

  if(currentMovementStatus != MovementResult::Success && currentMovementStatus != MovementResult::None) {
    setErrorState(currentMovementStatus);
  }
}

void loop() {
  // We always want to make sure the solenoid is not being powered when a command is not executing. There are some bugs that are mostly out of my control
  // that may cause the solenoid to become HIGH, for example, unplugging the USB from the Arduino can sometimes alter the state of the software.
  digitalWrite(HORIZONTAL_GEARING_SOLENOID, LOW);

  monitorCommandButtons();
  updateSevenSegmentDisplay();
}

// When a command button is pressed (i.e. Home/Play, or Pause/Unpause), then its respective command will be executed.
void monitorCommandButtons() {
  MovementResult currentMovementStatus = MovementResult::None;

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

  if(currentMovementStatus != MovementResult::Success && currentMovementStatus != MovementResult::None) {
    setErrorState(currentMovementStatus);
  }
}

void updateSevenSegmentDisplay() {

  double currSevSegValue = 0.0;

  // If the calibration button is being pressed, display the current value of the active potentiometer
  if(mux.readDigitalValue(MultiplexerInput::DisplayCalibrationValue)) {
    currSevSegValue = getActiveSensorCalibration();
  }

  // Otherwise, display the turntable speed
  else {

    // If 3 seconds elapse without a speed sensor interrupt, we can assume that the turntable has stopped.
    if(millis() - currMillisSpeed > 3000 && currSpeed > 0.0) {
      currSevSegValue = 0.0;
    }
    else currSevSegValue = currSpeed;
  }

  // Only re-write the display if the number will be different
  if(currSevSegValue != lastSevSegValue) {
    sevSeg.print(currSevSegValue);
    sevSeg.writeDisplay();
  }

  lastSevSegValue = currSevSegValue;
}

MovementResult playRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);
  digitalWrite(PAUSE_STATUS_LED, LOW);

  MovementResult result = MovementResult::None;

  int calibration = getActiveSensorCalibration();

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  result = tonearmController.moveTonearmHorizontally(getActivePlaySensor(), MOVEMENT_TIMEOUT_STEPS, calibration, 4);
  if(result != MovementResult::Success) return result;

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, 4);
  if(result != MovementResult::Success) return result;

  tonearmController.horizontalRelativeMove(100, DEFAULT_MOVEMENT_RPM);

  digitalWrite(MOVEMENT_STATUS_LED, LOW);
  
  return result;
}

MovementResult homeRoutine() {
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);
  digitalWrite(PAUSE_STATUS_LED, LOW);

  MovementResult result = MovementResult::None;

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  // -200 calibration to push the tonearm past the home sensor, into the homing mount
  result = tonearmController.moveTonearmHorizontally(MultiplexerInput::HorizontalHomeOpticalSensor, MOVEMENT_TIMEOUT_STEPS, -200, 7);
  if(result != MovementResult::Success) return result;

  tonearmController.horizontalRelativeMove(35, DEFAULT_MOVEMENT_RPM); // This is so the tonearm doesn't get "stuck" on the homing mount that it just rammed into

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  tonearmController.horizontalRelativeMove(200, DEFAULT_MOVEMENT_RPM); // Disengage the gear from the tonearm so it doesn't get stuck

  digitalWrite(MOVEMENT_STATUS_LED, LOW);

  return result;
}

MovementResult pauseOrUnpause() {
  digitalWrite(MOVEMENT_STATUS_LED, LOW);
  digitalWrite(PAUSE_STATUS_LED, HIGH);

  MovementResult result = MovementResult::None;

  // If the vertical lower limit is pressed (i.e., the tonearm is vertically homed), then move it up
  if(mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
    result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
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

    result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, MOVEMENT_TIMEOUT_STEPS, tonearmSetRpm);

    digitalWrite(PAUSE_STATUS_LED, LOW);
  }

  return result;
}

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
  // values between 0 and 50.
  calibration = (calibration - 615) * 50 / 408;

  if(calibration < 0) 
    calibration = 0;

  else if(calibration > 50) 
    calibration = 50;

  return calibration;
}

// This function is attached to an interrupt, and therefore, is not called in the main loop
void calculateTurntableSpeed() {
  currMillisSpeed = millis();
  currSpeed = 60000 / (double)(currMillisSpeed - lastMillisSpeed);
  lastMillisSpeed = currMillisSpeed;
}

void setErrorState(MovementResult movementResult) {
  digitalWrite(PAUSE_STATUS_LED, HIGH);
  digitalWrite(MOVEMENT_STATUS_LED, HIGH);

  sevSeg.clear();
  sevSeg.writeDigitNum(0, movementResult, false);
  sevSeg.writeDisplay();

  // Wait for the user to press the Play/Home or Pause/Unpause buttons to break out of the error state
  while(!mux.readDigitalValue(MultiplexerInput::PlayHomeButton) && !mux.readDigitalValue(MultiplexerInput::PauseButton)) { delay(1); }

  sevSeg.clear();
  sevSeg.writeDisplay();
}