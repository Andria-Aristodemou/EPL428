EPL428 - GPS on LilyGO with LoRa telemetry
90-minute student lab - English - confirmed 433 MHz hardware band

START HERE
Use the accompanying EPL428_GPS_LoRa_Student_Lab.pdf.
Extract this entire folder and open it as a PlatformIO project.
The instructor should prepare PlatformIO Espressif32 6.12.0 before class.
This uses Arduino-ESP32 2.0.17 and the supplied t-beams3-supreme board definition.

TARGET HARDWARE
T-Beam S3 Supreme V3.0 configuration from the supplied Display_gps.zip:
ESP32-S3, L76K at 9600 baud, SH1106 128x64 OLED, AXP2101 and SX1262.
Confirm the actual PCB revision, GNSS part, SX1262 radio and matching antennas.
This is not a generic sketch for every LilyGO or every 433 MHz radio.

FILES
src/gps_student.cpp       Student starter. Complete TODO 1 and TODO 2.
src/lora_receiver.cpp     Supplied companion receiver.
include/lab_config.h     Group ID, diagnostic mode, timing and radio parameters.
include/lab_board.h      Adapted board initialization; retains original PMU sequence.
include/utilities.h      Pin definitions supplied in Display_gps.zip.
include/radio_link.h     Shared SX1262 initialization and minimal interrupt flag.
reference/               Completed GPS reference for tutor use / later comparison.
evidence/                CSV headers for the lab observation records.
lib/                     Four complete course library copies with their notices.
boards/                  Supplied Supreme PlatformIO board definition.

DEPENDENCIES
Library versions retained from the course repository, rather than silently updated:
RadioLib 6.0.0; U8g2 2.34.17; XPowersLib 0.1.6; TinyGPSPlus course copy.
TinyGPSPlus manifests disagree: library.properties says 1.0.0,
library.json says 1.0.2. This kit preserves the actual supplied source.
Their original library documentation and licence notices are included.
Do not install a second copy of the same library into this project.

BUILD / UPLOAD
  pio run -e gps_student
  pio run -e lora_receiver
Upload only to the intended board's port:
  pio run -e gps_student -t upload --upload-port YOUR_PORT
  pio device monitor --port YOUR_PORT --baud 115200
Replace YOUR_PORT with the actual port; do not copy a port from another machine.
Close a serial monitor before uploading if it is holding the port.
The two source files are filtered by environment; only the selected one is built.

REFERENCE
For a tutor demo, make a separate project copy and replace src/gps_student.cpp
with reference/gps_reference.cpp, retaining the filename gps_student.cpp.
The reference finishes both student TODOs. It uses the same configuration.

RADIO IS OFF INITIALLY
RADIO_ENABLED=false permits the GPS activity with no transmission.
Before the LoRa activity, attach the appropriate antenna and have the instructor
confirm the channel and classroom radio conditions. Then set RADIO_ENABLED=true
in each board's project and upload the correct environment.
433.92 MHz is a candidate carrier within the confirmed hardware band;
it is not a blanket statement about permitted operation in every location.
Starting parameters: LoRa, 125 kHz, SF7, CR4/5, sync 0x12, preamble 8,
CRC enabled, 2 dBm and a 10-second transmit interval.
The TCXO setting is the original example default, 1.6 V; verify the board revision.
The receiver needs the same settings and does not need its own GPS fix.

CONTROLS / DIAGNOSTICS
RAW_NMEA=true prints incoming GNSS bytes for a short diagnostic capture.
Restore false before taking decoded logs or enabling LoRa.
In Serial Monitor, send p to pause the parser or r to resume.
The UART continues draining while paused, so bytes increase but parsed does not.
After a real fix, the pause should eventually show STALE and hide coordinates.
Before any fix, the same pause remains NO_FIX; do not claim an observed stale fix.

PACKET
Nine CSV fields:
version,node,sequence,fresh,latitude,longitude,satellites,location_age_ms,utc
Example ONLY: GPS1,G07,12,1,35.000000,33.000000,8,250,09:15:23
fresh=0 means both coordinate fields are empty. Sequence increases per attempt.
TX_DONE confirms only local completion. A matching RX log proves reception.
No acknowledgement, LoRaWAN, encryption, backend or persistent tracking is added.

VALIDATION LIMIT
PlatformIO configuration and library discovery succeeded for the starter,
reference and receiver on the preparation host. Windows prevented the existing
ESP32 compiler from starting (Illegal System DLL Relocation), before C/C++
compilation. The binaries were NOT successfully built or linked here.
API names/signatures were inspected against the bundled library versions.
No firmware was uploaded; GNSS, OLED and radio were NOT tested on hardware.
The instructor must build and bench-test the actual pair before classroom use.

PROVENANCE
Based on the user-supplied LAB_5_25.pdf and Display_gps.zip, plus the four
library folders and board definition from the supplied LilyGo-LoRa-Series tree.
Original source files were not modified. lab_board.h retains the supplied PMU
sequence, removes unrelated SD startup diagnostics and the unused gps1 parser,
and provides guarded full-buffer display initialization.
New teaching code adds observable parser TODOs, freshness rules, timed reports,
asynchronous GPS telemetry, a receiver and evidence templates.
Board manufacturer: https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series
TinyGPSPlus: https://github.com/mikalhart/TinyGPSPlus
U8g2: https://github.com/olikraus/u8g2
RadioLib: https://github.com/jgromes/RadioLib
XPowersLib: https://github.com/lewisxhe/XPowersLib
