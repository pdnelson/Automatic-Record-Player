#ifndef MOVEMENTRESULT_H
#define MOVEMENTRESULT_H

// Various errors that the turntable could encounter during operation
enum MovementResult : uint8_t {
    // Took too long for the tonearm to move up to its destination.
    VerticalPositiveDirectionError = 0xA,

    // Took too long for the tonearm to move down to its destination.
    VerticalNegativeDirectionError = 0xB,

    // Took too long for the tonearm to move clockwise to its destination.
    HorizontalClockwiseDirectionError = 0xC,

    // Took too long for the tonearm to move counterclockwise to its destination.
    HorizontalCounterclockwiseDirectionError = 0xD,

    // The movement succeeded.
    Success = 1,

    // No status was set yet.
    None = 0
};

#endif