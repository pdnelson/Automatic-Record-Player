#ifndef HORIZONTALCLUTCHPOSITION_H
#define HORIZONTALCLUTCHPOSITION_H

// Each position that the horizontal clutch could move to.
enum HorizontalClutchPosition : bool {

    // Engage the horizontal clutch gears, so that automatic horizontal movement
    // may occur.
    Engage = true,

    // Disengage the horizontal clutch gears, so that manual horizontal movement
    // may occur.
    Disengage = false
};

#endif