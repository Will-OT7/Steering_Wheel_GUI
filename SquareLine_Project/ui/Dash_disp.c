#include "Dash_disp.h"
#include "vehicle_can.h"
#include "ui.h"
#include <stdio.h>
#include <math.h>



void dashboard_init() {
    /*
    speed = 120;
    battery = 100;
    temperature = 50.0;
    */  
}

float get_sim_temp(){
    float t = millis() / 1000.0;
    return 50 + 10 * sin(t);
}

float get_sim_battery(){
    float t = millis() / 1000.0;
    return 50 + 25 * sin(t);
}

int get_sim_speed(){
    float t = millis() / 1000.0;
    float s = 100 + 20 * sin(t * 0.2);
    return (int)s;
}

int get_sim_gas(){
    float t = millis() / 1000.0;
    float s = 50 + 50 * sin(t);
    return (int)s;
}

int get_sim_brake(){
    float t = millis() / 1000.0;
    float s = 50 + 50 * sin(t + 3.14);
    return (int)s;
}


void dashboard_update() {
    char buf[16];
    
    float speed_kph = speed_kph_x10 / 10.0;
    float speed_mph = speed_kph * 0.621371;
    float temperature = get_sim_temp();
    int speed = (int)speed_mph;
    int gas = (int)throttle_pct;
    int brake = (int)brake_pct;
    float battery = soc_pct;

    // Update Accumulator temperature label
    sprintf(buf, "%.1f\u00B0F", temperature);
    lv_label_set_text(ui_AccTempValue, buf);
    // Update Accumulator Temperature arc
    lv_arc_set_value(ui_AccTempArc, temperature);

    
    // Update Battery label
    sprintf(buf, "%.1f%%", battery);
    lv_label_set_text(ui_BatteryVal, buf);
    // Update Battery arc
    lv_arc_set_value(ui_BatteryArc, battery);
    
    

    // Update speed label
    sprintf(buf, "%d", speed);
    lv_label_set_text(ui_SpeedVal, buf);
    //Update Speed Bar
    lv_bar_set_value(ui_SpeedBar, (int)(speed/1.86), LV_ANIM_OFF);

    //Update Gas Bar
    lv_bar_set_value(ui_GasBar, gas, LV_ANIM_OFF);

    //Update Brake Bar
    lv_bar_set_value(ui_BrakeBar, brake, LV_ANIM_OFF);


}

