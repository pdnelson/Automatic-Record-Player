#include <Stepper.h>
#include <Multiplexer.h>
#include <DcMotor.h>
#include "proto/AutomaticTurntable.h"
#include "proto/Constants.h"
#include "enums/ArduinoPin.h"
#include "enums/MultiplexerInput.h"
#include "enums/MovementResult.h"
#include "enums/AutoManualSwitchPosition.h"
#include "TonearmMovementController.h"

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
  ArduinoPin::MuxSelectorC
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
  tonearmMotor,
  ArduinoPin::MotorAxisSelector,
  horizontalClutch,
  MultiplexerInput::VerticalLowerLimit,
  MultiplexerInput::VerticalUpperLimit
);

// All of these fields are used to calculate the speed that the turntable is spinning.
volatile unsigned long currMillisSpeed = millis();
volatile unsigned long lastMillisSpeed = currMillisSpeed;
volatile double currSpeed;

// Used for the pickup sensor
uint8_t consecutivePickupSensorChanges = 0;
bool lastPickupSensorStatus = false;
unsigned long lastMillisPickup = __LONG_MAX__;

bool paused = false;

void setup() { //Serial.begin(SERIAL_SPEED);
  // Set pins
  pinMode(ArduinoPin::MovementStatusLed, OUTPUT);
  pinMode(ArduinoPin::PauseStatusLed, OUTPUT);
  pinMode(ArduinoPin::SpeedSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(ArduinoPin::SpeedSensor), calculateTurntableSpeed, RISING);

  // Set calibration values
  tonearmController.setClutchEngagementMs(CLUTCH_ENGAGEMENT_MS);
  tonearmController.setTopMotorSpeed(MOVEMENT_RPM_TOP_SPEED);
  tonearmController.setVerticalTimeout(VERTICAL_MOVEMENT_TIMEOUT_STEPS);
  mux.setDelayMicroseconds(MULTIPLEXER_DELAY_MICROS);

  // Engage the horizontal clutch. This will be setting it to the "starting" point for where we know it is engaged
  horizontalClutch.immediateStart(HorizontalClutchPosition::Engage);

  // Begin startup light show
  delay(100);
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
  delay(100);
  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  digitalWrite(ArduinoPin::PauseStatusLed, HIGH);
  delay(100);
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);
  delay(100);
  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  // End startup light show

  // Check sensors, and perform an initial movement if necessary.
  MovementResult currentMovementStatus = MovementResult::None;

  bool homeExecuted = false;

  // If the turntable is turned on to "automatic," then home the whole tonearm if it is not already home.
  if(mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) == AutoManualSwitchPosition::Automatic && 
    digitalRead(ArduinoPin::HorizontalHomeOrPlayOpticalSensor)) {
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
  monitorPickupSensor();
}

// When a command button is pressed (i.e. Home/Play, or Pause/Unpause), then its respective command will be executed.
void monitorCommandButtons() {
  MovementResult currentMovementStatus = MovementResult::None;

  if(mux.readDigitalValue(MultiplexerInput::PauseButton)) {
    currentMovementStatus = pauseOrUnpause();
  }
  
  // If the play/home button is pressed, the command executes
  else if(mux.readDigitalValue(MultiplexerInput::PlayHomeButton)) {

    // If the tonearm is past the location of the home sensor, then this button will home it. Otherwise, it will execute
    // the play routine.
    if(!digitalRead(ArduinoPin::HorizontalHomeOrPlayOpticalSensor)) 
      currentMovementStatus = playRoutine();
    else 
      currentMovementStatus = homeRoutine();
  }

  // If the movement was anything other than success/none, then it failed, and we must set the error state.
  if(currentMovementStatus != MovementResult::Success && currentMovementStatus != MovementResult::None) {
    setErrorState(currentMovementStatus);
  }
}

// Monitor the pickup sensor. If this sensor detects that the tonearm is traveling over the end deadwax of a record, it
// it will execute the homing routine. This will only occur if the auto/manual switch is set to Automatic.
//
// TODO: This method is not very accurate, and results in a lot of misfires (or no-fires). In the future, a smaller
// sensor will need to be used, the interrupter's resolution will need to be increased, and an additional sensor will
// need to be added so that we can determine the direction the tonearm is moving.
//
// Presently, no direction is being checked, so every sensor change is being taken into account, which is problematic 
// because no record's grooves will sit perfectly in the center of a turntable. There will ALWAYS be some degree of 
// wobble, which can happen back and forth over the falling/rising edge of the sensor/interrupter.
void monitorPickupSensor() {
  if(!paused && mux.readDigitalValue(MultiplexerInput::AutoManualSwitch) == AutoManualSwitchPosition::Automatic) {
    bool currPickupSensorStatus = digitalRead(ArduinoPin::PickupEncoderA);
    unsigned long currMillisPickup = millis();

    if(currPickupSensorStatus != lastPickupSensorStatus) {

      // The base values were found through trial on error, so this equation just translates the interval to whatever speed the
      // turntable is currently spinning at.
      double pickupInterval = (TONEARM_PICKUP_BASE_INTERVAL - (TONEARM_PICKUP_BASE_INTERVAL * currSpeed / TONEARM_PICKUP_BASE_SPEED)) + TONEARM_PICKUP_BASE_INTERVAL;

      // Add to the consecutivePickupSensorChanges if we are within the pickup interval and debounce range. Otherwise, reset to 0.
      if((currMillisPickup - lastMillisPickup) > TONEARM_PICKUP_DEBOUNCE_MS && (currMillisPickup - lastMillisPickup) < pickupInterval) {
        consecutivePickupSensorChanges++;
      }
      else {
        consecutivePickupSensorChanges = 0;
      }

      lastMillisPickup = currMillisPickup;
      lastPickupSensorStatus = currPickupSensorStatus;
    }

    // After x consecutive sensor status changes between the debounce and pickup interval, run the homing routine.
    if(consecutivePickupSensorChanges == TONEARM_PICKUP_CONSECUTIVE_SENSOR_CHANGES) {
      MovementResult movementStatus = homeRoutine();

      // If the movement was anything other than success/none, then it failed, and we must set the error state.
      if(movementStatus != MovementResult::Success && movementStatus != MovementResult::None) {
        setErrorState(movementStatus);
      }

      consecutivePickupSensorChanges = 0;
    }
  }
}

// Move the tonearm clockwise to the play sensor
// This is a multi-movement routine, meaning that multiple tonearm movements are executed. If one of those movements fails, the
// whole routine is aborted.
MovementResult playRoutine() {
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);
  paused = false;

  MovementResult result = MovementResult::None;

  // TODO: Rewrite this using the new play routine.

  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  
  return result;
}

// Move the tonearm counterclockwise to the home sensor.
// This is a multi-movement routine, meaning that multiple tonearm movements are executed. If one of those movements fails, the
// whole routine is aborted.
MovementResult homeRoutine() {
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);
  paused = false;

  MovementResult result = MovementResult::None;

  result = tonearmController.moveUp(MOVEMENT_RPM_DEFAULT);
  if(result != MovementResult::Success) return result;

  // TODO: Shut turntable motor off here

  result = tonearmController.horizontalHome();
  if(result != MovementResult::Success) return result;

  result = tonearmController.moveDown(MOVEMENT_RPM_DEFAULT);
  if(result != MovementResult::Success) return result;

  digitalWrite(ArduinoPin::MovementStatusLed, LOW);

  return result;
}

// This is the pause routine that will lift up the tonearm from the record until the user "unpauses" by pressing the
// pause button again
MovementResult pauseOrUnpause() {
  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  digitalWrite(ArduinoPin::PauseStatusLed, HIGH);
  paused = true;

  MovementResult result = MovementResult::None;

  // If the vertical lower limit is pressed (i.e., the tonearm is vertically homed), then move it up
  if(mux.readDigitalValue(MultiplexerInput::VerticalLowerLimit)) {
    result = tonearmController.moveUp(MOVEMENT_RPM_DEFAULT);
  }

  // Otherwise, just move it down and then shut off the LED
  else {
    uint8_t tonearmSetRpm = 0;

    // If the tonearm is hovering over home position, then just go down at default speed
    if(!digitalRead(ArduinoPin::HorizontalHomeOrPlayOpticalSensor)) {
      tonearmSetRpm = MOVEMENT_RPM_DEFAULT;
    }

    // Otherwise, set it down carefully
    else tonearmSetRpm = MOVEMENT_RPM_CAREFUL;

    result = tonearmController.moveDown(tonearmSetRpm);

    digitalWrite(ArduinoPin::PauseStatusLed, LOW);
    paused = false;
  }

  return result;
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
// TODO: Re-implement error codes with LED flashes just like in the earliest revisions...
void setErrorState(MovementResult movementResult) {
  digitalWrite(ArduinoPin::PauseStatusLed, HIGH);
  digitalWrite(ArduinoPin::MovementStatusLed, HIGH);

  // Wait for the user to press the Play/Home or Pause/Unpause buttons to break out of the error state
  while(!mux.readDigitalValue(MultiplexerInput::PlayHomeButton) && !mux.readDigitalValue(MultiplexerInput::PauseButton)) { delay(1); }

  // Clear all statuses. Even though technically the next routine should execute right away, there's that 1/10000 chance that the user can
  // release the button quickly enough to break out of the error state, but not yet execute the next command
  digitalWrite(ArduinoPin::PauseStatusLed, LOW);
  digitalWrite(ArduinoPin::MovementStatusLed, LOW);
  paused = false;
}