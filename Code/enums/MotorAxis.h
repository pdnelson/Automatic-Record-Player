#ifndef MOTORAXIS_H
#define MOTORAXIS_H

// One of the motors that adjusts the position of the tonearm.
enum MotorAxis : uint8_t {

    // Motor that controls the vertical tonearm movement.
    Vertical = 0x1,

    // Motor that controls the horizontal tonearm movement.
    Horizontal = 0x0
};

#endif