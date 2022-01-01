#include "../enums/MultiplexerInput.h"
#include "../enums/MovementResult.h"

#ifndef AutoTurntable_h
#define AutoTurntable_h

    /* Main loop functions */
    void monitorCommandButtons();
    void updateSevenSegmentDisplay();


    /* Routine commands */

    // Move the tonearm clockwise to the active play sensor, designated by getActivePlaySensor()
    // This is a multi-movement routine, meaning that multiple tonearm movements are executed. If one of those movements fails, the
    // whole routine is aborted.
    MovementResult playRoutine();

    // Move the tonearm counterclockwise to the home sensor.
    // This is a multi-movement routine, meaning that multiple tonearm movements are executed. If one of those movements fails, the
    // whole routine is aborted.
    MovementResult homeRoutine();

    // This is the pause routine that will lift up the tonearm from the record until the user "unpauses" by pressing the
    // pause button again
    MovementResult pauseOrUnpause();


    /* Status/settings */

    // This uses RecordSizeSelector1 and RecordSizeSelector2 to determine the record size the user
    // currently has selected.
    MultiplexerInput getActivePlaySensor();

    // Returns the calibration step offset for the given sensor.
    // The returned value will be the number of steps (clockwise or counterclockwise) that the horizontal motor should move.
    // Pickup calibration return value is expressed in number of seconds that should be waited before the homing routine is
    // executed at the end of a record.
    unsigned int getActiveSensorCalibration();


    /* Turntable speed */

    // This function will calculate the speed of the turntable 8 times per rotation.
    double calculateTurntableSpeed(double lastValue);


    /* Error handling */

    // This stops all movement and sets the turntable in an error state to prevent damage.
    // This will be called if a motor stall has been detected.
    void setErrorState(MovementResult errorCode);

#endif