/**
 * @file User_Setup.h
 * @brief TFT_eSPI Configuration for ESP32-S3 Central Node
 *
 * IMPORTANT: This file must be placed in the TFT_eSPI library directory
 * Location: Arduino/libraries/TFT_eSPI/User_Setups/User_Setup.h
 *
 * After placing this file, rebuild your project to use this configuration
 */

// ============================================================================
// Display Configuration
// ============================================================================
// ILI9341 display definition
#define ILI9341_DRIVER

// SPI pins
#define TFT_MOSI 11   // GPIO 11 (DIN)
#define TFT_MISO 13   // GPIO 13 (DO)
#define TFT_SCLK 12   // GPIO 12 (CLK)
#define TFT_CS 10     // CS pin for Display 1 (will be changed per-display)
#define TFT_DC 14     // GPIO 14
#define TFT_RST 21    // GPIO 21

// Display dimensions
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

// Rotation
#define TFT_ROTATION 0  // 0=portrait, 1=landscape, 2=inverted, 3=inverted landscape

// ============================================================================
// SPI Bus Configuration
// ============================================================================
// Use default SPI0 (ESP32 standard)
#define SPI_FREQUENCY 40000000  // 40MHz SPI clock

// Use DMA (Direct Memory Access) for faster transfers
#define SPI_READ_FREQUENCY  6000000  // Slower for reading

// ============================================================================
// Touch Configuration (for XPT2046 touch screens)
// ============================================================================
// Note: Touch screens require separate implementation
// TFT_eSPI does not include XPT2046 support by default
// See touch_handler.h for custom implementation

// ============================================================================
// Font Selection
// ============================================================================
// Use built-in fonts (smooth fonts provide better quality)
#define LOAD_GLCD       // Default 5x7 pixel font - always loaded
#define LOAD_FONT2      // Small 16 pixel high font
#define LOAD_FONT4      // Medium 26 pixel high font
#define LOAD_FONT6      // Large 48 pixel high font
#define LOAD_FONT7      // 7 segment 48 pixel font
#define LOAD_FONT8      // Large 75 pixel font (smooth)

// ============================================================================
// Color Definitions (16-bit RGB565)
// ============================================================================
// Format: 0xRRRGGGBB (5 bits R, 6 bits G, 5 bits B)
// Black   0x0000, Red     0xF800, Green   0x07E0, Blue    0x001F
// White   0xFFFF, Yellow  0xFFE0, Cyan    0x07FF, Magenta 0xF81F

// ============================================================================
// Performance Options
// ============================================================================
// Partial update support (faster updates for areas)
#define TFT_HARDWARE_SPI

// Enable double-buffering for flicker-free updates
// Note: Requires more PSRAM
#define SUPPORT_TRANSACTIONS

// ============================================================================
// Feature Flags
// ============================================================================
// Disable SPIFFS support (we use SD card instead)
#define DISABLE_SD_SPI_CS_CONTROL

// ============================================================================
// Backlight Control
// ============================================================================
// LED backlight on GPIO 47
// Control with PWM (already configured in display_manager.cpp)
// ledcSetup(0, 5000, 8);     // Channel 0, 5kHz, 8-bit resolution
// ledcAttachPin(47, 0);      // Attach GPIO 47 to channel 0

// ============================================================================
// Debug Options (comment out for production)
// ============================================================================
// #define DEBUG_TFT_INIT    // Print initialization debug messages

#endif // USER_SETUP_H
