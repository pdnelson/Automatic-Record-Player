#include "../enums/MultiplexerInput.h"
#include "../enums/MovementResult.h"

#ifndef AutoTurntable_h
#define AutoTurntable_h

    /* Main loop functions */
    void monitorCommandButtons();
    void updateSevenSegmentDisplay();

    /* Routine commands */
    MovementResult playRoutine();
    MovementResult homeRoutine();
    MovementResult pauseOrUnpause();

    /* Status/settings */
    unsigned int getActiveSensorCalibration();

    /* Turntable speed */
    void calculateTurntableSpeed();

    /* Error handling */
    void setErrorState(MovementResult errorCode);

#endif