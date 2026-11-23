#pragma once
// Edit these values only as instructed in the lab.
constexpr char NODE_ID[] = "G01";         // Your assigned group ID, no comma.
constexpr bool RAW_NMEA = false;          // true: short serial diagnostic capture.
constexpr bool RADIO_ENABLED = false;     // Enable only for the LoRa activity.
constexpr float RADIO_FREQUENCY_MHZ = 433.92; // 433 MHz-band candidate; instructor confirms channel.
constexpr float RADIO_BANDWIDTH_KHZ = 125.0;
constexpr uint8_t RADIO_SPREADING_FACTOR = 7;
constexpr uint8_t RADIO_CODING_RATE = 5;   // RadioLib value 5 means 4/5.
constexpr uint8_t RADIO_SYNC_WORD = 0x12;
constexpr int8_t RADIO_POWER_DBM = 2;
constexpr uint16_t RADIO_PREAMBLE = 8;
constexpr float RADIO_TCXO_V = 1.6;        // Supplied RadioLib example default.
constexpr uint32_t REPORT_INTERVAL_MS = 1000;
constexpr uint32_t TX_INTERVAL_MS = 10000;
constexpr uint32_t NO_DATA_AFTER_MS = 2000;
constexpr uint32_t FRESH_FIX_MAX_AGE_MS = 3000;
