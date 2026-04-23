SET(SOURCES ui_MainScreen.c
    ui_TireScreen.c
    ui_SimpleSpeedScreen.c
    ui_ErrorScreen.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_conical_gradient_2_png.c)

add_library(ui ${SOURCES})
