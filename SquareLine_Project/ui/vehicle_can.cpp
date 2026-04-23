#include "vehicle_can.h"
#include "can_logger.h"
#include <CANSAME5x.h>

// Feather M4 CAN Express CAN controller
static CANSAME5x CANbus;

// =====================================================
// Decoded CAN variables  (RX)
// =====================================================
uint16_t speed_kph_x10 = 0;
uint8_t  throttle_pct  = 0;
uint8_t  soc_pct       = 0;
uint8_t  vehicle_state = VEH_STATE_WAIT_PRECHARGE;
uint8_t  brake_pct     = 0;
uint8_t  torque_req_pct = 0;
uint8_t  acc_temp = 0;
uint8_t  warning_flags = 0;
uint8_t  fault_flags   = 0;

uint8_t  sequence_rx100 = 0;
uint8_t  sequence_rx102 = 0;

bool telemetry_received = false;
bool warnings_received  = false;

// =====================================================
// Setup
// =====================================================
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

// =====================================================
// RX – parse an incoming frame
// =====================================================
void processCanFrame(uint32_t id, const uint8_t *data, uint8_t len)
{
    if (!data) return;

    if (id == CAN_ID_MAIN_TELEMETRY && len >= 8) {
        speed_kph_x10   = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        throttle_pct    = data[2];
        soc_pct         = data[3];
        vehicle_state   = data[4];
        brake_pct       = data[5];
        acc_temp        = data[6];
        sequence_rx100  = data[7];
        telemetry_received = true;
        can_logger_log_decoded(speed_kph_x10,
                               throttle_pct,
                               soc_pct,
                               vehicle_state,
                               brake_pct,
                               acc_temp,
                               warning_flags,
                               fault_flags,
                               sequence_rx100,
                               sequence_rx102);
        return;

    }

    if (id == CAN_ID_WARNINGS && len >= 8) {
        warning_flags   = data[0];
        fault_flags     = data[1];
        sequence_rx102  = data[7];
        warnings_received = true;
        return;
    }
}

// =====================================================
// RX – poll CAN bus and dispatch frames
// =====================================================
void vehicleCanTask()
{
    int packetSize = CANbus.parsePacket();

    while (packetSize) {
        if (!CANbus.packetRtr()) {
            uint8_t  data[8];
            uint8_t  len = 0;
            uint32_t id  = CANbus.packetId();

            while (CANbus.available() && len < sizeof(data)) {
                data[len++] = (uint8_t)CANbus.read();
            }
            can_logger_log_frame(id, data, len);
            processCanFrame(id, data, len);
        } else {
            while (CANbus.available()) {
                (void)CANbus.read();
            }
        }

        packetSize = CANbus.parsePacket();
    }
}

// =====================================================
// TX – unified wheel command  (CAN ID 0x201)
// =====================================================
// Frame layout:
//   [0] command ID
//   [1] value (signed int8)
//   [2] source (0x01=button, 0x02=encoder, or encoder index)
//   [3] event type (0x01=press, 0x02=release, 0x03=turn)
//   [4-5] reserved (0x00)
//   [6] sequence counter
//   [7] reserved (0x00)
// =====================================================
static uint8_t _wheelCmdSeq = 0;

void canSendWheelCommand(uint8_t cmdId, int8_t val, uint8_t source, uint8_t eventType)
{
    uint8_t frame[8] = { 0 };
    frame[0] = cmdId;
    frame[1] = (uint8_t)val;
    frame[2] = source;
    frame[3] = eventType;
    frame[4] = 0x00;
    frame[5] = 0x00;
    frame[6] = _wheelCmdSeq++;
    frame[7] = 0x00;

    CANbus.beginPacket(CAN_ID_BTN_EVENT);  // 0x201
    CANbus.write(frame, 8);
    CANbus.endPacket();

    Serial.print("[CAN TX 0x201] CMD=0x");
    Serial.print(cmdId, HEX);
    Serial.print(" val=");
    Serial.print(val);
    Serial.print(" src=0x");
    Serial.print(source, HEX);
    Serial.print(" evt=0x");
    Serial.print(eventType, HEX);
    Serial.print(" seq=");
    Serial.println(frame[6]);
}
