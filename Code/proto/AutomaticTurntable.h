#include "../enums/MultiplexerInput.h"
#include "../enums/MovementResult.h"
#include "../enums/RecordSize.h"

#ifndef AutoTurntable_h
#define AutoTurntable_h

    /* Main loop functions */
    void monitorCommandButtons();
    void monitorSevenSegmentInput();
    void monitorPickupSensor();
    void calibrationSettingLoop();
    void updateSevenSegmentDisplay(double newValue);

    /* Routine commands */
    MovementResult playRoutine();
    MovementResult homeRoutine();
    MovementResult pauseOrUnpause();

    /* Status/settings */
    uint16_t getActiveSensorCalibration();
    RecordSize getActiveRecordSize();
    void loadCalibrationEEPROMValues(bool loadDefaults);
    void updateCalibrationEEPROMValues(uint16_t old7In, uint16_t old10In, uint16_t old12In);

    /* Turntable speed */
    void calculateTurntableSpeed();

    /* Error handling */
    void setErrorState(MovementResult errorCode);

#endif