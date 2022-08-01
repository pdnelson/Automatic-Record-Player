// See Constants.h file for more details.

// These constants are only to be used by the AutomaticTurntable.ino file.

/********** SETUP */

    //#define SERIAL_SPEED 115200

    // The number of steps it takes for either stepper motor to make a full 360-degree rotation.
    #define STEPS_PER_REVOLUTION 2048

    // The start address of the seven-segment display.
    #define SEV_SEG_START_ADDRESS 0x70

/********** CALIBRATION VALUES */

    // The "middle of the road" RPM that a stepper should move.
    #define MOVEMENT_RPM_DEFAULT 10

    // How fast the stepper should move when it is seeking an optical sensor.
    #define MOVEMENT_RPM_SENSOR_SEEK 8

    // The top speed that a stepper will ever move at.
    #define MOVEMENT_RPM_TOP_SPEED 14

    // How fast the vertical stepper should move when it is carefully setting down the tonearm.
    #define MOVEMENT_RPM_CAREFUL 3

    // The approximate number of steps between the play sensor, and the homing mount. This value is only used to move
    // the tonearm counterclockwise, so it accounts for some extra steps to push the tonearm into the mount.
    #define STEPS_FROM_PLAY_SENSOR_HOME 700

    // These are timeouts used for error checking, so the hardware doesn't damage itself.
    // Essentially, if the steps exceed this number and the motor has not yet reached its
    // destination, an error has occurred.
    #define VERTICAL_MOVEMENT_TIMEOUT_STEPS 1500
    #define HORIZONTAL_MOVEMENT_TIMEOUT_STEPS 3000

    #define CLUTCH_ENGAGEMENT_MS 200

    #define MULTIPLEXER_DELAY_MICROS 10

/********** USER-SET CALIBRATION VALUES */

    #define CALIBRATION_HOLD_CHANGE_MS 200 // The interval between value updates
    #define CALIBRATION_HOLD_DECREMENT_MS 20 // The number of MS removed from the overall change MS when the hold change interval is reached
    #define CALIBRATION_HOLD_LOWEST_MS 20 // The lowest number that the hold change MS can go to before not being subtracted from anymore.
    #define CALIBRATION_HOLD_CHANGE_INTERVAL 3 // The number of updates before the change MS decreases
    #define CALIBRATION_DEBOUNCE_MS 100

    // The maximum value that can be stored for the calibration.
    #define CALIBRATION_VALUE_MAX 2500

/********** USER-SET CALIBRATION VALUE DEFAULTS */

    #define CALIBRATION_7IN_DEFAULT 7
    #define CALIBRATION_10IN_DEFAULT 10
    #define CALIBRATION_12IN_DEFAULT 12

/********** EEPROM ADDRESSES */

    #define CALIBRATION_7IN_EEPROM_START_ADDRESS 0
    #define CALIBRATION_10IN_EEPROM_START_ADDRESS 2
    #define CALIBRATION_12IN_EEPROM_START_ADDRESS 4