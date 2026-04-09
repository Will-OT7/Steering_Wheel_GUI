#pragma once
#include <Arduino.h>
// =====================================================
// CAN IDs
// =====================================================
static const uint32_t CAN_ID_MAIN_TELEMETRY = 0x100;
static const uint32_t CAN_ID_WARNINGS       = 0x102;
static const uint32_t CAN_ID_WHEEL_COMMAND  = 0x201;

// =====================================================
// EV states
// =====================================================
static const uint8_t VEH_STATE_WAIT_PRECHARGE = 0;
static const uint8_t VEH_STATE_READY_TO_GO    = 1;
static const uint8_t VEH_STATE_IDLE           = 2;
static const uint8_t VEH_STATE_FAULT          = 3;

// =====================================================
// Warning / fault bits
// =====================================================
static const uint8_t WARN_LOW_SOC      = (1 << 0);
static const uint8_t WARN_CAN_TIMEOUT  = (1 << 4);
static const uint8_t WARN_GENERAL      = (1 << 6);
static const uint8_t FAULT_GENERAL     = (1 << 6);

// =====================================================
// Decoded CAN variables
// =====================================================
extern uint16_t speed_kph_x10;
extern uint8_t throttle_pct;
extern uint8_t soc_pct;
extern uint8_t vehicle_state;
extern uint8_t brake_pct;
extern uint8_t torque_req_pct;
extern uint8_t warning_flags;
extern uint8_t fault_flags;

extern uint8_t sequence_rx100;
extern uint8_t sequence_rx102;

extern bool telemetry_received;
extern bool warnings_received;

// =====================================================
// API
// =====================================================
void vehicleCanSetup();
void vehicleCanTask();
void processCanFrame(uint32_t id, const uint8_t *data, uint8_t len);