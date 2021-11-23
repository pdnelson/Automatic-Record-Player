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

    // The 7" "play" position optical sensor, which indicates where the tonearm would drop to begin playing a 7" record.
    HorizontalPlay7InchOpticalSensor = 5,

    // The 10" "play" position optical sensor, which indicates where the tonearm would drop to begin playing a 10" record.
    HorizontalPlay10InchOpticalSensor = 6,

    // The 12" "play" position optical sensor, which indicates where the tonearm would drop to begin playing a 12" record.
    HorizontalPlay12InchOpticalSensor = 7,

    // This, used together with RecordSizeSelector2, tells the software which record size it should be looking for.
    // If this is HIGH while RecordSizeSelector2 is LOW, the software will use the 7" optical sensor.
    // If both this and RecordSizeSelector2 are LOW, the software will use the 10" optical sensor.
    RecordSizeSelector1 = 8,

    // This, used together with RecordSizeSelector1, tells the software which record size it should be looking for.
    // If this is HIGH while RecordSizeSelector1 is LOW, the software will use the 12" optical sensor.
    // If both this and RecordSizeSelector2 are LOW, the software will use the 10" optical sensor.
    RecordSizeSelector2 = 9,

    // The "pickup" position optical sensor, which indicates the location the tonearm must be at in order for the homing routine to be executed.
    // Ideally, this would be at the inner deadwax of the record.
    HorizontalPickupOpticalSensor = 10,

    // This is the switch used to determine whether the user turned on the turntable in automatic or manual mode.
    // Automatic: The tonearm will return to "home" at the end of a record.
    // Manual: The tonearm will not return to "home" at the end of a record. All buttons still have their same functionality in "automatic" mode.
    AutoManualSwitch = 11,

    // This is an optical sensor used to detect what speed the turntable is currently spinning at.
    TurntableSpeedSensor = 12
};

#endif