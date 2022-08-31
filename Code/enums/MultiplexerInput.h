#ifndef INPUTMULTIPLEXERINPUTS_H
#define INPUTMULTIPLEXERINPUTS_H

// One of the motors that adjusts the position of the tonearm.
enum MultiplexerInput : uint8_t {

    // Positioning sensor for the vertical tonearm movement. This is the lower sensor, indicating the "play"/"home" vertical position of the tonearm.
    VerticalLowerLimit = 0,
    
    // Positioning sensor for the vertical tonearm movement. This is the upper sensor, indicating the "pause" vertical position of the tonearm.
    VerticalUpperLimit = 3,
    
    // The "play"/"home" button, which will move the tonearm (horizontally) to the play sensor if the play sensor is HIGH, or 
    // move the tonearm (horizontally) to the home sensor if the play sensor is LOW. 
    PlayHomeButton = 2,

    // The "pause" button, which will either lift the tonearm up (if it is currently down), or set the tonearm down (if it is currently up).
    PauseButton = 1,

    // This, used together with RecordSizeSelector2, tells the software which record size it should be looking for.
    // If this is HIGH while RecordSizeSelector2 is LOW, the software will use the 7" optical sensor.
    // If both this and RecordSizeSelector2 are LOW, the software will use the 10" optical sensor.
    RecordSizeSelector1 = 4,

    // This, used together with RecordSizeSelector1, tells the software which record size it should be looking for.
    // If this is HIGH while RecordSizeSelector1 is LOW, the software will use the 12" optical sensor.
    // If both this and RecordSizeSelector2 are LOW, the software will use the 10" optical sensor.
    AutoManualSwitch = 5,

    // This is a button that the user can press that will display, numerically, the value of an analog potentiometer that corresponds with the current
    // record size that is selected.
    DisplayCalibrationValue = 6,
    
    // This is the switch used to determine whether the user turned on the turntable in automatic or manual mode.
    // Automatic: The tonearm will return to "home" at the end of a record.
    // Manual: The tonearm will not return to "home" at the end of a record. All buttons still have their same functionality in "automatic" mode.
    RecordSizeSelector2 = 7
};

#endif