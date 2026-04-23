#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include "vehicle_can.h"
#include "inputs.h"
#include "config.h"
#include "pages.h"
#include "can_logger.h"
#include "Dash_disp.h"


// ============================================================
//  Display config
// ============================================================
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);
bool g_test_mode = false;

#if LV_USE_LOG != 0
void my_print(const char *buf) {
    Serial.printf(buf);
    Serial.flush();
}
#endif

// ============================================================
//  Display flush callback
// ============================================================
#line 32 "C:\\Users\\mzees\\Documents\\ECD619\\Screen_Project_Code\\SquareLine_Project\\ui\\ui.ino"
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
#line 48 "C:\\Users\\mzees\\Documents\\ECD619\\Screen_Project_Code\\SquareLine_Project\\ui\\ui.ino"
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
#line 78 "C:\\Users\\mzees\\Documents\\ECD619\\Screen_Project_Code\\SquareLine_Project\\ui\\ui.ino"
void handle_inputs();
#line 119 "C:\\Users\\mzees\\Documents\\ECD619\\Screen_Project_Code\\SquareLine_Project\\ui\\ui.ino"
void setup();
#line 166 "C:\\Users\\mzees\\Documents\\ECD619\\Screen_Project_Code\\SquareLine_Project\\ui\\ui.ino"
void loop();
#line 32 "C:\\Users\\mzees\\Documents\\ECD619\\Screen_Project_Code\\SquareLine_Project\\ui\\ui.ino"
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

// ============================================================
//  Touchpad callback  (unused / placeholder)
// ============================================================
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX = 0, touchY = 0;
    bool touched = false;

    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state   = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

// ============================================================
//  setup()
// ============================================================
void setup()
{
    Serial.begin(115200);

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(LVGL_Arduino);

    // Initialise inputs (matrix + encoders)
    inputs_init();

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    tft.begin();
    tft.setRotation(2);
    tft.invertDisplay(true);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = screenWidth;
    disp_drv.ver_res  = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();

    can_logger_init(A4); // SD card CS pin
    pages_init();
    dashboard_init();
    vehicleCanSetup();     // CAN bus init (RX + TX)

    Serial.println("Setup done");
}

// ============================================================
//  loop()
// ============================================================
void loop()
{
    inputs_update();       // scan matrix, read encoders
    pages_update();       // handle page navigation + test mode combo
    can_logger_task();    // handle CAN log flushing + SD reconnects
    vehicleCanTask();      // receive CAN frames, update globals
    dashboard_update();    // push decoded values to LVGL widgets
    

    lv_timer_handler();    // LVGL internal work
    delay(5);
}

