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

    // This is the multiplexer pin that we read values from.
    MuxOutput = 2,

    // This motor pin (4) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin4 = 3,

    // This motor pin (3) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin3 = 4,

    // This motor pin (2) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin2 = 5,

    // This motor pin (1) channels into a 2-channel demultiplexer, so that either the vertical or horizontal motor receives
    // the pulses. Only one of these motors can ever be moving at once.
    StepperPin1 = 6,

    // This is the pin used to select which motor we are moving, using the demultiplexers.
    MotorAxisSelector = 7,

    // Setting this HIGH will cause the the horizontal motor clutch to either engage or disengage. This is used with HorizontalClutchMotorDir1
    // to drive a DC motor through an H-bridge.
    HorizontalClutchMotorDir2 = 8,

    // Setting this HIGH will cause the the horizontal motor clutch to either engage or disengage. This is used with HorizontalClutchMotorDir2
    // to drive a DC motor through an H-bridge.
    HorizontalClutchMotorDir1 = 9,

    // One (of four) multiplexer selector pins. This is used (with the other pins) to select which device we want
    // to read a value from.
    MuxSelectorA = 10,

    // One (of four) multiplexer selector pins. This is used (with the other pins) to select which device we want
    // to read a value from.
    MuxSelectorB = 11,

    // One (of four) multiplexer selector pins. This is used (with the other pins) to select which device we want
    // to read a value from.
    MuxSelectorC = 12,

    // A solenoid that is used to brake the turntable. Because the platter is made of solid aluminum, it maintains a lot of momentum even after the motor
    // stops spinning, so this brake will slow it down much more quickly.
    TurntableBrake = 13,

    // The "home" or "play" position optical sensor, depending on which direction the tonearm is moving. If the tonearm is moving
    // counter-clockwise, this sensor acts as the "home" sensor, whereas if it is moving clockwise, it acts as the play sensor.
    HorizontalHomeOrPlayOpticalSensor = A0,

    // This is used, in combination with an interrupt, to calculate what speed the turntable is currently spinning at.
    SpeedSensor = A1,

    // Sensor A of the pickup optical sensor, which, when tripped so many times within a certain interval (also depending on B), will 
    // trigger the homing routine.
    PickupEncoderA = A2,

    // Sensor B of the pickup optical sensor, which, when tripped so many times within a certain interval (also depending on B), will 
    // trigger the homing routine.
    PickupEncoderB = A3,

    // Currently in use by the Adafruit 7-segment display. This display deals with displaying the current speed of the turntable,
    // as well as calibration values when the calibration button is being held.
    Reserved7SegSda = A4,

    // Currently in use by the Adafruit 7-segment display. This display deals with displaying the current speed of the turntable,
    // as well as calibration values when the calibration button is being held.
    Reserved7SegScl = A5,

    // Indicator light so we can tell what the turntable is currently doing. This pin will be set HIGH if the turntable
    // is currently executing the "home" or "play" routine.
    MovementStatusLed = A6,

    // Indicator light so we can tell what the turntable is currently doing. This pin will be set HIGH if the turntable
    // is currently executing the "pause" routine, or if the tonearm is currently in the "pause" position.
    PauseStatusLed = A7
};

#endif