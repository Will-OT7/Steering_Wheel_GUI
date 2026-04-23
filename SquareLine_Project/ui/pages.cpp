#include <Arduino.h>
#include <lvgl.h>
#include "config.h"
#include "pages.h"
#include "vehicle_can.h"
#include "inputs.h"

extern "C" {
    #include "ui.h"
}

/* =====================================================
   SquareLine screens
   ===================================================== */
static lv_obj_t *screens[4] = { nullptr, nullptr, nullptr, nullptr };


/* =====================================================
   State
   ===================================================== */
static int current_screen = 0;


/* Debounce */
static bool last_next = HIGH;
static bool last_prev = HIGH;
static unsigned long last_press_time = 0;
static const unsigned long debounce_ms = 150;

static unsigned long last_page_change_time = 0;
static const unsigned long auto_scroll_ms = 5000;

static const uint8_t TEST_MODE_BTN_A = 1;
static const uint8_t TEST_MODE_BTN_B = 4;

static const uint32_t TEST_MODE_HOLD_MS = 1500;


/* =====================================================
   Public API
   ===================================================== */
int get_current_screen()
{
    return current_screen;
}

/* =====================================================
   Screen loader
   ===================================================== */
void load_screen(int index, lv_scr_load_anim_t anim)
{
    if (index < 0 || index >= NUM_SCREENS) return;
    if (screens[index] == nullptr) return;

    current_screen = index;
    last_page_change_time = millis();

    lv_scr_load_anim(
        screens[current_screen],
        anim,
        450,
        0,
        false
    );
}


/* =====================================================
   Init
   ===================================================== */
void pages_init()
{

    screens[0] = ui_MainScreen;
    screens[1] = ui_TireScreen;
    screens[2] = ui_SimpleSpeedScreen;
    screens[3] = ui_ErrorScreen;

    current_screen = 0;
    lv_scr_load(screens[current_screen]);
    last_page_change_time = millis();
}


// ============================================================
//  handle_inputs()
//  Called every loop() — reads buttons + encoders, fires CAN TX.
//
//  Button mapping:
//    BTN 0 → CMD_NEXT_PAGE  — advances GUI screen forward
//    BTN 1 → CMD_PREV_PAGE  — advances GUI screen backward
//    BTN 2 → CMD_BTN_GENERIC (val=2) — Teensy prints only, no action
//    BTN 3 → CMD_BTN_GENERIC (val=3) — Teensy prints only, no action
//    BTN 4 → CMD_BTN_GENERIC (val=4) — Teensy prints only, no action
//    BTN 5 → CMD_BTN_GENERIC (val=5) — Teensy prints only, no action
//
//  Encoder mapping:
//    ENC 0 (Rotary1) → CMD_ENC_EVENT_CMD, source=0 — Teensy prints delta only
//    ENC 1 (Rotary2) → CMD_ENC_EVENT_CMD, source=1 — Teensy prints delta only
// ============================================================
void handle_inputs()
{
    const ButtonState *combo_a = inputs_getButton(TEST_MODE_BTN_A);
    const ButtonState *combo_b = inputs_getButton(TEST_MODE_BTN_B);
    bool combo_pressed = combo_a && combo_b && combo_a->pressed && combo_b->pressed;

    // ---- Buttons --------------------------------------------
    for (uint8_t i = 0; i < 6; i++) {
        const ButtonState *b = inputs_getButton(i);
        if (!b || !b->justPressed) continue;
        if (combo_pressed && (i == TEST_MODE_BTN_A || i == TEST_MODE_BTN_B)) {continue;}

        Serial.print("Button pressed: ");
        Serial.println(i);

        if (i == 1) {
            // BTN 1— next screen
            canSendWheelCommand(CMD_NEXT_PAGE, 1, 0x01, 0x01);
            current_screen = (current_screen + 1) % NUM_SCREENS;
            load_screen(current_screen, LV_SCR_LOAD_ANIM_FADE_ON);

        } else if (i == 4) {
            // BTN 4 — previous screen
            canSendWheelCommand(CMD_PREV_PAGE, 1, 0x01, 0x01);
            current_screen = (current_screen - 1 + NUM_SCREENS) % NUM_SCREENS;
            load_screen(current_screen, LV_SCR_LOAD_ANIM_FADE_ON);

        } else {
            // BTN 2-5 — generic passthrough, val carries the button index
            canSendWheelCommand(CMD_BTN_GENERIC, (int8_t)i, 0x01, 0x01);
        }
    }

    // ---- Encoders -------------------------------------------
    for (uint8_t i = 0; i < 2; i++) {
        const EncoderState *e = inputs_getEncoder(i);
        if (!e || e->delta == 0) continue;

        int8_t clampedDelta = (int8_t)constrain(e->delta, -128, 127);

        // source field carries the encoder index (0 or 1)
        // val carries the signed delta for this cycle
        canSendWheelCommand(CMD_ENC_EVENT_CMD, clampedDelta, (uint8_t)i, 0x03);
    }
}

static bool last_test_button = HIGH;
static unsigned long last_test_toggle_time = 0;
static const unsigned long test_toggle_debounce_ms = 250;

static void handle_auto_scroll()
{
    if (!is_test_mode()) return;

    unsigned long now = millis();
    if (now - last_page_change_time >= auto_scroll_ms) {
        int next_screen = (current_screen + 1) % NUM_SCREENS;
        load_screen(next_screen, LV_SCR_LOAD_ANIM_FADE_ON);
    }
}


static bool test_mode_combo_active = false;
static bool test_mode_combo_fired = false;
static uint32_t test_mode_combo_start = 0;

static void handle_test_mode_combo()
{
    const ButtonState *a = inputs_getButton(TEST_MODE_BTN_A);
    const ButtonState *b = inputs_getButton(TEST_MODE_BTN_B);

    if (!a || !b) return;

    bool both_pressed = a->pressed && b->pressed;
    uint32_t now = millis();

    if (both_pressed) {
        if (!test_mode_combo_active) {
            test_mode_combo_active = true;
            test_mode_combo_start = now;
            test_mode_combo_fired = false;
        }

        if (!test_mode_combo_fired &&
            (now - test_mode_combo_start >= TEST_MODE_HOLD_MS)) {
            toggle_test_mode();
            current_screen = 0;
            load_screen(current_screen, LV_SCR_LOAD_ANIM_FADE_ON);  
            test_mode_combo_fired = true;
        }
    } else {
        test_mode_combo_active = false;
        test_mode_combo_fired = false;
    }
}

/* =====================================================
   Main update (call from loop)
   ===================================================== */
void pages_update()
{
    handle_test_mode_combo();
    handle_inputs();
    handle_auto_scroll();
}