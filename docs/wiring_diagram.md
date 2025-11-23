# Wiring Diagram & Pin Configuration

Detailed pin connections for all three nodes of the Home Weather Station.

## Central Node - ESP32-S3 N16R8

### Power Supply

| Pin | Connection | Voltage | Notes |
|-----|-----------|---------|-------|
| Vin | USB 5V or external PSU | 5V | Power input |
| GND | Ground | 0V | Power ground |
| 3V3 | 3.3V regulator output | 3.3V | For sensors (50mA max) |

### SPI Bus (Shared for Displays & SD Card)

| Signal | GPIO | Purpose |
|--------|------|---------|
| MOSI | GPIO 11 | Master Out Slave In (DIN) |
| MISO | GPIO 13 | Master In Slave Out (DO) |
| SCK | GPIO 12 | Serial Clock (CLK) |

### Display Control (Shared)

| Signal | GPIO | Purpose | Notes |
|--------|------|---------|-------|
| DC | GPIO 14 | Data/Command | All displays share |
| RST | GPIO 21 | Reset | All displays share |
| LED | GPIO 47 | Backlight (PWM) | PWM channel 0, 5000Hz |

### Display 1 (Top - Time/Indoor)

| Signal | GPIO | Connection | Type |
|--------|------|-----------|------|
| CS | GPIO 10 | ILI9341 CS | SPI |
| T_CS | GPIO 3 | XPT2046 CS | SPI |
| T_IRQ | GPIO 17 | XPT2046 IRQ | Digital In |

**ILI9341 Pinout:**
- VCC → 3.3V
- GND → GND
- SCK → GPIO 12
- SDA/MOSI → GPIO 11
- A0/DC → GPIO 14
- RST → GPIO 21
- CS → GPIO 10
- LED (Backlight) → GPIO 47 (PWM)

**XPT2046 Pinout:**
- VCC → 3.3V
- GND → GND
- CLK → GPIO 12
- DIN → GPIO 11
- DO → GPIO 13
- CS → GPIO 3
- IRQ → GPIO 17

### Display 2 (Middle - Weather/Forecast)

| Signal | GPIO | Connection | Type |
|--------|------|-----------|------|
| CS | GPIO 9 | ILI9341 CS | SPI |
| T_CS | GPIO 8 | XPT2046 CS | SPI |
| T_IRQ | GPIO 16 | XPT2046 IRQ | Digital In |

**Connections:** Same as Display 1, but with different CS pins

### Display 3 (Bottom - Extended/Controls)

| Signal | GPIO | Connection | Type |
|--------|------|-----------|------|
| CS | GPIO 46 | ILI9341 CS | SPI |
| T_CS | GPIO 18 | XPT2046 CS | SPI |
| T_IRQ | GPIO 15 | XPT2046 IRQ | Digital In |

**Connections:** Same as Display 1, but with different CS pins

### I2C Bus (Local Sensors)

| Signal | GPIO | Voltage | Notes |
|--------|------|---------|-------|
| SDA | GPIO 1 | 3.3V | I2C Data (with pull-ups) |
| SCL | GPIO 2 | 3.3V | I2C Clock (with pull-ups) |

**Pull-up Resistors:** 4.7kΩ from SDA/SCL to 3.3V (already on sensor modules)

### BME680 (Temperature/Humidity/Pressure/IAQ)

| Pin | Connection | Notes |
|-----|-----------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL | GPIO 2 | I2C Clock |
| SDA | GPIO 1 | I2C Data |
| **I2C Address** | **0x76** | Selectable (0x77 with SDO high) |

### MAX30102 (Heart Rate/SpO2)

| Pin | Connection | Notes |
|-----|-----------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL | GPIO 2 | I2C Clock |
| SDA | GPIO 1 | I2C Data |
| INT (optional) | Not connected | Interrupt pin (unused) |
| **I2C Address** | **0x57** | Fixed address |

### SD Card Module (SPI)

| Signal | GPIO | Notes |
|--------|------|-------|
| CS | GPIO 4 | Chip Select |
| MOSI | GPIO 11 | Shared with displays |
| MISO | GPIO 13 | Shared with displays |
| CLK | GPIO 12 | Shared with displays |
| VCC | 3.3V | Power (or 5V with regulator) |
| GND | GND | Ground |

---

## Interior Node - ESP-01S with DHT22

### Power Supply

| Pin | Connection | Voltage |
|-----|-----------|---------|
| Vin/VCC | 3.3V PSU | 3.3V (or USB) |
| GND | Ground | 0V |

### DHT22 Sensor

| Pin | GPIO | Connection | Notes |
|-----|------|-----------|-------|
| VCC | - | 3.3V | Power |
| DATA | GPIO 2 (D4) | GPIO 2 | Temperature/Humidity |
| GND | - | GND | Ground |
| **Pull-up** | - | 4.7kΩ | Between DATA and VCC |

**DHT22 Pinout (Left to Right):**
1. VCC (3.3V)
2. DATA (GPIO 2)
3. NC (Not Connected)
4. GND

### Status LED (Optional)

| Pin | GPIO | Notes |
|-----|------|-------|
| LED | GPIO 0 (D3) | Active LOW (LED on when GPIO=LOW) |
| Current | - | 10-20mA (use resistor if needed) |

### Serial Communication (for debugging)

| Pin | Connection | Baud Rate |
|-----|-----------|-----------|
| TX | GPIO 1 | 115200 |
| RX | GPIO 3 | 115200 |

---

## Exterior Node - ESP-01S with I2C Multiplexer

### Power Supply

| Pin | Connection | Voltage |
|-----|-----------|---------|
| Vin/VCC | 3.3V PSU | 3.3V (or USB) |
| GND | Ground | 0V |

### I2C Bus (Shared for all sensors + multiplexer)

| Signal | GPIO | Voltage | Notes |
|--------|------|---------|-------|
| SDA | GPIO 0 (D3) | 3.3V | I2C Data |
| SCL | GPIO 2 (D4) | 3.3V | I2C Clock |
| **Pull-ups** | - | 4.7kΩ | To 3.3V (on modules or external) |

### TCA9548A I2C Multiplexer

| Pin | Connection | Address | Notes |
|-----|-----------|---------|-------|
| VCC | 3.3V | - | Power |
| GND | GND | - | Ground |
| SCL | GPIO 2 | 0x70 | I2C Clock (A0=A1=A2=GND) |
| SDA | GPIO 0 | - | I2C Data |
| CH0 | AHT20 + BMP280 | - | Channel 0 |
| CH1 | BH1750 | - | Channel 1 |

**Multiplexer Control Register:**
- 0x00 = Channel disabled
- 0x01 = Channel 0 enabled
- 0x02 = Channel 1 enabled
- 0x04 = Channel 2 enabled
- etc.

### Channel 0 Sensors (AHT20 + BMP280)

#### AHT20 (Temperature/Humidity)

| Pin | Connection | Notes |
|-----|-----------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL | GPIO 2 (via MUX) | I2C Clock |
| SDA | GPIO 0 (via MUX) | I2C Data |
| **I2C Address** | **0x38** | Fixed address |

#### BMP280 (Pressure)

| Pin | Connection | Notes |
|-----|-----------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL | GPIO 2 (via MUX) | I2C Clock |
| SDA | GPIO 0 (via MUX) | I2C Data |
| **I2C Address** | **0x76** | Selectable (0x77 with SDO high) |

### Channel 1 Sensor (BH1750)

#### BH1750 (Ambient Light)

| Pin | Connection | Notes |
|-----|-----------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL | GPIO 2 (via MUX) | I2C Clock |
| SDA | GPIO 0 (via MUX) | I2C Data |
| ADDR | GND | Address: 0x23 (or 0x5C if VCC) |

---

## Pin Summary Tables

### ESP32-S3 Used Pins

| GPIO | Function | Type | Notes |
|------|----------|------|-------|
| 1 | I2C SDA | Digital | Sensors |
| 2 | I2C SCL | Digital | Sensors |
| 3 | Touch 1 CS | Digital | SPI |
| 4 | SD Card CS | Digital | SPI |
| 8 | Touch 2 CS | Digital | SPI |
| 9 | Display 2 CS | Digital | SPI |
| 10 | Display 1 CS | Digital | SPI |
| 11 | SPI MOSI | SPI | All displays & SD |
| 12 | SPI SCK | SPI | All displays & SD |
| 13 | SPI MISO | SPI | All displays & SD |
| 14 | Display DC | Digital | All displays |
| 15 | Touch 3 IRQ | Digital | Input |
| 16 | Touch 2 IRQ | Digital | Input |
| 17 | Touch 1 IRQ | Digital | Input |
| 18 | Touch 3 CS | Digital | SPI |
| 21 | Display RST | Digital | All displays |
| 46 | Display 3 CS | Digital | SPI |
| 47 | Backlight LED | PWM | Channel 0 |

**Available Pins:** GPIO 5, 6, 7, 19, 20, 37-45, 48

### ESP-01S Interior Pins

| GPIO | Function | Type | Notes |
|------|----------|------|-------|
| 0 | Status LED | Digital | Optional |
| 2 | DHT22 DATA | Digital | 1-wire protocol |
| 1 | TX (serial) | Digital | Debug output |
| 3 | RX (serial) | Digital | Debug input |

### ESP-01S Exterior Pins

| GPIO | Function | Type | Notes |
|------|----------|------|-------|
| 0 | I2C SDA | Digital | To multiplexer |
| 2 | I2C SCL | Digital | To multiplexer |
| 1 | TX (serial) | Digital | Debug output |
| 3 | RX (serial) | Digital | Debug input |

---

## ESP-NOW MAC Addresses

Each node needs a unique MAC address for ESP-NOW communication. Identify addresses:

1. Upload firmware to each node
2. Open Serial Monitor at 115200 baud
3. Look for: `[INFO] MAC Address: XX:XX:XX:XX:XX:XX`
4. Update `secrets.h` files with these addresses

**Example:**

```
ESP32-S3 Central:   94:B9:7E:C0:A1:BC
ESP-01S Interior:   60:01:94:A0:12:34
ESP-01S Exterior:   60:01:94:A0:12:35
```

Then in `secrets.h`:

```cpp
// Central node
#define ESPNOW_MAC_INTERIOR {0x60, 0x01, 0x94, 0xA0, 0x12, 0x34}
#define ESPNOW_MAC_EXTERIOR {0x60, 0x01, 0x94, 0xA0, 0x12, 0x35}

// Interior node
#define ESPNOW_MAC_CENTRAL {0x94, 0xB9, 0x7E, 0xC0, 0xA1, 0xBC}

// Exterior node
#define ESPNOW_MAC_CENTRAL {0x94, 0xB9, 0x7E, 0xC0, 0xA1, 0xBC}
```

---

## I2C Addresses Summary

| Sensor | Default Address | Alternative | Notes |
|--------|-----------------|-------------|-------|
| BME680 | 0x76 | 0x77 | Central node |
| MAX30102 | 0x57 | - | Central node |
| AHT20 | 0x38 | - | Exterior (MUX CH0) |
| BMP280 | 0x76 | 0x77 | Exterior (MUX CH0) |
| BH1750 | 0x23 | 0x5C | Exterior (MUX CH1) |
| TCA9548A | 0x70 | - | Exterior |

---

## Soldering & Assembly Tips

1. **Pull-up Resistors:** Use 4.7kΩ on I2C lines SDA/SCL
2. **Decoupling Capacitors:** Add 100nF caps near sensor VCC pins
3. **Wire Gauge:** Use 18-22 AWG for power, 24-28 AWG for signals
4. **Connector Types:**
   - Displays: 2x13 pin header (13-pin SPI interface)
   - Sensors: 4-pin JST-GH (3.3V, GND, SDA, SCL)
   - Multiplexer: 8-pin DIP or breakout board

---

## Power Consumption

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-S3 (active) | ~150mA | WiFi/Bluetooth active |
| ESP32-S3 (idle) | ~10mA | CPU running, minimal I/O |
| Display (all 3) | ~200mA | Backlight at max brightness |
| BME680 | ~5mA | Active measurement |
| MAX30102 | ~50mA | When actively measuring |
| SD Card | ~50mA | During write operations |
| **Total (max)** | **~500mA** | All components active |
| **Total (idle)** | **~50mA** | Minimal operation |

**ESP-01S Nodes:**
- Active: ~80mA
- Deep sleep: <50µA

---

## Testing Checklist

- [ ] All power connections verified (correct voltage at each component)
- [ ] I2C pull-up resistors in place (4.7kΩ)
- [ ] SPI clock frequency appropriate (<40MHz for displays)
- [ ] CS pins isolated (each display has independent CS)
- [ ] Touch screen IRQ pins functioning
- [ ] No short circuits detected (continuity test)
- [ ] Sensor I2C addresses confirmed with scanner
- [ ] MAC addresses recorded in all secrets.h files

---

**Last Updated:** 2025-11-22
