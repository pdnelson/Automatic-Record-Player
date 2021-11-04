#ifndef TONEARM_MOVEMENT_DIRECTION_H
#define TONEARM_MOVEMENT_DIRECTION_H

// One of the motors that adjusts the position of the tonearm.
enum TonearmMovementDirection : int8_t {

    // The tonearm is moving clockwise horizontally.
    Clockwise = -1,

    // The tonearm is moving counterclockwise horizontally.
    Counterclockwise = 1,

    // The tonearm is moving up vertically.
    Positive = 1,

    // The tonearm is moving down vertically.
    Negative = -1
};

#endif