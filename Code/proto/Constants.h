// See Constants.h file for more details.

// These constants are only to be used by the AutomaticTurntable.ino file.

/********** SETUP */

    //#define SERIAL_SPEED 115200

    // The number of steps it takes for either stepper motor to make a full 360-degree rotation.
    #define STEPS_PER_REVOLUTION 2048

/********** CALIBRATION VALUES */

    // The "middle of the road" RPM that a stepper should move.
    #define MOVEMENT_RPM_DEFAULT 10

    // How fast the stepper should move when it is seeking an optical sensor.
    #define MOVEMENT_RPM_SENSOR_SEEK 5

    // The top speed that a stepper will ever move at.
    #define MOVEMENT_RPM_TOP_SPEED 14

    // How fast the vertical stepper should move when it is carefully setting down the tonearm.
    #define MOVEMENT_RPM_CAREFUL 4

    // The approximate number of steps between the play sensor, and the homing mount. This value is only used to move
    // the tonearm counterclockwise, so it accounts for some extra steps to push the tonearm into the mount.
    #define STEPS_FROM_PLAY_SENSOR_HOME 400

    // These are timeouts used for error checking, so the hardware doesn't damage itself.
    // Essentially, if the steps exceed this number and the motor has not yet reached its
    // destination, an error has occurred.
    #define VERTICAL_MOVEMENT_TIMEOUT_STEPS 1500
    #define HORIZONTAL_MOVEMENT_TIMEOUT_STEPS 3000

    #define CLUTCH_ENGAGEMENT_MS 100

    #define MULTIPLEXER_DELAY_MICROS 10

    // The amount of time since the last speed sensor interrupt before we consider the turntable "stopped"
    #define TURNTABLE_STOPPED_MS 2500

/********** TONEARM PICKUP CALIBRATION VALUES */

#define TONEARM_PICKUP_DEBOUNCE_MS 20 // Debounce interval to help prevent the tonearm pickup sensor from misfiring.
#define TONEARM_PICKUP_CONSECUTIVE_SENSOR_CHANGES 3 // The number of sensor changes that must happen within the pickup interval to trigger the homing routine.

// I've found that sensor trips 700 ms apart at 45.5 RPM will yield positive results; home routine not triggering during
// record play, but triggering during end deadwax. For this reason, we apply speed ratio to translate 700 ms to an interval
// that will work with the exact speed the turntable is spinning at.
#define TONEARM_PICKUP_BASE_SPEED 45.5 // The base speed the turntable was spinning at when the base interval was found through trial and error.
#define TONEARM_PICKUP_BASE_INTERVAL 700 // The base interval that the pickup sensor must change when the turntable is spinning at TONEARM_PICKUP_BASE_SPEED RPM