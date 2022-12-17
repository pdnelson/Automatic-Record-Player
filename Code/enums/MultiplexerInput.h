#ifndef INPUTMULTIPLEXERINPUTS_H
#define INPUTMULTIPLEXERINPUTS_H

// One of the motors that adjusts the position of the tonearm.
enum MultiplexerInput : uint8_t {

    UNUSED_MUX_0 = 0,
    
    // The "play"/"home" button, which will move the tonearm (horizontally) to the play sensor if the play sensor is HIGH, or 
    // move the tonearm (horizontally) to the home sensor if the play sensor is LOW. 
    PlayHomeButton = 1,

    // The "pause" button, which will either lift the tonearm up (if it is currently down), or set the tonearm down (if it is currently up).
    PauseButton = 2,
    
    UNUSED_MUX_3 = 3,
    
    // Positioning sensor for the vertical tonearm movement. This is the upper sensor, indicating the "pause" vertical position of the tonearm.
    VerticalUpperLimit = 4,

    // This tells us whether the turntable power has been set, or if it's currently turned off.
    TurntablePowerSet = 5,

    // Positioning sensor for the vertical tonearm movement. This is the lower sensor, indicating the "play"/"home" vertical position of the tonearm.
    VerticalLowerLimit = 6,

    // This pin decides whether we are in automatic or manual mode. To know what each value does, see AutoManualSwitchPosition.h.
    AutoManualSwitch = 7
};

#endif