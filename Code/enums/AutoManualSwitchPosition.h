#ifndef AUTOMANUALSWITCHPOSITION_H
#define AUTOMANUALSWITCHPOSITION_H

// Each position that the Auto/Manual power switch could be in.
enum AutoManualSwitchPosition : uint8_t {

    // The Auto/Manual switch is set to "Manual"
    Manual = 0,

    // The Auto/Manual switch is set to "Automatic"
    Automatic = 1
};

#endif