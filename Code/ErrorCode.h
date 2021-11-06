#ifndef ERRORCODE_H
#define ERRORCODE_H

// Various errors that the turntable could encounter during operation
enum ErrorCode : uint8_t {

    // Took to long to hit the vertical lower (home) limit switch or hit the wrong limit switch
    VerticalHomeError,

    // Took to long to hit the vertical upper (pickup) limit switch or hit the wrong limit switch
    VerticalPickupError,

    // Took to long to home the horizontal axis
    HorizontalHomeError,

    // Took too long to find the play position sensor
    PlayError,

    // The movement succeeded.
    Success
};

#endif