#include "colordemo.h"

#include <Arduino.h>
#include <math.h>
#include "keysmet.h"

// ---------------------------------------------------------------------------
// Color / gradient / pattern demo.
//
// Grid layout (1-indexed key -> position):
//
//     key:   1  2  3  4  5        col:  0 1 2 3 4   (row 0)
//            6  7  8  9 10               0 1 2 3 4   (row 1)
//
// col(i) = (i-1) % 5   in [0..4]
// row(i) = (i-1) / 5   in [0..1]
//
// Each key is a "bank" (synth-style patch bank): pressing key N selects
// bank N and paints the whole board with its first state; pressing the same
// key again advances that bank to its next state (brightness level,
// position, etc). Each bank remembers its own step. Every bank sticks to a
// single hue or a narrow analogous hue range so colors never clash.
// ---------------------------------------------------------------------------

namespace {

    constexpr int KEY_LO = 1;
    constexpr int KEY_HI = 10;
    constexpr int COLS = 5;
    constexpr int ROWS = 2;
    constexpr int BANK_COUNT = KEY_HI - KEY_LO + 1; // 10

    constexpr float SAT = 1.0f;

    inline int colOf(int key) { return (key - KEY_LO) % COLS; }
    inline int rowOf(int key) { return (key - KEY_LO) / COLS; }

    inline float xOf(int key) { return colOf(key) / float(COLS - 1); }
    inline float yOf(int key) { return ROWS > 1 ? rowOf(key) / float(ROWS - 1) : 0.0f; }

    inline float frac(float x) { return x - floorf(x); }
    inline float clamp01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }
    inline float fmax2(float a, float b) { return a > b ? a : b; }

    inline float now() { return (float)ksm::getTime(); }

    // Brightness levels shared by most banks so state-cycling also sweeps
    // intensity, not just hue/pattern.
    const float BRIGHT_LEVELS[] = { 0.15f, 0.4f, 0.7f, 1.0f };
    const int BRIGHT_LEVEL_COUNT = sizeof(BRIGHT_LEVELS) / sizeof(BRIGHT_LEVELS[0]);

    inline void off(int key) { ksm::setHSV(key, 0.0f, 0.0f, 0.0f); }

    // -----------------------------------------------------------------
    // Bank painters. Each takes the bank's current step and paints all
    // 10 keys (some intentionally leave keys dark).
    // -----------------------------------------------------------------

    // Bank 1: "full colors" — every look that washes the whole board evenly
    // (solids, gradients, duotone, saturation ramp) lives here as one long
    // step sequence, so all the flat/uniform colors are grouped on a single
    // key. Every other bank is free to focus on shape/pattern instead.
    void bankFullColors(int step) {
        int sub = step / BRIGHT_LEVEL_COUNT;   // which color look
        float v = BRIGHT_LEVELS[step % BRIGHT_LEVEL_COUNT];
        switch (sub) {
            case 0: // solid amber
                for (int i = KEY_LO; i <= KEY_HI; ++i) ksm::setHSV(i, 0.08f, SAT, v);
                break;
            case 1: // solid azure
                for (int i = KEY_LO; i <= KEY_HI; ++i) ksm::setHSV(i, 0.55f, SAT, v);
                break;
            case 2: // solid magenta
                for (int i = KEY_LO; i <= KEY_HI; ++i) ksm::setHSV(i, 0.86f, SAT, v);
                break;
            case 3: { // sunset gradient (red -> orange -> amber)
                const float base = 0.03f, spread = 0.07f;
                for (int i = KEY_LO; i <= KEY_HI; ++i)
                    ksm::setHSV(i, frac(base + (xOf(i) - 0.5f) * 2.0f * spread), SAT, v);
                break;
            }
            case 4: { // ocean gradient (teal -> cyan -> azure)
                const float base = 0.52f, spread = 0.07f;
                for (int i = KEY_LO; i <= KEY_HI; ++i)
                    ksm::setHSV(i, frac(base + (xOf(i) - 0.5f) * 2.0f * spread), SAT, v);
                break;
            }
            case 5: { // blue -> pink duotone
                const float hueA = 0.58f, hueB = 0.92f;
                float d = hueB - hueA;
                if (d > 0.5f) d -= 1.0f;
                if (d < -0.5f) d += 1.0f;
                for (int i = KEY_LO; i <= KEY_HI; ++i)
                    ksm::setHSV(i, frac(hueA + d * xOf(i)), SAT, v);
                break;
            }
            case 6: // coral saturation ramp (pastel -> vivid)
                for (int i = KEY_LO; i <= KEY_HI; ++i)
                    ksm::setHSV(i, 0.98f, xOf(i), fmax2(v, 0.3f));
                break;
        }
    }
    const int FULL_COLORS_LOOKS = 7;
    const int FULL_COLORS_STEPS = FULL_COLORS_LOOKS * BRIGHT_LEVEL_COUNT;

    // Bank 2: sparse row spot — only one row lit (single hue), the other
    // dark. step alternates row and sweeps brightness.
    void bankRowSpot(int step) {
        int litRow = step % ROWS;
        float v = BRIGHT_LEVELS[(step / ROWS) % BRIGHT_LEVEL_COUNT];
        const float hue = 0.62f;
        for (int i = KEY_LO; i <= KEY_HI; ++i) {
            if (rowOf(i) == litRow)
                ksm::setHSV(i, hue, SAT, v);
            else
                off(i);
        }
    }

    // Bank 3: sparse column spot — only one column lit, walks across the
    // board as the step advances.
    void bankColumnSpot(int step) {
        int litCol = step % COLS;
        float v = BRIGHT_LEVELS[(step / COLS) % BRIGHT_LEVEL_COUNT];
        const float hue = 0.09f;
        for (int i = KEY_LO; i <= KEY_HI; ++i) {
            if (colOf(i) == litCol)
                ksm::setHSV(i, hue, SAT, v);
            else
                off(i);
        }
    }

    // Bank 4: checkerboard — alternating lit/dark keys, single hue, two
    // brightness tiers that swap on odd steps.
    void bankCheckerboard(int step) {
        const float hue = 0.75f; // violet
        bool swap = (step % 2) != 0;
        float v = BRIGHT_LEVELS[(step / 2) % BRIGHT_LEVEL_COUNT];
        for (int i = KEY_LO; i <= KEY_HI; ++i) {
            bool odd = ((colOf(i) + rowOf(i)) & 1) != 0;
            if (odd != swap)
                ksm::setHSV(i, hue, SAT, v);
            else
                off(i);
        }
    }

    // Bank 5: single lit key that walks across the board left-to-right,
    // top row then bottom row, like a chaser/marquee light.
    void bankChaser(int step) {
        int idx = step % (COLS * ROWS);
        const float hue = 0.33f; // green
        for (int i = KEY_LO; i <= KEY_HI; ++i) {
            int pos = rowOf(i) * COLS + colOf(i);
            if (pos == idx)
                ksm::setHSV(i, hue, SAT, 1.0f);
            else
                off(i);
        }
    }

    // Bank 6: corners + center cross alternate — two complementary sparse
    // shapes (corners lit / cross lit), single hue, brightness sweep.
    void bankCornersCross(int step) {
        const float hue = 0.02f; // red
        bool showCorners = (step % 2) == 0;
        float v = BRIGHT_LEVELS[(step / 2) % BRIGHT_LEVEL_COUNT];
        for (int i = KEY_LO; i <= KEY_HI; ++i) {
            int c = colOf(i), r = rowOf(i);
            bool isCorner = (c == 0 || c == COLS - 1);
            bool isCenterCol = (c == COLS / 2);
            bool lit = showCorners ? isCorner : isCenterCol;
            (void)r;
            if (lit)
                ksm::setHSV(i, hue, SAT, v);
            else
                off(i);
        }
    }

    // Bank 7: expanding diamond — keys light up in order of distance from
    // board center, growing outward each step (single hue).
    void bankExpandingDiamond(int step) {
        const float hue = 0.60f; // blue
        const float cx = (COLS - 1) / 2.0f;
        const float cy = (ROWS - 1) / 2.0f;
        int ring = step % 4; // max Manhattan distance on this small grid is ~3
        for (int i = KEY_LO; i <= KEY_HI; ++i) {
            float d = fabsf(colOf(i) - cx) + fabsf(rowOf(i) - cy);
            if ((int)(d + 0.5f) <= ring)
                ksm::setHSV(i, hue, SAT, 1.0f);
            else
                off(i);
        }
    }

    // Small deterministic hash, used to derive "random" values from a seed
    // without needing a stateful RNG (so a given step always reproduces the
    // same look, and pressing the key again reshuffles via a new seed).
    inline uint32_t hash32(uint32_t x) {
        x ^= x >> 16; x *= 0x7feb352du;
        x ^= x >> 15; x *= 0x846ca68bu;
        x ^= x >> 16;
        return x;
    }

    // Bank 8: random — alternates each step between two looks: a random
    // scattered sparkle pattern (random subset of keys lit, single random
    // hue for that roll) and a random solid uniform color across the whole
    // board. Every roll picks its own hue so successive presses cycle
    // through different but always-coherent (single-hue) looks.
    void bankRandom(int step) {
        uint32_t seed = hash32((uint32_t)step * 2654435761u + 1u);
        float hue = (seed & 0xFFFFu) / 65536.0f; // random hue for this roll

        bool uniform = (step % 2) != 0;
        if (uniform) {
            // Random uniform color: whole board one flat random hue/brightness.
            float v = BRIGHT_LEVELS[1 + (seed >> 16) % (BRIGHT_LEVEL_COUNT - 1)];
            for (int i = KEY_LO; i <= KEY_HI; ++i)
                ksm::setHSV(i, hue, SAT, v);
        } else {
            // Random scattered pattern: random subset of keys lit at that
            // roll's hue, each with its own brightness jitter.
            for (int i = KEY_LO; i <= KEY_HI; ++i) {
                uint32_t h = hash32(seed + (uint32_t)i * 374761393u);
                bool lit = (h & 1u) != 0;
                if (lit) {
                    float v = BRIGHT_LEVELS[1 + (h >> 1) % (BRIGHT_LEVEL_COUNT - 1)];
                    ksm::setHSV(i, hue, SAT, v);
                } else {
                    off(i);
                }
            }
        }
    }
    const int RANDOM_STEPS = 16; // 8 pattern rolls + 8 uniform-color rolls

    // Bank 9: random twinkle — animated, each key independently flickers on
    // and off at its own pseudo-random phase/rate (single hue), like a
    // scattered field of twinkling lights rather than a moving shape.
    void bankRandomTwinkle(int /*step*/) {
        const float hue = 0.70f; // indigo
        float t = now();
        for (int i = KEY_LO; i <= KEY_HI; ++i) {
            uint32_t h = (uint32_t)i * 2246822519u;
            h ^= h >> 15; h *= 2654435761u; h ^= h >> 13;
            float phaseOffset = (h % 1000) / 1000.0f;
            float rate = 0.6f + (h % 700) / 1000.0f; // ~0.6-1.3 Hz-ish
            float b = 0.5f + 0.5f * sinf((t * rate + phaseOffset) * 2.0f * (float)M_PI);
            float v = 0.05f + b * b * 0.9f; // ease so it snaps rather than smoothly breathes
            ksm::setHSV(i, hue, SAT, v);
        }
    }

    // Bank 10: animated — tonal breathing pulse (step 0) or a traveling
    // brightness wave across a single lit row (step 1), single-hue violet.
    void bankAnimatedViolet(int step) {
        const float hue = 0.75f;
        if (step % 2 == 0) {
            float b = 0.5f + 0.5f * sinf(now() * 2.0f);
            float v = 0.12f + clamp01(b) * 0.83f;
            for (int i = KEY_LO; i <= KEY_HI; ++i)
                ksm::setHSV(i, hue, SAT, v);
        } else {
            float t = now();
            for (int i = KEY_LO; i <= KEY_HI; ++i) {
                float phase = xOf(i) * 2.0f - t * 1.5f;
                float v = 0.1f + (0.5f + 0.5f * cosf(phase * (float)M_PI)) * 0.8f;
                ksm::setHSV(i, hue, SAT, v);
            }
        }
    }

    struct Bank {
        void (*paint)(int step);
        int steps;
        const char* name;
    };

    // One bank per key (index 0 == key 1, ... index 9 == key 10). Key 1 is
    // the single "full colors" bank; keys 2-10 favor shape/pattern variety.
    const Bank BANKS[BANK_COUNT] = {
        { bankFullColors,        FULL_COLORS_STEPS,         "full-colors"        }, // key 1
        { bankRowSpot,           ROWS * BRIGHT_LEVEL_COUNT, "row-spot"           }, // key 2
        { bankColumnSpot,        COLS * BRIGHT_LEVEL_COUNT, "column-spot"        }, // key 3
        { bankCheckerboard,      2 * BRIGHT_LEVEL_COUNT,    "checkerboard"       }, // key 4
        { bankChaser,            COLS * ROWS,               "chaser"             }, // key 5
        { bankCornersCross,      2 * BRIGHT_LEVEL_COUNT,    "corners-cross"      }, // key 6
        { bankExpandingDiamond,  4,                         "expanding-diamond"  }, // key 7
        { bankRandom,            RANDOM_STEPS,              "random"             }, // key 8
        { bankRandomTwinkle,     1,                         "random-twinkle"     }, // key 9
        { bankAnimatedViolet,    2,                         "animated-violet"    }, // key 10
    };

    int gActiveBank = 0;              // 0-indexed bank == key - 1
    int gStep[BANK_COUNT] = { 0 };    // per-bank remembered step

} // namespace

namespace colordemo {

    void init() {
        gActiveBank = 0;
        for (int b = 0; b < BANK_COUNT; ++b)
            gStep[b] = 0;
    }

    void update() {
        for (int key = KEY_LO; key <= KEY_HI; ++key) {
            if (ksm::press(key)) {
                int bank = key - KEY_LO;
                if (bank == gActiveBank) {
                    // Same key pressed again: advance this bank's step.
                    gStep[bank] = (gStep[bank] + 1) % BANKS[bank].steps;
                } else {
                    // Switching banks: select it fresh at step 0.
                    gActiveBank = bank;
                    gStep[bank] = 0;
                }
                break; // one key change per frame
            }
        }

        BANKS[gActiveBank].paint(gStep[gActiveBank]);
    }

    int bankCount() { return BANK_COUNT; }

    const char* bankName(int bank) {
        if (bank < 0 || bank >= BANK_COUNT) return "?";
        return BANKS[bank].name;
    }

} // namespace colordemo
