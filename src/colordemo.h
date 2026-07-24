#pragma once

// Color / gradient / pattern demo for photoshoots.
//
// Layout of the 10 physical keys (1-indexed, matching ksm::press/setHSV):
//
//     1  2  3  4  5      row 0
//     6  7  8  9 10      row 1
//
// So it is a 2-row x 5-column grid. Gradients run along the 5 columns
// (key 1 and key 6 share column 0, key 5 and key 10 share column 4); going
// 1->10 linearly would look wrong, so all gradients use column/row position.
//
// Interaction: each of the 10 keys is a "bank" (like a synth patch bank).
// Pressing key N selects bank N and paints the whole board with that bank's
// first state. Pressing the SAME key again advances that bank to its next
// state (brightness level, position, etc); each bank remembers its own step
// independently. Every bank keeps its colors tonal/analogous so nothing
// clashes.
//
// Key 1 groups all the "full color" looks (solids, gradients, duotone,
// saturation ramps — anything that washes the whole board evenly) into one
// long step sequence. Keys 2-10 are dedicated to shape/pattern variety
// (spots, checkerboard, chaser, diamond, etc) rather than more flat colors.

namespace colordemo {
    // Call once from setup() (resets all banks to step 0, selects bank 0).
    void init();

    // Call every frame from loop(). Handles key-press bank selection /
    // step-cycling and repaints all 10 keys for the active bank+step.
    void update();

    // Total number of banks (should be 10, one per key).
    int bankCount();

    // Human-readable name of a bank (for logging).
    const char* bankName(int bank);
}
