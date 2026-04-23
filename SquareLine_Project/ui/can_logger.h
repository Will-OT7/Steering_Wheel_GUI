#ifndef CAN_LOGGER_H
#define CAN_LOGGER_H

#include <Arduino.h>

typedef enum {
    CAN_LOGGER_OFFLINE = 0,
    CAN_LOGGER_ONLINE,
    CAN_LOGGER_ERROR
} can_logger_state_t;

bool can_logger_init(uint8_t sd_cs_pin);
void can_logger_task(void);
void can_logger_log_frame(uint32_t id, const uint8_t *data, uint8_t len);
void can_logger_log_decoded(uint16_t speed_kph_x10,
                            uint8_t throttle_pct,
                            uint8_t soc_pct,
                            uint8_t vehicle_state,
                            uint8_t brake_pct,
                            uint8_t torque_req_pct,
                            uint8_t warning_flags,
                            uint8_t fault_flags,
                            uint8_t sequence_rx100,
                            uint8_t sequence_rx102);
bool can_logger_ready(void);
can_logger_state_t can_logger_state(void);

#endif
