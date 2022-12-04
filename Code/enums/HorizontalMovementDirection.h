#ifndef HORIZONTAL_MOVEMENT_DIRECTION_H
#define HORIZONTAL_MOVEMENT_DIRECTION_H

// The horizontal direction that the tonearm can move.
enum HorizontalMovementDirection : int8_t {
    // The tonearm should move clockwise.
    Clockwise = 1,

    // The tonearm should move counterclockwise.
    Counterclockwise = -1,
};

#endif