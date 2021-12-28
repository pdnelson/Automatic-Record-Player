#include "../enums/MultiplexerInput.h"
#include "../enums/ErrorCode.h"
#include "../enums/MotorAxis.h"
#include "../enums/TonearmMovementDirection.h"

#ifndef AutoTurntable_h
#define AutoTurntable_h

    // Routine commands
    ErrorCode homeRoutine();
    ErrorCode pauseAndWaitUntilUnpaused();
    ErrorCode playRoutine();

    // Tonearm movement
    bool moveTonearmToSensor(MotorAxis axis, MultiplexerInput destinationSensor, uint8_t speed, unsigned int timeout, unsigned int calibration = 0);
    void horizontalRelativeMove(int steps);
    void releaseCurrentFromMotors();

    // Status/settings
    MultiplexerInput getActivePlaySensor();
    unsigned int getHorizontalSensorCalibration(MultiplexerInput sensor);

    // Turntable speed
    void calculateTurntableSpeed();

    // Error handling
    void setErrorState(ErrorCode errorCode);
    void blinkLed(int led, int interval);

#endif