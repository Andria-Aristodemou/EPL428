
import serial
import csv
import threading
import os
from datetime import datetime

# CHANGE THESE TO YOUR ACTUAL COM PORTS
GPS_PORT = "COM3"
LORA_PORT = "COM6"

BAUD_RATE = 115200
CSV_FILE = "measurements.csv"

HEADERS = [
    "pc_timestamp", "source", "gps_millis", "rx_bytes",
    "chars_processed", "checksum_ok", "gps_status",
    "latitude", "longitude", "satellites", "hdop", "utc",
    "receiver_count", "rssi_dbm", "snr_db", "node_id",
    "sequence", "fix", "gps_age_ms", "payload_utc"
]

csv_lock = threading.Lock()


def save_row(row):
    if len(row) != len(HEADERS):
        print(f"[CSV] Invalid row length: {len(row)}")
        return

    with csv_lock:
        file_exists = (
            os.path.exists(CSV_FILE)
            and os.path.getsize(CSV_FILE) > 0
        )

        with open(CSV_FILE, "a", newline="", encoding="utf-8") as file:
            writer = csv.writer(file)

            if not file_exists:
                writer.writerow(HEADERS)

            writer.writerow(row)
            file.flush()


def read_gps():
    try:
        with serial.Serial(GPS_PORT, BAUD_RATE, timeout=1) as esp:
            print(f"[GPS] Connected to {GPS_PORT}")

            while True:
                line = esp.readline().decode(
                    "utf-8", errors="replace"
                ).strip()

                if not line.startswith("CSV,"):
                    continue

                values = line.split(",")

                if len(values) != 11:
                    print("[GPS] Invalid record:", line)
                    continue

                row = [
                    datetime.now().isoformat(timespec="milliseconds"),
                    "GPS"
                ] + values[1:] + [""] * 8

                save_row(row)
                print("[GPS] Measurement saved")

    except serial.SerialException as error:
        print("[GPS] Serial error:", error)


def read_lora():
    try:
        with serial.Serial(LORA_PORT, BAUD_RATE, timeout=1) as esp:
            print(f"[LORA] Connected to {LORA_PORT}")

            while True:
                line = esp.readline().decode(
                    "utf-8", errors="replace"
                ).strip()

                if not line.startswith("LORA,"):
                    continue

                values = line.split(",", 4)

                if len(values) != 5:
                    print("[LORA] Invalid record:", line)
                    continue

                receiver_count = values[1]
                rssi = values[2]
                snr = values[3]

                payload = values[4].split(",")

                if len(payload) != 9 or payload[0] != "GPS1":
                    print("[LORA] Invalid GPS payload:", values[4])
                    continue

                row = [
                    datetime.now().isoformat(timespec="milliseconds"),
                    "LORA",
                    "", "", "", "",
                    "", 
                    payload[4],  # latitude
                    payload[5],  # longitude
                    payload[6],  # satellites
                    "", "",      # HDOP and UTC unavailable in payload
                    receiver_count,
                    rssi,
                    snr,
                    payload[1],  # node ID
                    payload[2],  # sequence
                    payload[3],  # fix
                    payload[7],  # GPS age
                    payload[8]   # GPS UTC
                ]

                save_row(row)
                print("[LORA] Measurement saved")

    except serial.SerialException as error:
        print("[LORA] Serial error:", error)


if __name__ == "__main__":

    print("===================================")
    print(" EPL428 GPS + LoRa Measurement Logger")
    print("===================================")

    print("Output:", os.path.abspath(CSV_FILE))

    gps_thread = threading.Thread(target=read_gps, daemon=True)
    lora_thread = threading.Thread(target=read_lora, daemon=True)

    gps_thread.start()
    lora_thread.start()

    try:
        while True:
            gps_thread.join(timeout=1)
            lora_thread.join(timeout=1)

    except KeyboardInterrupt:
        print("\nMeasurement logging stopped.")
