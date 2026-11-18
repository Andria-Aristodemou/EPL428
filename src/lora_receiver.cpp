#include <Arduino.h>
#include "lab_board.h"
#include "lab_config.h"
#include "radio_link.h"
uint32_t rxCount=0;
bool boardReady=false;
void drawReceiver(const String& payload,float rssi,float snr) {
  if (!u8g2) return;
  u8g2->clearBuffer(); u8g2->setFont(u8g2_font_6x10_tf);
  u8g2->setCursor(0,10); u8g2->print("RX packets "); u8g2->print(rxCount);
  u8g2->setCursor(0,20); u8g2->print("RSSI "); u8g2->print(rssi,1); u8g2->print(" dBm");
  u8g2->setCursor(0,30); u8g2->print("SNR "); u8g2->print(snr,1); u8g2->print(" dB");
  // Preview only: use the serial log for the complete CSV payload.
  u8g2->setCursor(0,42); u8g2->print(payload.substring(0,21));
  u8g2->setCursor(0,52); u8g2->print(payload.substring(21,42));
  u8g2->setCursor(0,63); u8g2->print("Full packet: Serial");
  u8g2->sendBuffer();
}
void setup() {
  boardReady=initLabBoard();
  if (boardReady && initRadio()) {
    int state=radio.startReceive();
    Serial.printf("RX start code=%d\n",state);
    if(state!=RADIOLIB_ERR_NONE) radioReady=false;
  }
}
void loop() {
  // Receiver does not require its own GPS fix. Drain its unused GPS UART.
  while(Serial1.available()) Serial1.read();
  if (!boardReady || !radioReady || !radioEvent) return;
  radioEvent=false;
  String packet;
  int state=radio.readData(packet);
  if(state==RADIOLIB_ERR_NONE) {
    ++rxCount;
    float rssi=radio.getRSSI(), snr=radio.getSNR();

   // Human-readable output
Serial.printf("RX #%lu RSSI=%.1f dBm SNR=%.1f dB payload=%s\n",
    (unsigned long)rxCount,rssi,snr,packet.c_str());

// Machine-readable output for Python logger
Serial.printf("LORA,%lu,%.1f,%.1f,%s\n",
    (unsigned long)rxCount,
    rssi,
    snr,
    packet.c_str()
);
    drawReceiver(packet,rssi,snr);
  } else Serial.printf("RX read error=%d (payload rejected)\n",state);
  state=radio.startReceive();
  if(state!=RADIOLIB_ERR_NONE) { Serial.printf("RX restart error=%d\n",state); radioReady=false; }
}
