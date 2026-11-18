#include <Arduino.h>
#include <TinyGPS++.h>
#include <WiFi.h>

#include "lab_board.h"
#include "lab_config.h"
#include "radio_link.h"
#include "wifi_config.h"
#include "dashboard.h"//The application interface
#include <WebServer.h>

#define RAW_NMEA false

WebServer server(80);

TinyGPSPlus gps;
uint32_t rxBytes=0, lastByteMs=0, lastReportMs=0, lastTxMs=0;
uint32_t sequence=0, txStartedMs=0;
bool boardReady=false, txBusy=false, parsingPaused=false;
String txPacket;

bool webServerStarted = false;



void feedGps() {
  while (Serial1.available() > 0) {
    const int c = Serial1.read();
    ++rxBytes;
    lastByteMs = millis();
    if (RAW_NMEA) Serial.write(static_cast<uint8_t>(c));
    if (!parsingPaused) {
      // TODO 1: Feed c into the SAME TinyGPSPlus object used by reportGps().
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




void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

void handleStatus() {
    String json = "{";

    json += "\"gps_status\":\"" + String(fixState()) + "\",";
    json += "\"latitude\":\"" + latitudeText() + "\",";
    json += "\"longitude\":\"" + longitudeText() + "\",";
    json += "\"satellites\":\"" + satelliteText() + "\",";
    json += "\"hdop\":\"" + hdopText() + "\",";
    json += "\"utc\":\"" + utcText() + "\",";
    json += "\"date\":\"" + dateText() + "\",";

    json += "\"gps_age_ms\":";
    if (gps.location.isValid()) {
        json += String(gps.location.age());
    } else {
        json += "null";
    }

    json += ",\"rx_bytes\":" + String(rxBytes);
    json += ",\"chars_processed\":" + String(gps.charsProcessed());
    json += ",\"checksum_ok\":" + String(gps.passedChecksum());

    json += ",\"wifi_connected\":";
    json += (WiFi.status() == WL_CONNECTED) ? "true" : "false";

    json += ",\"wifi_rssi\":";
    json += (WiFi.status() == WL_CONNECTED)
        ? String(WiFi.RSSI())
        : "null";

    json += ",\"radio_ready\":";
    json += radioReady ? "true" : "false";

    json += ",\"tx_busy\":";
    json += txBusy ? "true" : "false";

    json += ",\"sequence\":" + String(sequence);

    json += "}";

    server.send(200, "application/json", json);
}





void drawGps() {
  if (!u8g2) return;
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_6x10_tf);
  u8g2->setCursor(0, 10);
  u8g2->print("GPS "); u8g2->print(fixState());
  u8g2->setCursor(0, 20);
  u8g2->print("Lat "); u8g2->print(latitudeText());
  u8g2->setCursor(0, 30);
  u8g2->print("Lon "); u8g2->print(longitudeText());
  u8g2->setCursor(0, 40);
  u8g2->print("S "); u8g2->print(satelliteText());
  u8g2->print(" HDOP "); u8g2->print(hdopText());
  u8g2->setCursor(0, 50);
  u8g2->print(dateText());
  u8g2->setCursor(0, 60);
  u8g2->print("UTC "); u8g2->print(utcText());
  u8g2->sendBuffer();
}
void reportGps() {
  Serial.printf("ms=%lu bytes=%lu parsed=%lu checksum_ok=%lu state=%s",
    (unsigned long)millis(),(unsigned long)rxBytes,(unsigned long)gps.charsProcessed(),
    (unsigned long)gps.passedChecksum(),fixState());
  Serial.print(" age_ms=");
  if (gps.location.isValid()) Serial.print(gps.location.age()); else Serial.print("--");
 
 //This is gia na pianei tis sintetagmenes kai na apothikevei tis times se csv gia tis draft metriseis apo to gps.
  Serial.printf(" lat=%s lon=%s sats=%s hdop=%s utc=%s date=%s\n",
    latitudeText().c_str(),longitudeText().c_str(),satelliteText().c_str(),hdopText().c_str(),
    utcText().c_str(),dateText().c_str());

    Serial.printf("CSV,%lu,%lu,%lu,%lu,%s,%s,%s,%s,%s,%s\n",
    (unsigned long)millis(),
    (unsigned long)rxBytes,
    (unsigned long)gps.charsProcessed(),
    (unsigned long)gps.passedChecksum(),
    fixState(),
    latitudeText().c_str(),
    longitudeText().c_str(),
    satelliteText().c_str(),
    hdopText().c_str(),
    utcText().c_str()
);
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


//this is for the hotspot
void connectWiFi() {
    Serial.println("Connecting to WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 20000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}





void setup() {
    boardReady = initLabBoard();

    if (boardReady) {
        connectWiFi();
        initRadio();

        if (WiFi.status() == WL_CONNECTED) {
            server.on("/", handleRoot);
            server.on("/api/status", handleStatus);

            server.begin();
            webServerStarted = true;

            Serial.println("Web server started!");
            Serial.print("Dashboard URL: http://");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("Web server not started: WiFi unavailable.");
        }
    }

    Serial.println(
        "EPL428: p pauses parsing, r resumes; UART bytes still drained."
    );
}

void loop() {
  if (!boardReady) { delay(10); return; }

if (webServerStarted) {
    server.handleClient();
}

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

  server.handleClient();
}
