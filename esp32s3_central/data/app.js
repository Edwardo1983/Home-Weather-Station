/**
 * @file app.js
 * @brief Home Weather Station - Web Interface JavaScript
 * Handles WebSocket communication, real-time updates, and UI interactions
 */

// ============================================================================
// Configuration
// ============================================================================

const CONFIG = {
    WS_URL: `ws://${window.location.hostname}:81`,
    API_BASE: `/api`,
    UPDATE_INTERVAL: 5000, // 5 seconds for sensor updates
    CHART_UPDATE_INTERVAL: 60000 // 1 minute for charts
};

// ============================================================================
// Global State
// ============================================================================

let ws = null;
let wsConnected = false;
let sensorData = {
    indoor_main: {},
    indoor_secondary: {},
    outdoor: {},
    weather_api: {},
    ml_prediction: {}
};
let systemStatus = {
    uptime: 0,
    heap: 0,
    wifi_signal: 0,
    temperature: 0
};

// ============================================================================
// WebSocket Management
// ============================================================================

function initWebSocket() {
    console.log(`Connecting to WebSocket: ${CONFIG.WS_URL}`);

    ws = new WebSocket(CONFIG.WS_URL);

    ws.onopen = (event) => {
        console.log('WebSocket connected');
        wsConnected = true;
        updateSystemStatus('online');
        requestInitialData();
    };

    ws.onmessage = (event) => {
        try {
            const message = JSON.parse(event.data);
            handleWebSocketMessage(message);
        } catch (error) {
            console.error('Error parsing WebSocket message:', error);
        }
    };

    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        updateSystemStatus('error');
    };

    ws.onclose = (event) => {
        console.log('WebSocket disconnected');
        wsConnected = false;
        updateSystemStatus('offline');
        // Reconnect after 3 seconds
        setTimeout(initWebSocket, 3000);
    };
}

function handleWebSocketMessage(message) {
    switch (message.type) {
        case 'sensor_update':
            handleSensorUpdate(message.data);
            break;
        case 'system_status':
            handleSystemStatus(message.data);
            break;
        case 'forecast_update':
            handleForecastUpdate(message.data);
            break;
        case 'ml_prediction':
            handleMLPrediction(message.data);
            break;
        default:
            console.log('Unknown message type:', message.type);
    }
}

function sendWebSocketMessage(type, data = {}) {
    if (wsConnected && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ type, data }));
    } else {
        console.warn('WebSocket not connected');
    }
}

function requestInitialData() {
    sendWebSocketMessage('request_sensor_data');
    sendWebSocketMessage('request_system_status');
    sendWebSocketMessage('request_forecast');
    sendWebSocketMessage('request_ml_prediction');
}

// ============================================================================
// Data Handlers
// ============================================================================

function handleSensorUpdate(data) {
    // Indoor main room (BME680)
    if (data.indoor_main) {
        sensorData.indoor_main = data.indoor_main;
        updateElement('temp-room1', formatTemperature(data.indoor_main.temperature));
        updateElement('humidity-room1', formatPercentage(data.indoor_main.humidity));
        updateElement('pressure-room1', formatPressure(data.indoor_main.pressure));
        updateElement('iaq-room1', formatIAQ(data.indoor_main.iaq));
    }

    // Indoor secondary room (DHT22 from node)
    if (data.indoor_secondary) {
        sensorData.indoor_secondary = data.indoor_secondary;
        updateElement('temp-room2', formatTemperature(data.indoor_secondary.temperature));
        updateElement('humidity-room2', formatPercentage(data.indoor_secondary.humidity));

        const status = data.indoor_secondary.rssi ? 'Online' : 'Offline';
        updateElement('status-room2', status,
            status === 'Online' ? 'status-badge' : 'status-badge offline');

        const minutesAgo = Math.floor((Date.now() - data.indoor_secondary.timestamp) / 60000);
        updateElement('update-room2', minutesAgo + ' min ago');
    }

    // Outdoor sensors
    if (data.outdoor) {
        sensorData.outdoor = data.outdoor;
        updateElement('temp-outdoor', formatTemperature(data.outdoor.temperature));
        updateElement('humidity-outdoor', formatPercentage(data.outdoor.humidity));
        updateElement('pressure-outdoor', formatPressure(data.outdoor.pressure));
        updateElement('light-outdoor', data.outdoor.light + ' lux');

        // Pressure trend
        let trend = '→ Stable';
        if (data.outdoor.pressure_trend > 0) trend = '↑ Rising';
        else if (data.outdoor.pressure_trend < 0) trend = '↓ Falling';
        updateElement('pressure-trend', trend);
    }

    // Heart rate
    if (data.heart_rate !== undefined) {
        if (data.heart_rate > 0) {
            updateElement('heart-rate', data.heart_rate + ' BPM');
            updateElement('spo2', data.spo2 + ' %');
            updateElement('hr-status', 'Measuring');
        } else {
            updateElement('hr-status', 'Waiting for data');
        }
    }

    updateTimestamp();
}

function handleSystemStatus(data) {
    systemStatus = data;

    updateElement('uptime', formatUptime(data.uptime));
    updateElement('heap', (data.heap / 1024).toFixed(1) + ' KB');
    updateElement('wifi-signal', data.wifi_signal + ' dBm');
    updateElement('cpu-temp', formatTemperature(data.temperature));
}

function handleForecastUpdate(data) {
    if (!data || !data.forecast) return;

    const container = document.getElementById('forecast-container');
    container.innerHTML = '';

    const days = ['Today', 'Tomorrow', '+2', '+3', '+4'];

    data.forecast.forEach((day, index) => {
        const dayDiv = document.createElement('div');
        dayDiv.className = 'forecast-day';

        const icon = getWeatherIcon(day.condition);
        dayDiv.innerHTML = `
            <div class="forecast-day-label">${days[index]}</div>
            <div class="forecast-icon">${icon}</div>
            <div class="forecast-temp">${day.temp}°C</div>
            <div class="forecast-condition">${day.condition}</div>
        `;

        container.appendChild(dayDiv);
    });
}

function handleMLPrediction(data) {
    if (!data) return;

    // Rain probability
    updateElement('rain-prob', Math.round(data.rain_probability) + '%');
    document.getElementById('rain-prob-bar').style.width = data.rain_probability + '%';

    // Temperature trend
    let trendText = '→ STABLE';
    let trendColor = 'info';
    if (data.temperature_trend > 0) {
        trendText = '↑ RISING';
        trendColor = 'info';
    } else if (data.temperature_trend < 0) {
        trendText = '↓ FALLING';
        trendColor = 'warning';
    }
    updateElement('temp-trend', trendText);

    // General condition
    updateElement('general-condition', data.general_condition);
}

// ============================================================================
// UI Update Functions
// ============================================================================

function updateElement(id, content, className = null) {
    const element = document.getElementById(id);
    if (!element) return;

    element.textContent = content;

    if (className) {
        element.className = className;
    }
}

function updateSystemStatus(status) {
    const indicator = document.getElementById('system-status');
    if (!indicator) return;

    const dot = indicator.querySelector('.status-dot');
    const text = indicator.querySelector('span:not(.status-dot)');

    if (status === 'online') {
        dot.classList.remove('offline');
        dot.classList.add('online');
        text.textContent = 'Online';
    } else if (status === 'offline') {
        dot.classList.remove('online');
        dot.classList.add('offline');
        text.textContent = 'Offline';
    }
}

function updateTimestamp() {
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    updateElement('last-update', 'Last update: ' + timeString);
    updateElement('footer-time', timeString);
}

// ============================================================================
// Formatting Functions
// ============================================================================

function formatTemperature(temp) {
    if (temp === undefined || temp === null) return '-- °C';
    return temp.toFixed(1) + ' °C';
}

function formatPercentage(percent) {
    if (percent === undefined || percent === null) return '-- %';
    return percent.toFixed(0) + ' %';
}

function formatPressure(pressure) {
    if (pressure === undefined || pressure === null) return '-- hPa';
    return pressure.toFixed(0) + ' hPa';
}

function formatIAQ(iaq) {
    if (iaq === undefined || iaq === null) return '-- (Unknown)';

    let quality = 'Unknown';
    if (iaq < 50) quality = 'Good';
    else if (iaq < 100) quality = 'Acceptable';
    else if (iaq < 150) quality = 'Little Bad';
    else if (iaq < 200) quality = 'Bad';
    else quality = 'Very Bad';

    return iaq + ' (' + quality + ')';
}

function formatUptime(milliseconds) {
    if (!milliseconds) return '00:00:00';

    const totalSeconds = Math.floor(milliseconds / 1000);
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;

    return String(hours).padStart(2, '0') + ':' +
           String(minutes).padStart(2, '0') + ':' +
           String(seconds).padStart(2, '0');
}

function getWeatherIcon(condition) {
    const iconMap = {
        'Sunny': '☀️',
        'Partly Cloudy': '⛅',
        'Cloudy': '☁️',
        'Rainy': '🌧️',
        'Stormy': '⛈️',
        'Snow': '❄️',
        'Foggy': '🌫️'
    };

    return iconMap[condition] || '🌡️';
}

// ============================================================================
// API Functions
// ============================================================================

async function apiCall(endpoint, method = 'GET', data = null) {
    const options = {
        method: method,
        headers: {
            'Content-Type': 'application/json'
        }
    };

    if (data) {
        options.body = JSON.stringify(data);
    }

    try {
        const response = await fetch(CONFIG.API_BASE + endpoint, options);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error('API call failed:', error);
        return null;
    }
}

// ============================================================================
// Page Navigation
// ============================================================================

document.addEventListener('DOMContentLoaded', function() {
    // Initialize WebSocket connection
    initWebSocket();

    // Update timestamp every second
    setInterval(updateTimestamp, 1000);

    // Periodic data refresh
    setInterval(requestInitialData, CONFIG.UPDATE_INTERVAL);
});

// ============================================================================
// Configuration Page Functions
// ============================================================================

function scanWiFiNetworks() {
    const button = event.target;
    button.disabled = true;
    button.textContent = 'Scanning...';

    apiCall('/wifi/scan').then(result => {
        if (result && result.networks) {
            const select = document.getElementById('wifi-ssid-select');
            if (select) {
                select.innerHTML = '<option value="">-- Select network --</option>';
                result.networks.forEach(network => {
                    const option = document.createElement('option');
                    option.value = network;
                    option.textContent = network;
                    select.appendChild(option);
                });
                showAlert('Networks found: ' + result.networks.length, 'success');
            }
        }
        button.disabled = false;
        button.textContent = 'Scan Networks';
    });
}

function saveWiFiConfig() {
    const config = {
        ssid: document.getElementById('wifi-ssid')?.value || '',
        password: document.getElementById('wifi-password')?.value || '',
        static_ip: document.getElementById('wifi-static-ip')?.checked || false,
        ip: document.getElementById('wifi-ip')?.value || '',
        gateway: document.getElementById('wifi-gateway')?.value || '',
        subnet: document.getElementById('wifi-subnet')?.value || '',
        dns: document.getElementById('wifi-dns')?.value || ''
    };

    apiCall('/config/wifi', 'POST', config).then(result => {
        if (result && result.success) {
            showAlert('WiFi configuration saved. Restarting...', 'success');
            setTimeout(() => location.reload(), 2000);
        } else {
            showAlert('Failed to save configuration', 'error');
        }
    });
}

function saveAPIConfig() {
    const config = {
        openweathermap_key: document.getElementById('api-owm-key')?.value || '',
        tomorrow_key: document.getElementById('api-tomorrow-key')?.value || '',
        latitude: parseFloat(document.getElementById('api-latitude')?.value || 0),
        longitude: parseFloat(document.getElementById('api-longitude')?.value || 0)
    };

    apiCall('/config/api', 'POST', config).then(result => {
        if (result && result.success) {
            showAlert('API configuration saved', 'success');
        } else {
            showAlert('Failed to save API configuration', 'error');
        }
    });
}

function testAPIConnection() {
    apiCall('/api/test').then(result => {
        if (result && result.success) {
            showAlert('API connection successful!', 'success');
        } else {
            showAlert('API connection failed', 'error');
        }
    });
}

function downloadLogs() {
    window.location.href = CONFIG.API_BASE + '/logs/download';
}

function restartSystem() {
    if (confirm('Are you sure you want to restart the system?')) {
        apiCall('/system/restart', 'POST').then(result => {
            showAlert('System restarting...', 'success');
            setTimeout(() => location.reload(), 3000);
        });
    }
}

function factoryReset() {
    if (confirm('WARNING: This will reset all settings to defaults. Continue?')) {
        if (confirm('This action cannot be undone. Are you absolutely sure?')) {
            apiCall('/system/reset', 'POST').then(result => {
                showAlert('Factory reset in progress...', 'success');
                setTimeout(() => location.reload(), 3000);
            });
        }
    }
}

function uploadFirmware() {
    const fileInput = document.getElementById('firmware-file');
    if (!fileInput || !fileInput.files[0]) {
        showAlert('Please select a firmware file', 'error');
        return;
    }

    const formData = new FormData();
    formData.append('firmware', fileInput.files[0]);

    const progressBar = document.getElementById('upload-progress');

    fetch(CONFIG.API_BASE + '/ota/upload', {
        method: 'POST',
        body: formData
    }).then(response => {
        if (response.ok) {
            showAlert('Firmware uploaded successfully. Restarting...', 'success');
            progressBar.style.width = '100%';
            setTimeout(() => location.reload(), 3000);
        } else {
            showAlert('Firmware upload failed', 'error');
        }
    }).catch(error => {
        showAlert('Upload error: ' + error.message, 'error');
    });
}

// ============================================================================
// Utility Functions
// ============================================================================

function showAlert(message, type = 'info') {
    // Create temporary alert
    const alert = document.createElement('div');
    alert.className = `alert alert-${type}`;
    alert.textContent = message;
    alert.style.position = 'fixed';
    alert.style.top = '20px';
    alert.style.right = '20px';
    alert.style.zIndex = '9999';
    alert.style.maxWidth = '400px';

    document.body.appendChild(alert);

    setTimeout(() => {
        alert.remove();
    }, 5000);
}

function togglePasswordVisibility(id) {
    const input = document.getElementById(id);
    if (!input) return;

    if (input.type === 'password') {
        input.type = 'text';
    } else {
        input.type = 'password';
    }
}

// ============================================================================
// Export functions for page-specific code
// ============================================================================

window.WeatherStationUI = {
    apiCall,
    showAlert,
    scanWiFiNetworks,
    saveWiFiConfig,
    saveAPIConfig,
    testAPIConnection,
    downloadLogs,
    restartSystem,
    factoryReset,
    uploadFirmware,
    togglePasswordVisibility
};
