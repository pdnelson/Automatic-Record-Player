#include "../enums/MultiplexerInput.h"
#include "../enums/MovementResult.h"
#include "../enums/MotorAxis.h"
#include "../enums/TonearmMovementDirection.h"

#ifndef AutoTurntable_h
#define AutoTurntable_h

    // Routine commands
    MovementResult playRoutine();
    MovementResult homeRoutine();
    MovementResult pauseOrUnpause();

    // Status/settings
    MultiplexerInput getActivePlaySensor();
    unsigned int getActiveSensorCalibration();

    // Turntable speed
    void calculateTurntableSpeedAndPrintToDisplay();

    // Error handling
    void setErrorState(MovementResult errorCode);

#endif