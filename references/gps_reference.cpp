#include <Arduino.h>
#include <TinyGPS++.h>
#include "lab_board.h"
#include "lab_config.h"
#include "radio_link.h"

TinyGPSPlus gps;
uint32_t rxBytes=0, lastByteMs=0, lastReportMs=0, lastTxMs=0;
uint32_t sequence=0, txStartedMs=0;
bool boardReady=false, txBusy=false, parsingPaused=false;
String txPacket;

void feedGps() {
  while (Serial1.available() > 0) {
    const int c = Serial1.read();
    ++rxBytes;
    lastByteMs = millis();
    if (RAW_NMEA) Serial.write(static_cast<uint8_t>(c));
    if (!parsingPaused) {
      gps.encode(static_cast<char>(c));
    }
  }
}

const char* fixState() {
  if (rxBytes==0 || uint32_t(millis()-lastByteMs)>NO_DATA_AFTER_MS) return "NO_DATA";
  if (!gps.location.isValid()) return "NO_FIX";
  if (gps.location.age()>FRESH_FIX_MAX_AGE_MS) return "STALE";
  return "FRESH";
}
bool freshFix() { return strcmp(fixState(),"FRESH")==0; }
String latitudeText() { return freshFix()?String(gps.location.lat(),6):String("--"); }
String longitudeText() { return freshFix()?String(gps.location.lng(),6):String("--"); }
String satelliteText() {
  return gps.satellites.isValid() && gps.satellites.age()<5000 ? String(gps.satellites.value()):String("--");
}
String hdopText() {
  return gps.hdop.isValid() && gps.hdop.age()<5000 ? String(gps.hdop.hdop(),1):String("--");
}
String utcText() {
  if (!gps.time.isValid() || gps.time.age()>5000) return "--";
  char b[10]; snprintf(b,sizeof(b),"%02d:%02d:%02d",gps.time.hour(),gps.time.minute(),gps.time.second());
  return b;
}
String dateText() {
  if (!gps.date.isValid() || gps.date.age()>5000) return "--";
  char b[12]; snprintf(b,sizeof(b),"%04d-%02d-%02d",gps.date.year(),gps.date.month(),gps.date.day());
  return b;
}
void drawGps() {
  if (!u8g2) return;
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_6x10_tf);
  u8g2->setCursor(0,10); u8g2->print("GPS "); u8g2->print(fixState());
  u8g2->setCursor(0,20); u8g2->print("Lat "); u8g2->print(latitudeText());
  u8g2->setCursor(0,30); u8g2->print("Lon "); u8g2->print(longitudeText());
  u8g2->setCursor(0,40); u8g2->print("S "); u8g2->print(satelliteText());
  u8g2->print(" HDOP "); u8g2->print(hdopText());
  u8g2->setCursor(0,50); u8g2->print(dateText());
  u8g2->setCursor(0,60); u8g2->print("UTC "); u8g2->print(utcText());
  u8g2->sendBuffer();
}
void reportGps() {
  Serial.printf("ms=%lu bytes=%lu parsed=%lu checksum_ok=%lu state=%s",
    (unsigned long)millis(),(unsigned long)rxBytes,(unsigned long)gps.charsProcessed(),
    (unsigned long)gps.passedChecksum(),fixState());
  Serial.print(" age_ms=");
  if (gps.location.isValid()) Serial.print(gps.location.age()); else Serial.print("--");
  Serial.printf(" lat=%s lon=%s sats=%s hdop=%s utc=%s date=%s\n",
    latitudeText().c_str(),longitudeText().c_str(),satelliteText().c_str(),hdopText().c_str(),
    utcText().c_str(),dateText().c_str());
}
String makePacket() {
  // 9 CSV fields; blanks for unavailable numeric fields, never fake zero coordinates.
  const bool fresh = freshFix(); // One freshness decision for this entire packet.
  String p=String("GPS1,")+NODE_ID+","+String(sequence)+","+(fresh?"1":"0")+",";
  if (fresh) p+=String(gps.location.lat(),6);
  p+=",";
  if (fresh) p+=String(gps.location.lng(),6);
  p+=",";
  if (satelliteText()!="--") p+=satelliteText();
  p+=",";
  if (gps.location.isValid()) p+=String(gps.location.age());
  p+=",";
  if (utcText()!="--") p+=utcText();
  return p;
}
void serviceTransmit() {
  if (!radioReady) return;
  if (txBusy && radioEvent) {
    radioEvent=false;
    int result=radio.finishTransmit();
    txBusy=false;
    Serial.printf("TX_DONE code=%d (local radio only; no acknowledgement)\n",result);
  }
  if (txBusy && uint32_t(millis()-txStartedMs)>5000) {
    radio.finishTransmit();
    txBusy=false; radioEvent=false;
    Serial.println("TX timeout; GPS parsing continues.");
  }
  if (!txBusy && uint32_t(millis()-lastTxMs)>=TX_INTERVAL_MS) {
    lastTxMs=millis();
    ++sequence;
    txPacket=makePacket();
    radioEvent=false;
    txStartedMs=millis();
    int state=radio.startTransmit(txPacket);
    txBusy=(state==RADIOLIB_ERR_NONE);
    Serial.printf("TX_START code=%d payload=%s\n",state,txPacket.c_str());
  }
}
void setup() {
  boardReady=initLabBoard();
  if (boardReady) initRadio();
  Serial.println("EPL428: p pauses parsing, r resumes; UART bytes still drained.");
}
void loop() {
  if (!boardReady) { delay(10); return; }
  while (Serial.available()) {
    char command=Serial.read();
    if (command=='p') { parsingPaused=true; Serial.println("TEST: parser paused"); }
    if (command=='r') { parsingPaused=false; Serial.println("TEST: parser resumed"); }
  }
  feedGps(); // Must run frequently, not just once per display refresh.
  serviceTransmit();
  if (uint32_t(millis()-lastReportMs)>=REPORT_INTERVAL_MS) {
    lastReportMs=millis();
    if (!RAW_NMEA) reportGps();
    drawGps();
  }
}
