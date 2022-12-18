#ifndef INPUTMULTIPLEXERINPUTS_H
#define INPUTMULTIPLEXERINPUTS_H

// One of the motors that adjusts the position of the tonearm.
enum MultiplexerInput : uint8_t {

    // Target speed switch A. This value is determined by the position of the main speed switch.
    TargetSpeedA = 0,
    
    // The "play"/"home" button, which will move the tonearm (horizontally) to the play sensor if the play sensor is HIGH, or 
    // move the tonearm (horizontally) to the home sensor if the play sensor is LOW. 
    PlayHomeButton = 1,

    // The "pause" button, which will either lift the tonearm up (if it is currently down), or set the tonearm down (if it is currently up).
    PauseButton = 2,
    
    // Taget speed switch B. This vlaue is determined by the position of the secondary speed button.
    TargetSpeedB = 3,

    // If this is high, when executing the "play" routine, the tonearm will will wait until it's within a certain percentage of the target speed
    // before lowering.
    WaitUntilTargetSpeed = 4,

    // Positioning sensor for the vertical tonearm movement. This is the lower sensor, indicating the "play"/"home" vertical position of the tonearm.
    VerticalLowerLimit = 5,
    
    // Positioning sensor for the vertical tonearm movement. This is the upper sensor, indicating the "pause" vertical position of the tonearm.
    VerticalUpperLimit = 6,

    // This pin decides whether we are in automatic or manual mode. To know what each value does, see AutoManualSwitchPosition.h.
    AutoManualSwitch = 7
};

#endif