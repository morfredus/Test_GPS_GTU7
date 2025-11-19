#ifndef WEBPAGE_H
#define WEBPAGE_H

const char HTML_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GPS Tester ESP32-S3</title>
    <style>
        :root {
            --bg-color: #282c34;
            --card-bg-color: #3a3f47;
            --text-color: #e0e0e0;
            --primary-color: #61dafb;
            --success-color: #4CAF50;
            --error-color: #F44336;
            --warning-color: #FFC107;
            --border-color: #444a54;
            --button-bg: #4a4f58;
            --button-hover-bg: #5a606b;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-color);
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
        }
        header {
            background-color: var(--card-bg-color);
            padding: 20px;
            text-align: center;
            border-bottom: 1px solid var(--border-color);
        }
        header h1 {
            margin: 0;
            color: var(--primary-color);
        }
        header p {
            margin: 5px 0 0;
            font-size: 0.9em;
            color: var(--text-color);
        }
        .container {
            flex: 1;
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
            padding: 20px;
            justify-content: center;
            align-items: flex-start;
        }
        .card {
            background-color: var(--card-bg-color);
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            padding: 20px;
            flex: 1;
            min-width: 300px;
            max-width: 450px;
            display: flex;
            flex-direction: column;
            border: 1px solid var(--border-color);
        }
        .card h2 {
            color: var(--primary-color);
            margin-top: 0;
            border-bottom: 1px solid var(--border-color);
            padding-bottom: 10px;
            margin-bottom: 15px;
        }
        .data-item {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px dashed var(--border-color);
        }
        .data-item:last-child {
            border-bottom: none;
        }
        .data-label {
            font-weight: bold;
            color: var(--text-color);
        }
        .data-value {
            color: var(--primary-color);
        }
        .status-indicator {
            padding: 5px 10px;
            border-radius: 5px;
            font-weight: bold;
            text-align: center;
            margin-bottom: 15px;
        }
        .status-ok { background-color: var(--success-color); color: white; }
        .status-no-fix { background-color: var(--error-color); color: white; }
        .button-group {
            display: flex;
            gap: 10px;
            margin-top: 20px;
            flex-wrap: wrap;
        }
        .button-group button, .button-group a {
            flex: 1;
            padding: 10px 15px;
            border: none;
            border-radius: 5px;
            background-color: var(--button-bg);
            color: var(--text-color);
            cursor: pointer;
            font-size: 1em;
            text-decoration: none;
            text-align: center;
            transition: background-color 0.3s ease;
        }
        .button-group button:hover, .button-group a:hover {
            background-color: var(--button-hover-bg);
        }
        #reset-gps-btn.confirm {
            background-color: var(--warning-color);
            color: var(--bg-color);
        }
        #notification-area {
            position: fixed;
            bottom: 70px; /* Above footer */
            left: 50%;
            transform: translateX(-50%);
            padding: 12px 20px;
            background-color: var(--card-bg-color);
            border: 1px solid var(--border-color);
            border-radius: 8px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.3);
            z-index: 1000;
            opacity: 0;
            visibility: hidden;
            transition: opacity 0.3s, visibility 0.3s;
        }
        .notification-success { color: var(--success-color); }
        .notification-error { color: var(--error-color); }
        footer {
            background-color: var(--card-bg-color);
            padding: 15px;
            text-align: center;
            font-size: 0.8em;
            color: var(--text-color);
            border-top: 1px solid var(--border-color);
            margin-top: auto;
        }
        @media (max-width: 768px) {
            .card {
                min-width: unset;
                width: 100%;
            }
            .button-group {
                flex-direction: column;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>GPS Tester ESP32-S3</h1>
        <p>Version: %PROJECT_VERSION%</p>
        <p>Dernière mise à jour: <span id="last-update">--:--:--</span></p>
    </header>

    <div class="container">
        <div class="card">
            <h2>Statut GPS</h2>
            <div id="fix-status" class="status-indicator status-no-fix">NO FIX</div>
            <div class="data-item"><span class="data-label">Satellites:</span> <span id="satellites" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">HDOP:</span> <span id="hdop" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Âge du Fix:</span> <span id="age" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Date UTC:</span> <span id="date" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Heure UTC:</span> <span id="time" class="data-value">--</span></div>
            <div class="button-group">
                <a id="google-maps-link" href="#" target="_blank" class="button" style="display: none;">Voir sur Google Maps</a>
                <button id="reset-gps-btn">Réinitialiser GPS</button>
            </div>
        </div>

        <div class="card">
            <h2>Données de Position</h2>
            <div class="data-item"><span class="data-label">Latitude:</span> <span id="latitude" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Longitude:</span> <span id="longitude" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Altitude:</span> <span id="altitude" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Vitesse:</span> <span id="speed" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Cap:</span> <span id="course" class="data-value">--</span></div>
        </div>

        <div class="card">
            <h2>Diagnostics GPS</h2>
            <div class="data-item"><span class="data-label">Modèle GPS:</span> <span id="gpsModel" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Baud Rate:</span> <span id="gpsBaud" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Taux de rafraîchissement:</span> <span id="gpsRate" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Phrases Valides:</span> <span id="validSentences" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Checksums Échoués:</span> <span id="failedChecksums" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Caractères Traités:</span> <span id="totalChars" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Taux de Succès:</span> <span id="successRate" class="data-value">--</span></div>
        </div>

        <div class="card">
            <h2>Informations Système</h2>
            <div class="data-item"><span class="data-label">Modèle de Puce:</span> <span id="chipModel" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Cœurs CPU:</span> <span id="chipCores" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Fréquence CPU:</span> <span id="chipFreq" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Mémoire:</span> <span id="chipMemory" class="data-value">--</span></div>
            <div class="data-item"><span class="data-label">Uptime:</span> <span id="uptime" class="data-value">--</span></div>
        </div>
    </div>

    <div id="notification-area"></div>

    <footer>
        &copy; 2024 GPS Tester ESP32-S3. Powered by PlatformIO.
    </footer>

    <script>
        var socket = new WebSocket('ws://' + location.hostname + '/ws');

        socket.onmessage = function(event) {
            var data = JSON.parse(event.data);
            
            // Update GPS Status
            var fixStatusElement = document.getElementById('fix-status');
            if (data.fix) {
                fixStatusElement.textContent = 'FIX OK';
                fixStatusElement.className = 'status-indicator status-ok';
            } else {
                fixStatusElement.textContent = 'NO FIX';
                fixStatusElement.className = 'status-indicator status-no-fix';
            }

            // Update GPS Data
            document.getElementById('satellites').textContent = data.satellites;
            document.getElementById('hdop').textContent = data.hdop;
            document.getElementById('age').textContent = data.age;
            document.getElementById('date').textContent = data.date;
            document.getElementById('time').textContent = data.time;
            document.getElementById('latitude').textContent = data.latitude;
            document.getElementById('longitude').textContent = data.longitude;
            document.getElementById('altitude').textContent = data.altitude;
            document.getElementById('speed').textContent = data.speed;
            document.getElementById('course').textContent = data.course;

            // Update GPS Diagnostics
            document.getElementById('gpsModel').textContent = data.gpsModel;
            document.getElementById('gpsBaud').textContent = data.gpsBaud;
            document.getElementById('gpsRate').textContent = data.gpsRate;
            document.getElementById('validSentences').textContent = data.validSentences;
            document.getElementById('failedChecksums').textContent = data.failedChecksums;
            document.getElementById('totalChars').textContent = data.totalChars;
            document.getElementById('successRate').textContent = data.successRate;

            // Update System Info
            document.getElementById('chipModel').textContent = data.chipModel;
            document.getElementById('chipCores').textContent = data.chipCores;
            document.getElementById('chipFreq').textContent = data.chipFreq;
            document.getElementById('chipMemory').textContent = data.chipMemory;
            document.getElementById('uptime').textContent = data.uptime;

            // Update Google Maps Link
            var googleMapsLink = document.getElementById('google-maps-link');
            if (data.latitude !== '--' && data.longitude !== '--') {
                googleMapsLink.href = `https://www.google.com/maps/search/?api=1&query=${data.latitude},${data.longitude}`;
                googleMapsLink.style.display = 'inline-block';
            } else {
                googleMapsLink.style.display = 'none';
            }

            // Update last update timestamp
            var now = new Date();
            document.getElementById('last-update').textContent = now.toLocaleTimeString();
        };

        socket.onopen = function(event) {
            console.log('WebSocket connection opened');
        };

        socket.onclose = function(event) {
            console.log('WebSocket connection closed');
            // Attempt to reconnect after a delay
            setTimeout(() => {
                socket = new WebSocket('ws://' + location.hostname + '/ws');
            }, 2000);
        };

        socket.onerror = function(error) {
            console.error('WebSocket Error: ', error);
        };

        // --- Notification System ---
        const notificationArea = document.getElementById('notification-area');
        let notificationTimeout;

        function showNotification(message, type = 'info', duration = 5000) {
            clearTimeout(notificationTimeout);
            const timestamp = new Date().toLocaleTimeString();
            notificationArea.innerHTML = `[${timestamp}] ${message}`;
            notificationArea.className = `notification-${type}`;
            notificationArea.style.opacity = '1';
            notificationArea.style.visibility = 'visible';

            if (duration > 0) {
                notificationTimeout = setTimeout(() => {
                    notificationArea.style.opacity = '0';
                    notificationArea.style.visibility = 'hidden';
                }, duration);
            }
        }

        // Handle GPS Reset button click
        const resetButton = document.getElementById('reset-gps-btn');
        let confirmResetTimeout;

        function resetConfirmState() {
            resetButton.classList.remove('confirm');
            resetButton.textContent = 'Réinitialiser GPS';
            clearTimeout(confirmResetTimeout);
        }

        resetButton.addEventListener('click', function() {
            if (resetButton.classList.contains('confirm')) {
                // Second click: perform reset
                resetConfirmState();
                showNotification('Envoi de la commande de réinitialisation...', 'info', 2000);
                fetch('/reset', { method: 'POST' }).then(response => {
                    if (response.ok) {
                        showNotification('Commande de réinitialisation GPS envoyée.', 'success');
                    } else {
                        showNotification('Erreur lors de l\'envoi de la commande.', 'error');
                    }
                }).catch(error => {
                    console.error('Erreur:', error);
                    showNotification('Erreur de connexion au serveur.', 'error');
                });
            } else {
                // First click: ask for confirmation
                resetButton.classList.add('confirm');
                resetButton.textContent = 'Confirmer la réinitialisation ?';
                showNotification('Cliquez à nouveau pour confirmer la réinitialisation.', 'warning', 5000);
                confirmResetTimeout = setTimeout(resetConfirmState, 5000);
            }
        });
    </script>
</body>
</html>
)rawliteral";

#endif // WEBPAGE_H