#ifndef ARDUINOPIN_H
#define ARDUINOPIN_H

// Each of these values corresponds to a pin on the Arduino Nano Every
enum ArduinoPin : uint8_t {

    // Currently reserved in the event of serial output being needed. This will also assure that there are no errors
    // when uploading to the Arduino while it is connected to the rest of the turntable controller board.
    ReservedSerial0 = 0,

    // Currently reserved in the event of serial output being needed. This will also assure that there are no errors
    // when uploading to the Arduino while it is connected to the rest of the turntable controller board.
    ReservedSerial1 = 1,

    // One (of four) multiplexer selector pins. This is used (with the other pins) to select which device we want
    // to read a value from.
    MuxSelectorA = 2,

    // One (of four) multiplexer selector pins. This is used (with the other pins) to select which device we want
    // to read a value from.
    MuxSelectorB = 3,

    // One (of four) multiplexer selector pins. This is used (with the other pins) to select which device we want
    // to read a value from.
    MuxSelectorC = 4,

    Unused_5 = 5,

    // This is the multiplexer pin that we read values from.
    MuxOutput = 6,

    // This motor pin (4) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin4 = 7,

    // This motor pin (3) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin3 = 8,

    // This motor pin (2) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin2 = 9,

    // This motor pin (1) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin1 = 11,

    // This is the pin used to select which motor we are moving, using the demultiplexers.
    MotorAxisSelector = 10,

    // Indicator light so we can tell what the turntable is currently doing. This pin will be set HIGH if the turntable
    // is currently executing the "home" or "play" routine.
    MovementStatusLed = A0,

    // Indicator light so we can tell what the turntable is currently doing. This pin will be set HIGH if the turntable
    // is currently executing the "pause" routine, or if the tonearm is currently in the "pause" position.
    PauseStatusLed = 13,

    // Setting this HIGH will cause the the horizontal motor clutch to either engage or disengage. This is used with HorizontalClutchMotorDir2
    // to drive a DC motor through an H-bridge.
    HorizontalClutchMotorDir1 = 12,

    // Setting this HIGH will cause the the horizontal motor clutch to either engage or disengage. This is used with HorizontalClutchMotorDir1
    // to drive a DC motor through an H-bridge.
    HorizontalClutchMotorDir2 = A1,

    // The "home" or "play" position optical sensor, depending on which direction the tonearm is moving. If the tonearm is moving
    // counter-clockwise, this sensor acts as the "home" sensor, whereas if it is moving clockwise, it acts as the play sensor.
    HorizontalHomeOrPlayOpticalSensor = A2,

    // This is used, in combination with an interrupt, to calculate what speed the turntable is currently spinning at.
    SpeedSensor = A3,

    // Currently in use by the Adafruit 7-segment display. This display deals with displaying the current speed of the turntable,
    // as well as calibration values when the calibration button is being held.
    Reserved7SegSda = A4,

    // Currently in use by the Adafruit 7-segment display. This display deals with displaying the current speed of the turntable,
    // as well as calibration values when the calibration button is being held.
    Reserved7SegScl = A5,

    // The "pickup" position optical sensor, which, when tripped so many times within a certain interval, will trigger the homing routine.
    HorizontalPickupOpticalSensor = A6,

    // The direction the tonearm is moving when the pickup sensor is tripped. This is currently not being used.
    HorizontalPickupDirectionOpticalSensor = A7
};

#endif