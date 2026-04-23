#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>

// ============================================================
//  PIN DEFINITIONS  (from schematic)
// ============================================================

// --- Button Matrix (3 rows × 2 columns = 6 buttons) ---------
//  Rows are OUTPUTS (driven LOW one at a time)
//  Columns are INPUTS (read with internal pull-up)
//
//  Feather net → Arduino pin
//   D0 → row 0   (buttons 1, 7  in schematic)
//   D1 → row 1   (buttons 3, 9  in schematic)
//   D4 → row 2   (buttons 5, 11 in schematic)
//   D5 → col 0   (buttons 2, 4, 6  in schematic)
//   D6 → col 1   (buttons 8, 10, 12 in schematic)
//
//  Resulting logical button index [row][col]:
//    [0][0]=BTN1  [0][1]=BTN2
//    [1][0]=BTN3  [1][1]=BTN4
//    [2][0]=BTN5  [2][1]=BTN6

#define MATRIX_ROWS  3
#define MATRIX_COLS  2

// Change these pin numbers to match your board's actual pin numbering
#define ROW0_PIN     0   // D0
#define ROW1_PIN     1   // D1
#define ROW2_PIN     4   // D4
#define COL0_PIN     5   // D5
#define COL1_PIN     6   // D6

// --- Rotary Encoders ----------------------------------------
//  S1 (Rotary1): A=R1A, B=R1B  →  Feather D10/PA20, D11/PA21
//  S2 (Rotary2): A=R2A, B=R2B  →  Feather D12/PA22, D13/PA23
//  COM of both encoders → GND  (use internal pull-ups on A/B)

#define ENC1_A_PIN   23  // R1A → Feather pin 23
#define ENC1_B_PIN   22  // R1B → Feather pin 22
#define ENC2_A_PIN   21  // R2A → Feather pin 21
#define ENC2_B_PIN   20  // R2B → Feather pin 20

// ============================================================
//  DEBOUNCE SETTINGS
// ============================================================
#define DEBOUNCE_MS      20   // button debounce window (milliseconds)
#define MATRIX_SCAN_MS    5   // how often to scan the matrix (milliseconds)

// ============================================================
//  PUBLIC DATA TYPES
// ============================================================

// One entry per button in the matrix (6 total, indexed 0-5)
typedef struct {
    bool pressed;       // true while button is held down
    bool justPressed;   // true for exactly one scan cycle after press
    bool justReleased;  // true for exactly one scan cycle after release
} ButtonState;

// One entry per rotary encoder (2 total)
typedef struct {
    int32_t position;   // cumulative tick count (+ = CW, - = CCW)
    int16_t delta;      // ticks changed since last inputs_update() call
} EncoderState;

// ============================================================
//  PUBLIC API
// ============================================================

/**
 * Call once in setup() – configures all GPIO and initialises state.
 */
void inputs_init(void);

/**
 * Call every loop() iteration – scans matrix, reads encoders,
 * updates all state structs, clears justPressed/justReleased flags
 * from the previous cycle.
 */
void inputs_update(void);

/**
 * Access the state of a matrix button.
 * @param index  0-5  (row-major: btn0=[r0,c0] … btn5=[r2,c1])
 * @return pointer to the ButtonState, or NULL if index out of range.
 */
const ButtonState* inputs_getButton(uint8_t index);

/**
 * Access the state of a rotary encoder.
 * @param index  0 = Rotary1 (S1),  1 = Rotary2 (S2)
 * @return pointer to the EncoderState, or NULL if index out of range.
 */
const EncoderState* inputs_getEncoder(uint8_t index);

#endif // INPUTS_H
