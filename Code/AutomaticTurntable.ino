#include <Stepper.h>

#define SERIAL_SPEED 115200

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
#define HOME_STATUS_LED 9
#define PLAY_STATUS_LED 11
#define PAUSE_STATUS_LED 13

// Positioning sensors for the vertical tonearm movement. The lower limit switch designates "home" for the tonearm's vertical position
// The upper sensor is the "pause" position, and will allow the tonearm to go higher to engage with the gearing that moves horizontally
#define VERTICAL_UPPER_SENSOR A0
#define VERTICAL_LOWER_LIMIT A1

// Positioning sensors for the horizontal tonearm movement. Each one represents a different point that the horizontal tonearm could be in,
// and will allow the various funcrtions to move it in the correct direction depending on where it currently is.
#define HORIZONTAL_HOME_SENSOR A2 // Tells the tonearm where "home" is horizontally
#define HORIZONTAL_PLAY_SENSOR A3 // Tells the tonearm where to drop the stylus down on the record
#define HORIZONTAL_PICKUP_SENSOR A4 // Tells the tonearm where the end of the record is

// This tells us whether the user flipped the 3-way switch to "automatic" or "manual"
// Automatic will automatically home the turntable at the end of the record, while manual
// will not. Even with manual selected, the "home," "pause" and "play" buttons will still work
#define AUTO_OR_MANUAL A5

// This is an internal step count for the motors, so we know how many steps it has taken
int32_t verticalStepCount = 0;
int32_t horizontalStepCount = 0;

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

  pinMode(HOME_STATUS_LED, OUTPUT);
  pinMode(PLAY_STATUS_LED, OUTPUT);
  pinMode(PAUSE_STATUS_LED, OUTPUT);

  Serial.begin(SERIAL_SPEED);

  bool verticalUpperIsPausedOrEngaged = digitalRead(VERTICAL_UPPER_SENSOR);

  // If the tonearm is currently lifted up to a "paused" or "moving" state, then we want to home it
  if(verticalUpperIsPausedOrEngaged) {
    homeBothAxes();
  }

  // Otherwise, we only want to home the vertical axis, because the stylus may already be on a record
  else {
    homeVerticalAxis();
  }
}

void loop() {

  // IR SENSOR TEST
  bool sensorStatus = digitalRead(VERTICAL_UPPER_SENSOR);

  // If the left button is pressed, move clockwise and light the left LED
  if(sensorStatus) {
    VerticalTonearmMotor.step(1);
    verticalStepCount++;

    HorizontalTonearmMotor.step(1);
    horizontalStepCount++;
  }
}

// This is only used when first powering on to verify the tonearm is not being moved
// If it is found that the pause sensor is not tripped, then it will know that the tonearm
// was previously lifted, and should be homed instead
void homeVerticalAxis() {
    // TODO: Implement

    // Now that the vertical axis has been homed, the step count can be reset to 0
    verticalStepCount = 0;
}

void homeBothAxes() {
  //TODO: Implement

  // Now that the axes have been homed, their step counts can be reset to 0
  verticalStepCount = 0;
  horizontalStepCount = 0;
}