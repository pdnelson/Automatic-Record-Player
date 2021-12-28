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
    bool moveTonearmHorizontally(MultiplexerInput destinationSensor, unsigned int timeout, int calibration, uint8_t speed = 8);
    bool moveTonearmVertically(MultiplexerInput destinationSensor, unsigned int timeout, uint8_t speed = 8);
    void horizontalRelativeMove(int steps, uint8_t speed = 8);
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