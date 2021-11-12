#ifndef AUTOMANUALSWITCHPOSITION_H
#define AUTOMANUALSWITCHPOSITION_H

// Various errors that the turntable could encounter during operation
enum AutoManualSwitchPosition : uint8_t {

    // The Auto/Manual switch is set to "Manual"
    Manual = 0,

    // The Auto/Manual switch is set to "Automatic"
    Automatic = 1
};

#endif