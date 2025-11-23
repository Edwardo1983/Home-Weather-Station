### 📋 PROCEDURA DE TESTARE (Când primești componentele)
**PASUL 1: Testare Individuală Noduri (Fără ESP-NOW)**
A. ESP-01S Interior (cel mai simplu)
Upload cod
Deschide Serial Monitor (115200 baud)
Verifică output:
[INFO] MAC Address: XX:XX:XX:XX:XX:XX  ← NOTEAZĂ!
[OK] DHT22 initialized
[DHT22] Temp: 22.5°C, Humidity: 45%
Dacă vezi erori DHT22: Verifică conexiune hardware (4.7kΩ pull-up pe data pin)
B. ESP-01S Exterior (mediu)
Upload cod
Serial Monitor:
[INFO] MAC Address: XX:XX:XX:XX:XX:XX  ← NOTEAZĂ!
[OK] I2C initialized
[OK] TCA9548A detected
[OK] AHT20 initialized
[OK] BMP280 initialized
[OK] BH1750 initialized
[SENSOR] Reading...
[AHT20] T=15.3°C H=60%
[BMP280] P=1013 hPa
[BH1750] Light=450 lux
Dacă erori I2C: Verifică pull-up resistors (4.7kΩ pe SDA și SCL)
C. ESP32-S3 Central (complex)

**ÎNAINTE de upload, creează secrets.h:**
cp esp32s3_central/secrets_template.h esp32s3_central/secrets.h

**Editează secrets.h:**
#define WIFI_SSID "RetelaTa"
#define WIFI_PASSWORD "ParolaTa"
#define OPENWEATHERMAP_API_KEY "cheia_ta_OWM"
#define LATITUDE 44.4268  // Locația ta
#define LONGITUDE 26.1025

**// MAC-urile notate la pasul 1:**
#define ESPNOW_MAC_INTERIOR {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01}
#define ESPNOW_MAC_EXTERIOR {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02}
Upload cod

**Serial Monitor:**
[OK] Displays initialized  ← Critc!
[OK] Sensors initialized
[OK] WiFi connected
IP: 192.168.1.XXX
[OK] ESP-NOW initialized
[OK] ESP-NOW peers registered  ← Critc!
[SENSOR] Temp: 22.5°C
[WEATHER] Fetching...
[WEATHER OK] Temp: 15°C, Clear
PASUL 2: Testare ESP-NOW Communication
Pornește TOATE 3 device-urile simultan

**Pe ESP32-S3 Central, urmărește:**
[ESPNOW] Received 60 bytes
[ESPNOW] Interior: T=22.3°C H=45%
[ESPNOW] Exterior: T=15.1°C H=62% P=1013 hPa

**Dacă NU primești date:**
Verifică MAC addresses în secrets.h (toate device-urile)
Verifică că toate device-urile folosesc același canal ESP-NOW (6)
Reduce distanța între noduri (<2 metri pentru test)
PASUL 3: Testare Display (ESP32-S3)
Verifică că display-urile se aprind (backlight)

**Verifică conținut afișat:**
Display 1 (sus): Ora, temperaturi indoor
Display 2 (mijloc): Vremea outdoor (API)
Display 3 (jos): Butoane și date extinse

**Dacă display-uri negre:**
Verifică backlight PWM (pin 47)
Verifică SPI connections (MOSI, MISO, SCK)
Verifică CS pins (10, 9, 46)
Verifică DC pin (14) și RST pin (21)
PASUL 4: Testare SD Card Logging
Inserează SD card formatat FAT32
După 5 minute, scoate cardul
Verifică fișier /weather.csv pe card:
timestamp,temp_indoor,humidity_indoor,temp_outdoor,humidity_outdoor,pressure,light,iaq
2024-01-15T08:30:45Z,22.5,45.2,15.3,60.1,1013.25,450,42

**🚨 PROBLEME COMUNE LA TESTARE**
"Sketch too big" la ESP-01S
// În config.h:
#define DEBUG_SERIAL false
#define DEBUG_DHT false
#define DEBUG_ESPNOW false
WiFi connection failed
Verifică SSID/Password în secrets.h
Verifică că routerul e pe 2.4GHz (nu 5GHz)
ESP8266 nu suportă WPA3 - folosește WPA2
ESP-NOW send failed
MAC addresses greșite în secrets.h
Device-uri pe canale WiFi diferite
Distanță prea mare (>20m în interior)
BME680 not found
Verifică adresa I2C: poate fi 0x76 sau 0x77
Schimbă în config.h dacă e nevoie
Display artifacts/glitches
Cabluri SPI prea lungi (max 15cm recomandat)
Alimentare insuficientă (folosește surse 5V/2A)
Interferență - separă cablurile SPI de cele de putere

**⚡ HARDWARE CHECKLIST (Când asamblezi)**
ESP-01S Deep Sleep - OBLIGATORIU!
GPIO 16 (D0) ----[470Ω]---- RST
Fără această conexiune, ESP-01S nu se va trezi din deep sleep!

Pull-up Resistors - OBLIGATORII!
**ESP-01S Interior:**
  DHT22 data pin ----[4.7kΩ]---- VCC

**ESP-01S Exterior:**
  I2C SDA ----[4.7kΩ]---- VCC
  I2C SCL ----[4.7kΩ]---- VCC
**Alimentare:**
ESP32-S3: 5V/2A (3 display-uri consumă ~600mA)
ESP-01S: 3.3V/500mA (peak WiFi)
**Condensatoare bypass (recomandat):**
100µF electrolitic la fiecare ESP
0.1µF ceramic lângă fiecare senzor

**📝 FIȘIERE DE CREAT MANUAL**
Când primești componentele, ÎNAINTE de primul upload:

# 1. Creează secrets.h pentru toate device-urile:
cp esp32s3_central/secrets_template.h esp32s3_central/secrets.h
cp esp01s_interior/secrets_template.h esp01s_interior/secrets.h
cp esp01s_exterior/secrets_template.h esp01s_exterior/secrets.h

# 2. Editează fiecare secrets.h și completează:
#    - WiFi credentials
#    - API keys
#    - Coordonate GPS
#    - MAC addresses (după prima testare)