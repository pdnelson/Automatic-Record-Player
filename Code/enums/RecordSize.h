#ifndef MOTORAXIS_H
#define MOTORAXIS_H

// The size of the record that the tonearm should move to the outer edge of.
enum RecordSize : uint8_t {

    // 7" record.
    Rec7Inch = 0,

    // 10" record.
    Rec10Inch = 1,

    // 12" record.
    Rec12Inch = 2
};

#endif