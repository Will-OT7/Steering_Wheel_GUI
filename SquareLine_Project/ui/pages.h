#pragma once

#include <Arduino.h>

/* =====================================================
   Page system API
   ===================================================== */

/* Initialize page system (call in setup) */
void pages_init();

/* Call every loop (handles button input + navigation) */
void pages_update();

/* Manually load a screen by index */
void load_screen(int index, lv_scr_load_anim_t anim);

/* Get current active screen index */
int get_current_screen();