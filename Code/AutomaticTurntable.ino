#include <Stepper.h>
#include <Multiplexer.h>
#include <DcMotor.h>
#include "proto/AutomaticTurntable.h"
#include "proto/Constants.h"
#include "enums/ArduinoPin.h"
#include "enums/MultiplexerInput.h"
#include "enums/MovementResult.h"
#include "enums/AutoManualSwitchPosition.h"
#include "enums/RecordSize.h"
#include "TonearmMovementController.h"

// Used for 7-segment display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// Used for calibration storage
#include <EEPROM.h>

// Though this is one stepper motor declaration, this object actually handles both the vertical and horizontal
// stepper motors. Which one is currently active is determined by the ArduinoPin::MotorAxisSelector.
Stepper tonearmMotor = Stepper(
  STEPS_PER_REVOLUTION, 
  ArduinoPin::StepperPin4,
  ArduinoPin::StepperPin2, 
  ArduinoPin::StepperPin3, 
  ArduinoPin::StepperPin1
);

// The multiplexer monitors most input values that we read to determine the statuses of various sensors.
Multiplexer mux = Multiplexer(
  ArduinoPin::MuxOutput, 
  ArduinoPin::MuxSelectorA, 
  ArduinoPin::MuxSelectorB, 
  ArduinoPin::MuxSelectorC, 
  ArduinoPin::MuxSelectorD
);

// The tonearmClutch allows us to engage or disengage the horizontal gearing to either allow for automatic
// movement (engaged), or manual movement (disengaged).
DcMotor horizontalClutch = DcMotor(
  ArduinoPin::HorizontalClutchMotorDir1,
  ArduinoPin::HorizontalClutchMotorDir2
);

// The tonearmController is in charge of automatically moving the tonearm vertically or horizontally.
TonearmMovementController tonearmController = TonearmMovementController(
  mux,
  ArduinoPin::StepperPin1,
  ArduinoPin::StepperPin2,
  ArduinoPin::StepperPin3,
  ArduinoPin::StepperPin4,
  tonearmMotor,
  ArduinoPin::MotorAxisSelector,
  horizontalClutch,
  MultiplexerInput::VerticalLowerLimit,
  MultiplexerInput::VerticalUpperLimit
);

// The 7-segment display is used for the following:
// - Displaying the speed that the turntable is spinning.
// - If the user is pressing the calibration button, displaying the current sensor's calibration value.
// - Error codes if a movement fails.
Adafruit_7segment sevSeg = Adafruit_7segment();

// These are calibration values set by the user to tell the tonearm how many steps to go past the "home" reference optical
// sensor.
uint16_t calibration7Inch = 0;
uint16_t calibration10Inch = 0;
uint16_t calibration12Inch = 0;

// The last value that was written to the 7-segment display. This is used so that we don't write the same value to it.
double lastSevSegValue = 0.0;

// All of these fields are used to calculate the speed that the turntable is spinning.
volatile unsigned long currMillisSpeed = millis();
volatile unsigned long lastMillisSpeed = currMillisSpeed;
volatile double currSpeed;

void setup() { //Serial.begin(SERIAL_SPEED);
  // Set pins
  pinMode(ArduinoPin::MovementStatusLed, OUTPUT);
  pinMode(ArduinoPin::PauseStatusLed, OUTPUT);
  pinMode(ArduinoPin::SpeedSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(ArduinoPin::SpeedSensor), calculateTurntableSpeed, RISING);

  // Set calibration values
  tonearmController.setClutchEngagementMs(CLUTCH_ENGAGEMENT_MS);
  mux.setDelayMicroseconds(MULTIPLEXER_DELAY_MICROS);

  // Start up seven-segment display
  sevSeg.begin(SEV_SEG_START_ADDRESS);
  sevSeg.print(0.0);
  sevSeg.writeDisplay();

  // Load calibration values from the EEPROM.
  // If the calibration button is being held, default values will be set instead of loading from the EEPROM.
  loadCalibrationEEPROMValues(mux.readDigitalValue(MultiplexerInput::DisplayCalibrationValue));

  // Engage the horizontal clutch. This will be setting it to the "starting" point for where we know it is engaged
  horizontalClutch.immediateStart(HorizontalClutchPosition::Engage);

  // Begin startup light show
  delay(100);
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
  delay(100);
  digitalWrite(ArduinoPin::PauseStatusLed, HIGH);
  delay(100);
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);
  delay(100);
  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  // End startup light show

  // Check sensors, and perform an initial movement if necessary.
  MovementResult currentMovementStatus = MovementResult::None;

  bool homeExecuted = false;

  // If the turntable is turned on to "automatic," then home the whole tonearm if it is not already home.
  if(mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) == AutoManualSwitchPosition::Automatic && 
    mux.readDigitalValue(MultiplexerInput::HorizontalHomeOrPlayOpticalSensor)) {
    MovementResult currentMovementStatus = homeRoutine();
    homeExecuted = true;
  }

  // Otherwise, we only want to home the vertical axis if it is not already homed, which will drop the tonearm in its current location.
  else if(!mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
      currentMovementStatus = pauseOrUnpause();
  }

  // If the movement was anything other than success/none, then it failed, and we must set the error state.
  if(currentMovementStatus != MovementResult::Success && currentMovementStatus != MovementResult::None) {
    setErrorState(currentMovementStatus);
  }

  // If the homing routine was not executed, then the horizontal clutch is still engaged, and we need to disengage it
  if(!homeExecuted) {
    delay(500); // Wait 500ms so the clutch can home the whole way before we disengage it
    tonearmController.setClutchPosition(HorizontalClutchPosition::Disengage);
  }
}

void loop() {
  monitorCommandButtons();
  monitorSevenSegmentInput();
}

// When a command button is pressed (i.e. Home/Play, or Pause/Unpause), then its respective command will be executed.
void monitorCommandButtons() {
  MovementResult currentMovementStatus = MovementResult::None;

  if(mux.readDigitalValue(MultiplexerInput::PauseButton)) {
    currentMovementStatus = pauseOrUnpause();
  }
  
  // If the play/home button is pressed, the command executes
  // The command also executes when the horizontal pickup sensor is set high at the same time as the home sensor, 
  // only if the auto/manual switch is set to automatic.
  else if(mux.readDigitalValue(MultiplexerInput::PlayHomeButton) ||
   (mux.readDigitalValue(MultiplexerInput::HorizontalPickupOpticalSensor) && 
    mux.readDigitalValue(MultiplexerInput::HorizontalHomeOrPlayOpticalSensor) && 
    mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) == AutoManualSwitchPosition::Automatic)
    ) {

    // If the tonearm is past the location of the home sensor, then this button will home it. Otherwise, it will execute
    // the play routine.
    if(!mux.readDigitalValue(MultiplexerInput::HorizontalHomeOrPlayOpticalSensor)) 
      currentMovementStatus = playRoutine();
    else 
      currentMovementStatus = homeRoutine();
  }

  // If the movement was anything other than success/none, then it failed, and we must set the error state.
  if(currentMovementStatus != MovementResult::Success && currentMovementStatus != MovementResult::None) {
    setErrorState(currentMovementStatus);
  }
}

// This will allow the 7-segment display to either display the current turntable speed from the input of the 
// speed sensor, or one of the three calibration values.
void monitorSevenSegmentInput() {

  double newValue;

  // If the calibration button is being pressed, enter "calibration" mode so the user can view or modify
  // values.
  if(mux.readDigitalValue(MultiplexerInput::DisplayCalibrationValue)) {
    calibrationSettingLoop();
  }

  // If 1.5 seconds elapse without a speed sensor interrupt, we can assume that the turntable has stopped.
  if(millis() - currMillisSpeed > 2500 && currSpeed > 0.0) {
    newValue = 0.0;
  }
  else newValue = currSpeed;

  updateSevenSegmentDisplay(newValue);
}

// This loop is what is executed while in "calibration" mode. This implementation allows the user to switch between
// all three calibrations while having the button held
void calibrationSettingLoop() {

  uint16_t calibrationDisplayValue = 0;

  // Keep track of the old values so we can decide which ones to save in the EEPROM
  uint16_t old7In = calibration7Inch;
  uint16_t old10In = calibration10Inch;
  uint16_t old12In = calibration12Inch;

  // Delay/debounce
  unsigned long lastButtonPressMsDelay = __LONG_MAX__;
  unsigned long lastButtonPressMsDebounce = __LONG_MAX__;
  unsigned long currButtonPressMs = 0;
  bool buttonDelay = true;
  bool buttonDebounce = true;
  
  // Iteration counting
  uint16_t buttonPressIterationCount = 0;
  uint16_t calibrationHoldChangeIterationCount = CALIBRATION_HOLD_CHANGE_INTERVAL;
  uint16_t calibrationHoldChangeInterval = CALIBRATION_HOLD_CHANGE_MS;
  uint16_t calibrationDebounceInterval = CALIBRATION_DEBOUNCE_MS;

  // With this button-pressing implementation, we are tracking two separate values: the delay and debounce.
  // For the delay, this is incrementing/decrementing the calibration value x number of times/second as they
  // hold the button, while the debounce value prevents too many button presses from being registered at once.
  // The delay will gradually increase the longer an increment/decrement button is held, so that further values
  // can be reached more quickly; this is tracked by the "iteration counting" variables.
  while(mux.readDigitalValue(MultiplexerInput::DisplayCalibrationValue)) {
    currButtonPressMs = millis();
    buttonDelay = currButtonPressMs - lastButtonPressMsDelay > calibrationHoldChangeInterval;
    buttonDebounce = currButtonPressMs - lastButtonPressMsDebounce > calibrationDebounceInterval;

    if(mux.readDigitalValue(MultiplexerInput::PauseButton) && calibrationDisplayValue < (CALIBRATION_VALUE_MAX - 1) && buttonDelay && buttonDebounce) {
      switch(getActiveRecordSize()) {
        case RecordSize::Rec7Inch: calibrationDisplayValue = calibration7Inch++; break;
        case RecordSize::Rec10Inch: calibrationDisplayValue = calibration10Inch++; break;
        case RecordSize::Rec12Inch: calibrationDisplayValue = calibration12Inch++;
      }
      lastButtonPressMsDelay = currButtonPressMs;
      lastButtonPressMsDebounce = currButtonPressMs;
      buttonPressIterationCount++;
    }
    else if(mux.readDigitalValue(MultiplexerInput::PlayHomeButton) && calibrationDisplayValue > 1 && buttonDelay && buttonDebounce) {
      switch(getActiveRecordSize()) {
        case RecordSize::Rec7Inch: calibrationDisplayValue = calibration7Inch--; break;
        case RecordSize::Rec10Inch: calibrationDisplayValue = calibration10Inch--; break;
        case RecordSize::Rec12Inch: calibrationDisplayValue = calibration12Inch--;
      }
      lastButtonPressMsDelay = currButtonPressMs;
      lastButtonPressMsDebounce = currButtonPressMs;
      buttonPressIterationCount++;
    }
    else if(!mux.readDigitalValue(MultiplexerInput::PauseButton) && !mux.readDigitalValue(MultiplexerInput::PlayHomeButton)) {
      buttonPressIterationCount = 0;
      calibrationHoldChangeIterationCount = CALIBRATION_HOLD_CHANGE_INTERVAL;
      calibrationHoldChangeInterval = CALIBRATION_HOLD_CHANGE_MS;
      lastButtonPressMsDelay = __LONG_MAX__;
      calibrationDebounceInterval = CALIBRATION_DEBOUNCE_MS;
      calibrationDisplayValue = getActiveSensorCalibration();
    }

    // If we have reached the destination number of iterations with a button held, then we can increase the speed at which 
    // numbers increase/decrease by.
    if(buttonPressIterationCount == calibrationHoldChangeIterationCount && calibrationHoldChangeInterval > CALIBRATION_HOLD_LOWEST_MS) {
      calibrationHoldChangeInterval = calibrationHoldChangeInterval - CALIBRATION_HOLD_DECREMENT_MS;
      buttonPressIterationCount = 0;
      calibrationDebounceInterval = 0; // Once the user holds the button so long, we know the debounce interval is no longer necessary
    }

    updateSevenSegmentDisplay((double)calibrationDisplayValue);
  }

  updateCalibrationEEPROMValues(old7In, old10In, old12In);
}

void updateSevenSegmentDisplay(double newValue) {
  // Only re-write the display if the number will be different
  if(newValue != lastSevSegValue) {
    sevSeg.print(newValue);
    sevSeg.writeDisplay();
    lastSevSegValue = newValue;
  }
}

// Move the tonearm clockwise to the play sensor
// This is a multi-movement routine, meaning that multiple tonearm movements are executed. If one of those movements fails, the
// whole routine is aborted.
MovementResult playRoutine() {
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);

  MovementResult result = MovementResult::None;

  int calibration = getActiveSensorCalibration();

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, VERTICAL_MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  result = tonearmController.moveTonearmHorizontally(MultiplexerInput::HorizontalHomeOrPlayOpticalSensor, HORIZONTAL_MOVEMENT_TIMEOUT_STEPS, calibration, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, VERTICAL_MOVEMENT_TIMEOUT_STEPS, 3);
  if(result != MovementResult::Success) return result;

  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  
  return result;
}

// Move the tonearm counterclockwise to the home sensor.
// This is a multi-movement routine, meaning that multiple tonearm movements are executed. If one of those movements fails, the
// whole routine is aborted.
MovementResult homeRoutine() {
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);

  MovementResult result = MovementResult::None;

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, VERTICAL_MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  result = tonearmController.moveTonearmHorizontally(MultiplexerInput::HorizontalHomeOrPlayOpticalSensor, HORIZONTAL_MOVEMENT_TIMEOUT_STEPS, STEPS_FROM_PLAY_SENSOR_HOME, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, VERTICAL_MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
  if(result != MovementResult::Success) return result;

  digitalWrite(ArduinoPin::MovementStatusLed, LOW);

  return result;
}

// This is the pause routine that will lift up the tonearm from the record until the user "unpauses" by pressing the
// pause button again
MovementResult pauseOrUnpause() {
  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  digitalWrite(ArduinoPin::PauseStatusLed, HIGH);

  MovementResult result = MovementResult::None;

  // If the vertical lower limit is pressed (i.e., the tonearm is vertically homed), then move it up
  if(mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
    result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalUpperLimit, VERTICAL_MOVEMENT_TIMEOUT_STEPS, DEFAULT_MOVEMENT_RPM);
  }

  // Otherwise, just move it down and then shut off the LED
  else {
    uint8_t tonearmSetRpm = 0;

    // If the tonearm is hovering over home position, then just go down at default speed
    if(!mux.readDigitalValue(MultiplexerInput::HorizontalHomeOrPlayOpticalSensor)) {
      tonearmSetRpm = DEFAULT_MOVEMENT_RPM;
    }

    // Otherwise, set it down carefully
    else tonearmSetRpm = 3;

    result = tonearmController.moveTonearmVertically(MultiplexerInput::VerticalLowerLimit, VERTICAL_MOVEMENT_TIMEOUT_STEPS, tonearmSetRpm);

    digitalWrite(ArduinoPin::PauseStatusLed, LOW);
  }

  return result;
}

// Returns the calibration step offset for the given sensor.
// The returned value will be the number of steps (clockwise or counterclockwise) that the horizontal motor should move.
uint16_t getActiveSensorCalibration() {
  switch(getActiveRecordSize()) {
    case RecordSize::Rec7Inch:
      return calibration7Inch;
    case RecordSize::Rec10Inch:
      return calibration10Inch;
    case RecordSize::Rec12Inch:
      return calibration12Inch;
  }
}

RecordSize getActiveRecordSize() {
  // If only RecordSizeSelector1 is HIGH (or BOTH RecordSelector1 and 2 are HIGH), we are using the 7" sensor
  if(mux.readDigitalValue(MultiplexerInput::RecordSizeSelector1))
    return RecordSize::Rec7Inch; 

  // If only RecordSizeSelector2 is HIGH, we are using the 12" sensor
  else if (mux.readDigitalValue(MultiplexerInput::RecordSizeSelector2))
    return RecordSize::Rec12Inch;

  // If NEITHER RecordSizeSelectors are HIGH, we are using the 10" sensor
  else return RecordSize::Rec10Inch;
}

void loadCalibrationEEPROMValues(bool loadDefaults) {
  // This mode is used so that the user can see if the 3-way switch is positioned correctly (with 7in pointing down, and 12in pointing up)
  if(loadDefaults) {
    calibration7Inch = CALIBRATION_7IN_DEFAULT;
    calibration10Inch = CALIBRATION_10IN_DEFAULT;
    calibration12Inch = CALIBRATION_12IN_DEFAULT;
  }
  else {
    // Load calibration values from EEPROM
    calibration7Inch = (EEPROM.read(CALIBRATION_7IN_EEPROM_START_ADDRESS) << 8) + EEPROM.read(CALIBRATION_7IN_EEPROM_START_ADDRESS + 1);
    calibration10Inch = (EEPROM.read(CALIBRATION_10IN_EEPROM_START_ADDRESS) << 8) + EEPROM.read(CALIBRATION_10IN_EEPROM_START_ADDRESS + 1);
    calibration12Inch = (EEPROM.read(CALIBRATION_12IN_EEPROM_START_ADDRESS) << 8) + EEPROM.read(CALIBRATION_12IN_EEPROM_START_ADDRESS + 1);

    // Validate that calibration values are within the accepted range
    if(calibration7Inch > CALIBRATION_VALUE_MAX) calibration7Inch = 0;
    if(calibration10Inch > CALIBRATION_VALUE_MAX) calibration10Inch = 0;
    if(calibration12Inch > CALIBRATION_VALUE_MAX) calibration12Inch = 0;
  }
}

// This will update all numbers that have changed
void updateCalibrationEEPROMValues(uint16_t old7In, uint16_t old10In, uint16_t old12In) {
  // We only try to write each value to the EEPROM if it has been changed.
  if(old7In != calibration7Inch) {
    EEPROM.write(CALIBRATION_7IN_EEPROM_START_ADDRESS, calibration7Inch >> 8);
    EEPROM.write(CALIBRATION_7IN_EEPROM_START_ADDRESS + 1, calibration7Inch  & 0xFF);
  }

  if(old10In != calibration10Inch) {
    EEPROM.write(CALIBRATION_10IN_EEPROM_START_ADDRESS, calibration10Inch >> 8);
    EEPROM.write(CALIBRATION_10IN_EEPROM_START_ADDRESS + 1, calibration10Inch  & 0xFF);
  }

  if(old12In != calibration12Inch) {
    EEPROM.write(CALIBRATION_12IN_EEPROM_START_ADDRESS, calibration12Inch >> 8);
    EEPROM.write(CALIBRATION_12IN_EEPROM_START_ADDRESS + 1, calibration12Inch  & 0xFF);
  }

  // If any values were changed, blink the play LED once to signify that the data was saved
  if(old12In != calibration12Inch || old10In != calibration10Inch || old7In != calibration7Inch) {
    digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
    delay(250);
    digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  }
}

// Each time the interrupt calls this function, the current milliseconds are polled, and compared against the last polling
// to calculate the speed the turntable is spinning on each rotation. This calculation is always occurring, even if the speed
// is not being displayed.
// This is attached to an interrupt because it is a time-sensitive operation, and having to wait for other code to finish executing
// would cause this to return inaccurate values.
void calculateTurntableSpeed() {
  currMillisSpeed = millis();
  currSpeed = 60000 / (double)(currMillisSpeed - lastMillisSpeed);
  lastMillisSpeed = currMillisSpeed;
}

// This stops all movement and sets the turntable in an error state to prevent damage.
// This will be called if a motor stall has been detected.
void setErrorState(MovementResult movementResult) {
  digitalWrite(ArduinoPin::PauseStatusLed, HIGH);
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);

  sevSeg.clear();
  sevSeg.writeDigitNum(0, movementResult, false);
  sevSeg.writeDisplay();

  tonearmController.setClutchPosition(HorizontalClutchPosition::Disengage);

  // Wait for the user to press the Play/Home or Pause/Unpause buttons to break out of the error state
  while(!mux.readDigitalValue(MultiplexerInput::PlayHomeButton) && !mux.readDigitalValue(MultiplexerInput::PauseButton)) { delay(1); }

  sevSeg.clear();
  sevSeg.writeDisplay();
}