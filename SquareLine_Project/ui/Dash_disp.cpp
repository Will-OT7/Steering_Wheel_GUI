#include "Dash_disp.h"
#include "vehicle_can.h"
#include "ui.h"
#include <stdio.h>
#include <math.h>
#include "config.h"
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

float get_sim_tire_pressure(){
    float t = millis() / 1000.0;
    return 30 + 5 * sin(t);
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
    float speed_mph, speed_kph, temperature, battery, fl_tire_pressure, fr_tire_pressure, rl_tire_pressure, rr_tire_pressure;
    int speed, gas, brake;
    if(!is_test_mode()){
        speed_kph = speed_kph_x10 / 10.0;
        speed_mph = speed_kph * 0.621371;
        temperature = acc_temp;
        speed = (int)speed_mph;
        gas = (int)throttle_pct;
        brake = (int)brake_pct;
        battery = soc_pct;
        fl_tire_pressure = get_sim_tire_pressure();
        fr_tire_pressure = get_sim_tire_pressure(); 
        rl_tire_pressure = get_sim_tire_pressure();
        rr_tire_pressure = get_sim_tire_pressure();
    }
    else if(is_test_mode()){
        speed_kph = get_sim_speed();
        speed_mph = get_sim_speed();
        temperature = get_sim_temp();
        fl_tire_pressure = get_sim_tire_pressure();
        fr_tire_pressure = get_sim_tire_pressure(); 
        rl_tire_pressure = get_sim_tire_pressure();
        rr_tire_pressure = get_sim_tire_pressure();
        speed = (int)speed_mph;
        gas = get_sim_gas();
        brake = get_sim_brake();
        battery = get_sim_battery();
    }

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
    lv_label_set_text(ui_SpeedVal1, buf);
    lv_label_set_text(ui_SpeedVal2, buf);
    
    //Update Speed Bar
    lv_bar_set_value(ui_SpeedBar, (int)(speed/1.86), LV_ANIM_OFF);
    lv_bar_set_value(ui_SpeedBar1, (int)(speed/1.86), LV_ANIM_OFF);
    lv_bar_set_value(ui_SpeedBar2, (int)(speed/1.86), LV_ANIM_OFF);

    //Update Gas Bar
    lv_bar_set_value(ui_GasBar, gas, LV_ANIM_OFF);

    //Update Brake Bar
    lv_bar_set_value(ui_BrakeBar, brake, LV_ANIM_OFF);

    // Update Tire Pressure labels
    sprintf(buf, "%.1f PSI", fl_tire_pressure);
    lv_label_set_text(ui_FLTirePressureVal, buf);  
    sprintf(buf, "%.1f PSI", fr_tire_pressure);
    lv_label_set_text(ui_FRTirePressureVal, buf);
    sprintf(buf, "%.1f PSI", rl_tire_pressure);
    lv_label_set_text(ui_BLTirePressureVal, buf);
    sprintf(buf, "%.1f PSI", rr_tire_pressure);
    lv_label_set_text(ui_BRTirePressureVal, buf);

    // Update Tire Pressure arcs
    lv_arc_set_value(ui_FLTirePressureArc, fl_tire_pressure);
    lv_arc_set_value(ui_FRTirePressureArc, fr_tire_pressure);
    lv_arc_set_value(ui_BLTirePressureArc, rl_tire_pressure);
    lv_arc_set_value(ui_BRTirePressureArc, rr_tire_pressure);
}

