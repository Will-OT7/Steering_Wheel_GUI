#include "vehicle_can.h"
#include <CANSAME5x.h>

// Feather M4 CAN Express CAN controller
static CANSAME5x CANbus;

// =====================================================
// Decoded CAN variables
// =====================================================
uint16_t speed_kph_x10 = 0;
uint8_t throttle_pct   = 0;
uint8_t soc_pct        = 0;
uint8_t vehicle_state  = VEH_STATE_WAIT_PRECHARGE;
uint8_t brake_pct      = 0;
uint8_t torque_req_pct = 0;
uint8_t warning_flags  = 0;
uint8_t fault_flags    = 0;

uint8_t sequence_rx100 = 0;
uint8_t sequence_rx102 = 0;

bool telemetry_received = false;
bool warnings_received  = false;

void vehicleCanSetup()
{
    pinMode(PIN_CAN_STANDBY, OUTPUT);
    digitalWrite(PIN_CAN_STANDBY, false);

    pinMode(PIN_CAN_BOOSTEN, OUTPUT);
    digitalWrite(PIN_CAN_BOOSTEN, true);

    if (!CANbus.begin(500000)) {
        Serial.println("CAN init failed");
        while (1) {
            delay(10);
        }
    }

    Serial.println("CAN init OK");
}

void processCanFrame(uint32_t id, const uint8_t *data, uint8_t len)
{
    if (!data) return;

    if (id == CAN_ID_MAIN_TELEMETRY && len >= 8) {
        speed_kph_x10  = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        throttle_pct   = data[2];
        soc_pct        = data[3];
        vehicle_state  = data[4];
        brake_pct      = data[5];
        torque_req_pct = data[6];
        sequence_rx100 = data[7];
        telemetry_received = true;
        return;
    }

    if (id == CAN_ID_WARNINGS && len >= 8) {
        warning_flags  = data[0];
        fault_flags    = data[1];
        sequence_rx102 = data[7];
        warnings_received = true;
        return;
    }
}

void vehicleCanTask()
{
    int packetSize = CANbus.parsePacket();

    while (packetSize) {
        if (!CANbus.packetRtr()) {
            uint8_t data[8];
            uint8_t len = 0;
            uint32_t id = CANbus.packetId();

            while (CANbus.available() && len < sizeof(data)) {
                data[len++] = (uint8_t)CANbus.read();
            }

            processCanFrame(id, data, len);
        } else {
            while (CANbus.available()) {
                (void)CANbus.read();
            }
        }

        packetSize = CANbus.parsePacket();
    }
}