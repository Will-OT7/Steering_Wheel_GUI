#include "inputs.h"

// ============================================================
//  PRIVATE STATE
// ============================================================

// ---- Matrix ------------------------------------------------
static const uint8_t _rowPins[MATRIX_ROWS] = { ROW0_PIN, ROW1_PIN, ROW2_PIN };
static const uint8_t _colPins[MATRIX_COLS] = { COL0_PIN, COL1_PIN };

static ButtonState  _buttons[MATRIX_ROWS * MATRIX_COLS];

// Debounce: track raw state and timestamp per button
static bool         _rawState[MATRIX_ROWS * MATRIX_COLS];
static uint32_t     _debounceTimer[MATRIX_ROWS * MATRIX_COLS];
static bool         _confirmedState[MATRIX_ROWS * MATRIX_COLS];

static uint32_t     _lastScanTime = 0;

// ---- Encoders ----------------------------------------------
// Quadrature lookup table: previous 2-bit state + current 2-bit state → delta
// Index = (prevAB << 2) | currAB
static const int8_t _quadTable[16] = {
//  00  01  10  11   ← current AB
     0, -1, +1,  0,  // prev AB = 00
    +1,  0,  0, -1,  // prev AB = 01
    -1,  0,  0, +1,  // prev AB = 10
     0, +1, -1,  0   // prev AB = 11
};

static EncoderState _encoders[2];

static const uint8_t _encAPins[2] = { ENC1_A_PIN, ENC2_A_PIN };
static const uint8_t _encBPins[2] = { ENC1_B_PIN, ENC2_B_PIN };

static uint8_t _encPrevAB[2] = { 0, 0 };   // last known AB state per encoder

// ============================================================
//  PRIVATE HELPERS
// ============================================================

static void _scanMatrix(void) {
    uint32_t now = millis();

    // Only drive one row LOW at a time; columns are read as inputs with pull-ups
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {

        // Drive current row LOW, all others HIGH (released)
        for (uint8_t rr = 0; rr < MATRIX_ROWS; rr++) {
            pinMode(_rowPins[rr], OUTPUT);
            digitalWrite(_rowPins[rr], (rr == r) ? LOW : HIGH);
        }

        // Small settling delay (give signals time to stabilise)
        delayMicroseconds(10);

        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            uint8_t idx      = r * MATRIX_COLS + c;
            bool    reading  = (digitalRead(_colPins[c]) == LOW);  // LOW = pressed (pull-up)

            // ---- Debounce ----
            if (reading != _rawState[idx]) {
                // State changed – restart debounce timer
                _rawState[idx]      = reading;
                _debounceTimer[idx] = now;
            }

            if ((now - _debounceTimer[idx]) >= DEBOUNCE_MS) {
                bool prev = _confirmedState[idx];
                _confirmedState[idx] = reading;

                _buttons[idx].pressed      = reading;
                _buttons[idx].justPressed  = (!prev &&  reading);
                _buttons[idx].justReleased = ( prev && !reading);
            }
        }
    }

    // Release all rows (set HIGH) when done
    for (uint8_t rr = 0; rr < MATRIX_ROWS; rr++) {
        digitalWrite(_rowPins[rr], HIGH);
    }
}

static void _readEncoders(void) {
    for (uint8_t i = 0; i < 2; i++) {
        uint8_t a   = digitalRead(_encAPins[i]) ? 1 : 0;
        uint8_t b   = digitalRead(_encBPins[i]) ? 1 : 0;
        uint8_t ab  = (a << 1) | b;

        uint8_t tableIdx = (_encPrevAB[i] << 2) | ab;
        int8_t  step     = _quadTable[tableIdx];

        _encoders[i].position += step;
        _encoders[i].delta    += step;

        _encPrevAB[i] = ab;
    }
}

// ============================================================
//  PUBLIC API IMPLEMENTATION
// ============================================================

void inputs_init(void) {
    // ---- Matrix pins ----
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        pinMode(_rowPins[r], OUTPUT);
        digitalWrite(_rowPins[r], HIGH);  // idle HIGH
    }
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        pinMode(_colPins[c], INPUT_PULLUP);
    }

    // ---- Encoder pins ----
    for (uint8_t i = 0; i < 2; i++) {
        pinMode(_encAPins[i], INPUT_PULLUP);
        pinMode(_encBPins[i], INPUT_PULLUP);

        // Capture initial AB state so the first read doesn't glitch
        uint8_t a    = digitalRead(_encAPins[i]) ? 1 : 0;
        uint8_t b    = digitalRead(_encBPins[i]) ? 1 : 0;
        _encPrevAB[i] = (a << 1) | b;

        _encoders[i].position = 0;
        _encoders[i].delta    = 0;
    }

    // ---- Clear button state ----
    for (uint8_t idx = 0; idx < MATRIX_ROWS * MATRIX_COLS; idx++) {
        _rawState[idx]       = false;
        _confirmedState[idx] = false;
        _debounceTimer[idx]  = 0;
        _buttons[idx]        = { false, false, false };
    }
}

void inputs_update(void) {
    uint32_t now = millis();

    // ---- Reset one-shot flags from last cycle ----
    for (uint8_t idx = 0; idx < MATRIX_ROWS * MATRIX_COLS; idx++) {
        _buttons[idx].justPressed  = false;
        _buttons[idx].justReleased = false;
    }

    // ---- Reset encoder delta ----
    for (uint8_t i = 0; i < 2; i++) {
        _encoders[i].delta = 0;
    }

    // ---- Scan matrix at throttled rate ----
    if ((now - _lastScanTime) >= MATRIX_SCAN_MS) {
        _lastScanTime = now;
        _scanMatrix();
    }

    // ---- Read encoders every cycle (needs fast polling) ----
    _readEncoders();
}

const ButtonState* inputs_getButton(uint8_t index) {
    if (index >= MATRIX_ROWS * MATRIX_COLS) return NULL;
    return &_buttons[index];
}

const EncoderState* inputs_getEncoder(uint8_t index) {
    if (index >= 2) return NULL;
    return &_encoders[index];
}
