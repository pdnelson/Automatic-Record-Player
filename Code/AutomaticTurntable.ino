#include <Stepper.h>

const int stepsPerRevolution = 2048;

// The vertical stepper motor controls the up and down movements of the tonearm, such as lifting the stylus off of the 
// record or setting it down
#define STEPPER_VERTICAL_PIN1 0
#define STEPPER_VERTICAL_PIN2 1
#define STEPPER_VERTICAL_PIN3 2
#define STEPPER_VERTICAL_PIN4 3
Stepper VerticalTonearmMotor = Stepper(stepsPerRevolution, STEPPER_VERTICAL_PIN1, STEPPER_VERTICAL_PIN3, STEPPER_VERTICAL_PIN2, STEPPER_VERTICAL_PIN4);

// The horizontal stepper motor controls the left and right movements of the tonearm, such as positioning it at a
// horizontal axis so the vertical movement can place the tonearm at the correct location
#define STEPPER_HORIZONTAL_PIN1 4
#define STEPPER_HORIZONTAL_PIN2 5
#define STEPPER_HORIZONTAL_PIN3 6
#define STEPPER_HORIZONTAL_PIN4 7
Stepper HorizontalTonearmMotor = Stepper(stepsPerRevolution, STEPPER_HORIZONTAL_PIN1, STEPPER_HORIZONTAL_PIN3, STEPPER_HORIZONTAL_PIN2, STEPPER_HORIZONTAL_PIN4);

// Buttons that the user can press to execute certain movements
#define HOME_BUTTON 8
#define PLAY_BUTTON 10
#define PAUSE_BUTTON 12

// Indicator lights so we can tell what movement the turntable is currently executing
#define HOME_STATUS_LED_AND_HORIZONTAL_GEARING_SOLENOID 9
#define PLAY_STATUS_LED 11
#define PAUSE_STATUS_LED 13

// Positioning sensors for the vertical tonearm movement. The lower limit switch designates "home" for the tonearm's vertical position
// The upper sensor is the "pause" position, and will allow the tonearm to go higher to engage with the gearing that moves horizontally
#define VERTICAL_PICKUP_LIMIT A0
#define VERTICAL_HOME_LIMIT A1

// Positioning sensors for the horizontal tonearm movement. Each one represents a different point that the horizontal tonearm could be in,
// and will allow the various funcrtions to move it in the correct direction depending on where it currently is.
#define HORIZONTAL_HOME_SENSOR A2 // Tells the tonearm where "home" is horizontally
#define HORIZONTAL_PLAY_SENSOR A3 // Tells the tonearm where to drop the stylus down on the record
#define HORIZONTAL_PICKUP_SENSOR A4 // Tells the tonearm where the end of the record is

// This tells us whether the user flipped the 3-way switch to "automatic" or "manual"
// Automatic will automatically home the turntable at the end of the record, while manual
// will not. Even with manual selected, the "home," "pause" and "play" buttons will still work
#define AUTO_OR_MANUAL_SWITCH A5 // Auto = high; manual = low

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
  
  pinMode(HOME_BUTTON, INPUT);
  pinMode(PLAY_BUTTON, INPUT);
  pinMode(PAUSE_BUTTON, INPUT);

  pinMode(HOME_STATUS_LED_AND_HORIZONTAL_GEARING_SOLENOID, OUTPUT);
  pinMode(PLAY_STATUS_LED, OUTPUT);
  pinMode(PAUSE_STATUS_LED, OUTPUT);

  // If the turntable is turned on to "automatic," then home the whole tonearm
  if(digitalRead(AUTO_OR_MANUAL_SWITCH)) {
    homeTonearm();
  }

  // Otherwise, we only want to home the vertical axis if it isn't already homed, which will drop the tonearm in its current location
  else if(!digitalRead(VERTICAL_HOME_LIMIT)) {
    homeVerticalAxis();
  }
}

// This sits and waits for any of the command buttons to be pressed.
// As soon as a button is pressed, the corresponding command routine is executed
void loop() {
  if(digitalRead(PLAY_BUTTON)) {
    playRoutine();
  }

  if(digitalRead(PAUSE_BUTTON)) {
    pauseAndWaitUntilUnpaused();
  }

  if(digitalRead(HOME_BUTTON)) {
    homeTonearm();
  }
}

// This is only used when first powering on in manual mode to verify that the tonearm
// drops down in place, if it was previously shut off between the sensor and home switch
// The home status LED will light for the duration of this command
void homeVerticalAxis() {
    // Move the motor down until it bumps into the limit
    while(!digitalRead(VERTICAL_HOME_LIMIT))
    {
      VerticalTonearmMotor.step(-1);
    }
}

void homeTonearm() {
  // TODO: Implement
}

void pauseAndWaitUntilUnpaused() {
  digitalWrite(PAUSE_STATUS_LED, HIGH);

  // Move the motor up until the pick-up limit is hit
  while(!digitalRead(VERTICAL_PICKUP_LIMIT)) {
    VerticalTonearmMotor.step(1);
  }

  // Wait for the user to unpause
  while(!digitalRead(PAUSE_BUTTON)) {
    delay(1);
  }

  // Move the motor down until the vertical home limit is hit
  while(!digitalRead(VERTICAL_HOME_LIMIT)) {
    VerticalTonearmMotor.step(-1);
  }

  digitalWrite(PAUSE_STATUS_LED, LOW);
}

void playRoutine() {
  // TODO: Implement
}