#ifndef INPUTMULTIPLEXERINPUTS_H
#define INPUTMULTIPLEXERINPUTS_H

// One of the motors that adjusts the position of the tonearm.
enum MultiplexerInput : uint8_t {

    // The "play"/"home" button, which will move the tonearm (horizontally) to the play sensor if the play sensor is HIGH, or 
    // move the tonearm (horizontally) to the home sensor if the play sensor is LOW.
    PlayHomeButton = 0,

    // The "pause" button, which will either lift the tonearm up (if it is currently down), or set the tonearm down (if it is currently up).
    PauseButton = 1,

    // Positioning sensor for the vertical tonearm movement. This is the upper sensor, indicating the "pause" vertical position of the tonearm.
    VerticalUpperLimit = 2,

    // Positioning sensor for the vertical tonearm movement. This is the lower sensor, indicating the "play"/"home" vertical position of the tonearm.
    VerticalLowerLimit = 3,

    // The "home" position optical sensor, which indicates what is considered "home" for the tonearm.
    HorizontalHomeOpticalSensor = 4,

    // The "play" position optical sensor, which indicates what is considered the "play" position for the tonearm. Ideally, this would be
    // the edge of a record.
    HorizontalPlayOpticalSensor = 5,

    // The "pickup" position optical sensor, which indicates the location the tonearm must be at in order for the homing routine to be executed.
    // Ideally, this would be at the inner deadwax of the record.
    HorizontalPickupOpticalSensor = 6,

    // This is the switch used to determine whether the user turned on the turntable in automatic or manual mode.
    // Automatic: The tonearm will return to "home" at the end of a record.
    // Manual: The tonearm will not return to "home" at the end of a record. All buttons still have their same functionality in "automatic" mode.
    AutoManualSwitch = 7
};

#endif