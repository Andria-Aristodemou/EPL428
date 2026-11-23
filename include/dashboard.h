
#pragma once

const char INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport"
          content="width=device-width, initial-scale=1.0">

    <title>EPL428 GPS Dashboard</title>

    <style>
        body {
            font-family: Arial, sans-serif;
            background: #101827;
            color: white;
            margin: 0;
            padding: 30px;
        }

        h1 {
            color: #38bdf8;
        }

        .card {
            background: #1e293b;
            padding: 20px;
            margin: 15px 0;
            border-radius: 12px;
        }

        .value {
            font-size: 24px;
            color: #38bdf8;
        }
    </style>
</head>

<body>

    <h1>EPL428 GPS Monitor</h1>

    <div class="card">
        <h2>GPS Status</h2>
        <div class="value" id="gpsStatus">Waiting...</div>
    </div>

    <div class="card">
        <h2>Location</h2>
        <p>Latitude: <span id="latitude">--</span></p>
        <p>Longitude: <span id="longitude">--</span></p>
    </div>

    <div class="card">
        <h2>GPS Information</h2>
        <p>Satellites: <span id="satellites">--</span></p>
        <p>HDOP: <span id="hdop">--</span></p>
    </div>

    <p>EPL428 | ESP32-S3 GPS Monitoring System</p>


<p>UTC: <span id="utc">--</span></p>
<p>Date: <span id="date">--</span></p>
<p>GPS age: <span id="gpsAge">--</span> ms</p>
<p>Received bytes: <span id="rxBytes">--</span></p>
<p>Valid checksums: <span id="checksum">--</span></p>
<p>WiFi: <span id="wifi">--</span></p>
<p>Radio: <span id="radio">--</span></p>
<p>Sequence: <span id="sequence">--</span></p>


<script>
async function updateDashboard() {
    try {
        const response = await fetch('/api/status', {
            cache: 'no-store'
        });

        if (!response.ok) {
            throw new Error('Server response error');
        }

        const data = await response.json();

        document.getElementById('gpsStatus').textContent =
            data.gps_status;

        document.getElementById('latitude').textContent =
            data.latitude;

        document.getElementById('longitude').textContent =
            data.longitude;

        document.getElementById('satellites').textContent =
            data.satellites;

        document.getElementById('hdop').textContent =
            data.hdop;

        document.getElementById('utc').textContent =
            data.utc;

        document.getElementById('date').textContent =
            data.date;

        document.getElementById('wifi').textContent =
            data.wifi_connected ? 'Connected' : 'Disconnected';

        document.getElementById('radio').textContent =
            data.radio_ready ? 'Ready' : 'Unavailable';

        document.getElementById('sequence').textContent =
            data.sequence;

        document.getElementById('gpsAge').textContent =
            data.gps_age_ms ?? '--';

        document.getElementById('rxBytes').textContent =
            data.rx_bytes;

        document.getElementById('checksum').textContent =
            data.checksum_ok;

    } catch (error) {
        document.getElementById('gpsStatus').textContent =
            'CONNECTION LOST';

        console.error('Dashboard error:', error);
    }
}

updateDashboard();
setInterval(updateDashboard, 1000);
</script>
</body>
</html>



</body>
</html>
)rawliteral";
