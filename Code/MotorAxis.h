#ifndef MOTORAXIS_H
#define MOTORAXIS_H

// One of the motors that adjusts the position of the tonearm.
enum class MotorAxis {

    // Motor that controls the vertical tonearm movement.
    Vertical,

    // Motor that controls the horizontal tonearm movement.
    Horizontal,

    // Both vertical and horizontal motors that control the tonearm movement.
    VerticalAndHorizontal
};

#endif