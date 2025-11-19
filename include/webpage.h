// Version: 1.3.2
// This file contains the HTML, CSS, and JavaScript for the web interface.

#ifndef WEBPAGE_H
#define WEBPAGE_H
 
const char HTML_CONTENT[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GPS GT-U7 Tester</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #fff;
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            text-align: center;
            padding: 30px 0;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            margin-bottom: 30px;
            backdrop-filter: blur(10px);
        }
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            display: flex; align-items: center; justify-content: center; gap: 15px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .header .subtitle {
            font-size: 1.2em;
            opacity: 0.9;
        }
        .status-bar {
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            gap: 15px;
            margin-bottom: 30px;
        }
        .status-item {
            background: rgba(255, 255, 255, 0.15);
            padding: 15px 25px;
            border-radius: 10px;
            backdrop-filter: blur(10px);
            flex: 1;
            min-width: 150px;
            text-align: center;
        }
        .status-item .label {
            font-size: 0.9em;
            opacity: 0.8;
            margin-bottom: 5px;
        }
        .status-item .value {
            font-size: 1.5em;
            font-weight: bold;
        }
        .status-good { border-left: 4px solid #4CAF50; }
        .status-warning { border-left: 4px solid #FFC107; }
        .status-error { border-left: 4px solid #F44336; }
        .cards {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .card {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
        }
        .card h2 {
            display: flex; align-items: center; gap: 10px;
            margin-bottom: 20px;
            font-size: 1.5em;
            border-bottom: 2px solid rgba(255, 255, 255, 0.3);
            padding-bottom: 10px;
        }
        .data-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        .data-row:last-child {
            border-bottom: none;
        }
        .data-label {
            opacity: 0.8;
        }
        .data-value {
            font-weight: bold;
            font-family: 'Courier New', monospace;
        }
        .map-container {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
            margin-bottom: 30px;
        }
        .map-container h2 {
            margin-bottom: 15px;
        }
        #map {
            width: 100%;
            height: 400px;
            border-radius: 10px;
            background: #fff;
        }
        .controls {
            display: flex;
            gap: 15px;
            justify-content: center;
            flex-wrap: wrap;
        }
        .btn {
            padding: 15px 30px;
            font-size: 1.1em;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s;
            font-weight: bold;
            text-transform: uppercase;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        .btn-primary {
            background: #4CAF50;
            color: white;
        }
        .btn-primary:hover {
            background: #45a049;
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.3);
        }
        .btn-danger {
            background: #F44336;
            color: white;
        }
        .btn-danger:hover {
            background: #da190b;
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.3);
        }
        .satellite-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(80px, 1fr));
            gap: 10px;
            margin-top: 15px;
        }
        .satellite {
            background: rgba(255, 255, 255, 0.2);
            padding: 10px;
            border-radius: 8px;
            text-align: center;
        }
        .satellite .sat-id {
            font-weight: bold;
            margin-bottom: 5px;
        }
        .satellite .sat-snr {
            font-size: 0.9em;
            opacity: 0.9;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .updating {
            animation: pulse 1s infinite;
        }
        @media (max-width: 768px) {
            .header h1 { font-size: 1.8em; }
            .cards { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>GPS Tester</h1>
            <div class="subtitle">ESP32-S3 DevKitC-1 N16R8 | v%PROJECT_VERSION%</div>
        </div>

        <div class="status-bar">
            <div class="status-item" id="fix-status">
                <div class="label">GPS Fix</div>
                <div class="value" id="fix-value">Searching...</div>
            </div>
            <div class="status-item" id="sat-status">
                <div class="label">Satellites</div>
                <div class="value" id="sat-value">0</div>
            </div>
            <div class="status-item" id="hdop-status">
                <div class="label">HDOP</div>
                <div class="value" id="hdop-value">--</div>
            </div>
            <div class="status-item status-good">
                <div class="label">Uptime</div>
                <div class="value" id="uptime-value">0s</div>
            </div>
        </div>

        <div class="cards">
            <div class="card">
                <h2>📍 Position Data</h2>
                <div class="data-row">
                    <span class="data-label">Latitude:</span>
                    <span class="data-value" id="lat">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Longitude:</span>
                    <span class="data-value" id="lng">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Altitude:</span>
                    <span class="data-value" id="alt">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Speed:</span>
                    <span class="data-value" id="speed">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Course:</span>
                    <span class="data-value" id="course">--</span>
                </div>
            </div>

            <div class="card">
                <h2>🕐 Time & Date</h2>
                <div class="data-row">
                    <span class="data-label">Date:</span>
                    <span class="data-value" id="date">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Time (UTC):</span>
                    <span class="data-value" id="time">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Age:</span>
                    <span class="data-value" id="age">--</span>
                </div>
            </div>

            <div class="card">
                <h2>📊 NMEA Diagnostics</h2>
                <div class="data-row">
                    <span class="data-label">Valid Sentences:</span>
                    <span class="data-value" id="valid">0</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Failed Checksums:</span>
                    <span class="data-value" id="failed">0</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Total Characters:</span>
                    <span class="data-value" id="chars">0</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Success Rate:</span>
                    <span class="data-value" id="success-rate">100%</span>
                </div>
            </div>

            <div class="card">
                <h2>🛰️ GPS Module Details</h2>
                <div class="data-row">
                    <span class="data-label">Model:</span>
                    <span class="data-value" id="gps-model">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Baud Rate:</span>
                    <span class="data-value" id="gps-baud">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Update Rate:</span>
                    <span class="data-value" id="gps-rate">--</span>
                </div>
            </div>

            <div class="card">
                <h2>🤖 Board Information</h2>
                 <div class="data-row">
                    <span class="data-label">Chip Model:</span>
                    <span class="data-value" id="board-chip">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">CPU Cores:</span>
                    <span class="data-value" id="board-cores">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">CPU Frequency:</span>
                    <span class="data-value" id="board-freq">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Flash / PSRAM:</span>
                    <span class="data-value" id="board-memory">--</span>
                </div>
            </div>
        </div>

        <div class="controls">
            <button class="btn btn-danger" onclick="resetGPS()">🔄 Reset GPS</button>
        </div>
    </div>

    <script>
        let ws;
        let reconnectInterval;

        function connect() {
            ws = new WebSocket('ws://' + window.location.hostname + '/ws');

            ws.onopen = () => {
                console.log('WebSocket connected');
                clearInterval(reconnectInterval);
            };

            ws.onmessage = (event) => {
                const data = JSON.parse(event.data);
                updateDisplay(data);
            };

            ws.onclose = () => {
                console.log('WebSocket disconnected');
                reconnectInterval = setInterval(() => {
                    connect();
                }, 3000);
            };

            ws.onerror = (error) => {
                console.error('WebSocket error:', error);
            };
        }

        function updateDisplay(data) {
            document.getElementById('fix-value').textContent = data.fix ? 'LOCKED' : 'NO FIX';
            const fixStatus = document.getElementById('fix-status');
            fixStatus.className = 'status-item ' + (data.fix ? 'status-good' : 'status-error');

            document.getElementById('sat-value').textContent = data.satellites;
            const satStatus = document.getElementById('sat-status');
            satStatus.className = 'status-item ' + (data.satellites >= 4 ? 'status-good' : 'status-warning');

            document.getElementById('hdop-value').textContent = data.hdop;
            const hdopStatus = document.getElementById('hdop-status');
            const hdopVal = parseFloat(data.hdop);
            hdopStatus.className = 'status-item ' + (hdopVal < 2 ? 'status-good' : hdopVal < 5 ? 'status-warning' : 'status-error');

            document.getElementById('uptime-value').textContent = data.uptime;

            document.getElementById('lat').textContent = data.latitude;
            document.getElementById('lng').textContent = data.longitude;
            document.getElementById('alt').textContent = data.altitude;
            document.getElementById('speed').textContent = data.speed;
            document.getElementById('course').textContent = data.course;

            document.getElementById('date').textContent = data.date;
            document.getElementById('time').textContent = data.time;
            document.getElementById('age').textContent = data.age;

            document.getElementById('valid').textContent = data.validSentences;
            document.getElementById('failed').textContent = data.failedChecksums;
            document.getElementById('chars').textContent = data.totalChars;
            document.getElementById('success-rate').textContent = data.successRate;

            document.getElementById('gps-model').textContent = data.gpsModel;
            document.getElementById('gps-baud').textContent = data.gpsBaud;
            document.getElementById('gps-rate').textContent = data.gpsRate;

            document.getElementById('board-chip').textContent = data.chipModel;
            document.getElementById('board-cores').textContent = data.chipCores;
            document.getElementById('board-freq').textContent = data.chipFreq;
            document.getElementById('board-memory').textContent = data.chipMemory;
        }

        function resetGPS() {
            if (confirm('Reset GPS module? This will restart the GPS.')) {
                fetch('/reset', { method: 'POST' })
                    .then(response => response.text())
                    .then(data => alert(data))
                    .catch(error => alert('Error: ' + error));
            }
        }

        connect();
    </script>
</body>
</html>
)rawliteral";

#endif // WEBPAGE_H