#include "../enums/MultiplexerInput.h"
#include "../enums/MovementResult.h"

#ifndef AutoTurntable_h
#define AutoTurntable_h

    /* Main loop functions */
    void monitorCommandButtons();
    void monitorPickupSensor();

    /* Routine commands */
    MovementResult playRoutine();
    MovementResult homeRoutine();
    MovementResult pauseOrUnpause();

    /* Turntable speed */
    void calculateTurntableSpeed();

    /* Error handling */
    void setErrorState(MovementResult errorCode);

#endif