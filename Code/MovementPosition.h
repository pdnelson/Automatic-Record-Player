#ifndef MOVEMENTPOSITION_H
#define MOVEMENTPOSITION_H

// A position that the tonearm can move to.
enum class MovementPosition {

    // Tonearm moves up to the upper limit switch.
    UpperLimit,

    // Tonearm moves to the lower "home" limit switch.
    LowerLimit,

    // Tonearm moves to the "play" horizontal position.
    PlayPosition,

    // Tonearm moves to the "home" horizontal position.
    HomePosition
};

#endif