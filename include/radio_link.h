#pragma once
#include <RadioLib.h>
#include "lab_config.h"
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
volatile bool radioEvent = false;
bool radioReady = false;
void IRAM_ATTR onRadioEvent() { radioEvent = true; } // No print/display/SPI in ISR.
bool initRadio() {
  if (!RADIO_ENABLED) { Serial.println("Radio disabled for GPS activity."); return false; }
  int state = radio.begin(RADIO_FREQUENCY_MHZ, RADIO_BANDWIDTH_KHZ,
    RADIO_SPREADING_FACTOR, RADIO_CODING_RATE, RADIO_SYNC_WORD,
    RADIO_POWER_DBM, RADIO_PREAMBLE, RADIO_TCXO_V);
  if (state == RADIOLIB_ERR_NONE) state = radio.setCRC(2);
  Serial.printf("Radio init code=%d, channel=%.2f MHz\n",state,RADIO_FREQUENCY_MHZ);
  if (state != RADIOLIB_ERR_NONE) return false;
  radio.setDio1Action(onRadioEvent);
  radioReady = true;
  return true;
}
