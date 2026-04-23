#pragma once
#include <Arduino.h>

// =====================================================
// CAN IDs  (RX)
// =====================================================
static const uint32_t CAN_ID_MAIN_TELEMETRY = 0x100;
static const uint32_t CAN_ID_WARNINGS       = 0x102;

// =====================================================
// CAN IDs  (TX)
// =====================================================
// All wheel input events sent on 0x201
// Frame layout:
//   [0] command ID  (CMD_* constants below)
//   [1] value       (signed int8: button index, encoder delta, etc.)
//   [2] source      (0x01 = button, 0x02 = encoder, or encoder index for ENC_EVENT)
//   [3] event type  (0x01 = press, 0x02 = release, 0x03 = turn)
//   [4-5] reserved  (0x00)
//   [6] sequence counter
//   [7] reserved    (0x00)
static const uint32_t CAN_ID_BTN_EVENT      = 0x201;
static const uint32_t CAN_ID_ENC_EVENT      = 0x202;  // kept for reference; not used in new protocol

// =====================================================
// Command IDs  (TX → Teensy)
// =====================================================
// Navigation (BTN 0 / BTN 1) — Teensy logs only, no state change
static const uint8_t CMD_NEXT_PAGE     = 0x01;
static const uint8_t CMD_PREV_PAGE     = 0x02;

// Encoder actions — Teensy logs only, no state change
static const uint8_t CMD_SPEED_UP      = 0x03;  // kept for reference
static const uint8_t CMD_SPEED_DOWN    = 0x04;  // kept for reference
static const uint8_t CMD_SOC_UP        = 0x05;  // kept for reference
static const uint8_t CMD_SOC_DOWN      = 0x06;  // kept for reference

// Warning / fault control
static const uint8_t CMD_ACK_WARNING   = 0x07;
static const uint8_t CMD_RESET_FAULT   = 0x08;
static const uint8_t CMD_STATE_ADVANCE = 0x0F;
static const uint8_t CMD_FAULT_TRIGGER = 0x10;

// Generic passthrough — Teensy prints only, no state change
// val = button index (2-5) for BTN_GENERIC
// val = signed delta, source = encoder index (0/1) for ENC_EVENT
static const uint8_t CMD_BTN_GENERIC   = 0x20;
static const uint8_t CMD_ENC_EVENT_CMD = 0x21;

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
// Decoded CAN variables  (RX)
// =====================================================
extern uint16_t speed_kph_x10;
extern uint8_t  throttle_pct;
extern uint8_t  soc_pct;
extern uint8_t  vehicle_state;
extern uint8_t  brake_pct;
extern uint8_t  acc_temp;
extern uint8_t  warning_flags;
extern uint8_t  fault_flags;

extern uint8_t  sequence_rx100;
extern uint8_t  sequence_rx102;

extern bool telemetry_received;
extern bool warnings_received;

// =====================================================
// API
// =====================================================
void vehicleCanSetup();
void vehicleCanTask();
void processCanFrame(uint32_t id, const uint8_t *data, uint8_t len);

/**
 * Send a unified wheel command frame on CAN_ID_BTN_EVENT (0x201).
 *
 * @param cmdId      CMD_* constant
 * @param val        signed value (button index, encoder delta, etc.)
 * @param source     0x01 = button, 0x02 = encoder (or encoder index for ENC_EVENT_CMD)
 * @param eventType  0x01 = press, 0x02 = release, 0x03 = turn
 */
void canSendWheelCommand(uint8_t cmdId, int8_t val, uint8_t source, uint8_t eventType);
