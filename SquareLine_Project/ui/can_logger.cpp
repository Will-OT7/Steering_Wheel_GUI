#include "can_logger.h"
#include "config.h"
#include <SPI.h>
#include <SD.h>

namespace {

constexpr uint32_t kFlushIntervalMs = 1000;
constexpr uint32_t kRetryIntervalMs = 3000;

File g_raw_log_file;
File g_decoded_log_file;
bool g_logger_ready = false;
can_logger_state_t g_logger_state = CAN_LOGGER_OFFLINE;
uint32_t g_last_flush_ms = 0;
uint32_t g_last_retry_ms = 0;
uint8_t g_sd_cs_pin = 0xFF;
bool g_sd_initialized = false;
char g_raw_filename[13] = {0};
char g_decoded_filename[13] = {0};

void close_logs()
{
    if (g_raw_log_file) {
        g_raw_log_file.flush();
        g_raw_log_file.close();
    }
    if (g_decoded_log_file) {
        g_decoded_log_file.flush();
        g_decoded_log_file.close();
    }
}

bool write_line(File &file, const String &line)
{
    if (!file) {
        return false;
    }

    const size_t expected = line.length() + 2;
    const size_t written = file.println(line);
    return written >= expected;
}

void mark_logger_error(const char *message)
{
    if (message) {
        Serial.print("[CAN LOGGER] ERROR: ");
        Serial.println(message);
    }

    close_logs();
    g_logger_ready = false;
    g_sd_initialized = false;
    g_logger_state = CAN_LOGGER_ERROR;
}

bool ensure_headers()
{
    if (!g_raw_log_file || !g_decoded_log_file) {
        return false;
    }

    if (g_raw_log_file.size() == 0) {
        if (!write_line(g_raw_log_file, "millis,can_id,len,b0,b1,b2,b3,b4,b5,b6,b7")) {
            return false;
        }
    }

    if (g_decoded_log_file.size() == 0) {
        if (!write_line(g_decoded_log_file,
                        "millis,speed_kph_x10,speed_kph,throttle_pct,soc_pct,vehicle_state,brake_pct,torque_req_pct,warning_flags,fault_flags,seq100,seq102")) {
            return false;
        }
    }

    g_raw_log_file.flush();
    g_decoded_log_file.flush();
    return true;
}

bool file_exists(const char *path)
{
    File existing = SD.open(path, FILE_READ);
    if (existing) {
        existing.close();
        return true;
    }
    return false;
}

bool build_next_filename(const char *prefix, char *out, size_t out_size)
{
    if (!prefix || !out || out_size < 13) {
        return false;
    }

    for (uint16_t i = 1; i <= 999; ++i) {
        snprintf(out, out_size, "/%s%03u.CSV", prefix, i);
        if (!file_exists(out)) {
            return true;
        }
    }

    return false;
}

bool try_mount_sd()
{
    Serial.print("[CAN LOGGER] Trying SD mount on CS pin ");
    Serial.println(g_sd_cs_pin);

    if (!SD.begin(g_sd_cs_pin)) {
        Serial.println("[CAN LOGGER] SD.begin() failed");
        g_logger_ready = false;
        g_sd_initialized = false;
        g_logger_state = CAN_LOGGER_OFFLINE;
        return false;
    }

    Serial.println("[CAN LOGGER] SD.begin() OK");
    g_sd_initialized = true;

    if (!build_next_filename("RAW", g_raw_filename, sizeof(g_raw_filename)) ||
        !build_next_filename("DEC", g_decoded_filename, sizeof(g_decoded_filename))) {
        mark_logger_error("No free log filename available");
        return false;
    }

    g_raw_log_file = SD.open(g_raw_filename, FILE_WRITE);
    g_decoded_log_file = SD.open(g_decoded_filename, FILE_WRITE);

    if (!g_raw_log_file || !g_decoded_log_file) {
        mark_logger_error("Failed to open CAN log files");
        return false;
    }

    if (!ensure_headers()) {
        mark_logger_error("Failed to write CAN log headers");
        return false;
    }

    g_logger_ready = true;
    g_logger_state = CAN_LOGGER_ONLINE;
    Serial.print("[CAN LOGGER] RAW: ");
    Serial.println(g_raw_filename);
    Serial.print("[CAN LOGGER] DECODED: ");
    Serial.println(g_decoded_filename);
    Serial.println("[CAN LOGGER] Logger ONLINE");
    return true;
}

} // namespace

bool can_logger_init(uint8_t sd_cs_pin)
{
    g_sd_cs_pin = sd_cs_pin;
    g_last_retry_ms = millis();
    Serial.println("[CAN LOGGER] Init start");
    return try_mount_sd();
}

void can_logger_task(void)
{
    if (!g_logger_ready) {
        if ((millis() - g_last_retry_ms) >= kRetryIntervalMs) {
            g_last_retry_ms = millis();
            Serial.println("[CAN LOGGER] Logger offline, retrying SD mount...");
            try_mount_sd();
        }
        return;
    }

    if ((millis() - g_last_flush_ms) >= kFlushIntervalMs) {
        if (!g_raw_log_file || !g_decoded_log_file) {
            mark_logger_error("CAN logger lost file handle");
            return;
        }

        g_raw_log_file.flush();
        g_decoded_log_file.flush();
        g_last_flush_ms = millis();
        Serial.println("[CAN LOGGER] Flush OK");
    }
}

void can_logger_log_frame(uint32_t id, const uint8_t *data, uint8_t len)
{
    if (!g_logger_ready || !g_raw_log_file || !data) {
        return;
    }

    String line;
    line.reserve(64);
    line += millis();
    line += ",0x";
    line += String(id, HEX);
    line += ",";
    line += len;

    for (uint8_t i = 0; i < 8; ++i) {
        line += ",";
        if (i < len) {
            if (data[i] < 16) {
                line += "0";
            }
            line += String(data[i], HEX);
        }
    }

    if (!write_line(g_raw_log_file, line)) {
        mark_logger_error("Raw CAN log write failed");
    }
}

void can_logger_log_decoded(uint16_t speed_kph_x10,
                            uint8_t throttle_pct,
                            uint8_t soc_pct,
                            uint8_t vehicle_state,
                            uint8_t brake_pct,
                            uint8_t torque_req_pct,
                            uint8_t warning_flags,
                            uint8_t fault_flags,
                            uint8_t sequence_rx100,
                            uint8_t sequence_rx102)
{
    if (!g_logger_ready || !g_decoded_log_file) {
        return;
    }

    String line;
    line.reserve(96);
    line += millis();
    line += ",";
    line += speed_kph_x10;
    line += ",";
    line += String(speed_kph_x10 / 10.0f, 1);
    line += ",";
    line += throttle_pct;
    line += ",";
    line += soc_pct;
    line += ",";
    line += vehicle_state;
    line += ",";
    line += brake_pct;
    line += ",";
    line += torque_req_pct;
    line += ",0x";
    line += String(warning_flags, HEX);
    line += ",0x";
    line += String(fault_flags, HEX);
    line += ",";
    line += sequence_rx100;
    line += ",";
    line += sequence_rx102;

    if (!write_line(g_decoded_log_file, line)) {
        mark_logger_error("Decoded CAN log write failed");
    }
}

bool can_logger_ready(void)
{
    return g_logger_ready;
}

can_logger_state_t can_logger_state(void)
{
    return g_logger_state;
}
