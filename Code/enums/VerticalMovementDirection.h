#ifndef VERTICAL_MOVEMENT_DIRECTION_H
#define VERTICAL_MOVEMENT_DIRECTION_H

// The vertical direction that the tonearm can move.
enum VerticalMovementDirection : int8_t {
    // The tonearm should move up.
    Up = -1,

    // The tonearm should move down.
    Down = 1
};

#endif