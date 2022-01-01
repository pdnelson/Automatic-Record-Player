#include "../enums/MultiplexerInput.h"
#include "../enums/MovementResult.h"
#include "../enums/MotorAxis.h"
#include "../enums/TonearmMovementDirection.h"

#ifndef AutoTurntable_h
#define AutoTurntable_h

    // Main loop functions
    void monitorCommandButtons();
    void updateSevenSegmentDisplay();

    // Routine commands
    MovementResult playRoutine();
    MovementResult homeRoutine();
    MovementResult pauseOrUnpause();

    // Status/settings
    MultiplexerInput getActivePlaySensor();
    unsigned int getActiveSensorCalibration();

    // Turntable speed
    double calculateTurntableSpeed(double lastValue);

    // Error handling
    void setErrorState(MovementResult errorCode);

#endif