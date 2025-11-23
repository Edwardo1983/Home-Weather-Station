# Weather API Setup Guide

Complete instructions for registering and configuring weather API services for the Home Weather Station.

## Overview

The system integrates with two weather APIs:

1. **OpenWeatherMap** - Primary weather data (current + 5-day forecast)
2. **Tomorrow.io** - Backup precipitation data (minute-by-minute detail)

Both offer free tiers suitable for personal weather stations.

---

## OpenWeatherMap API

### Step 1: Create Account

1. Visit: https://openweathermap.org/users/register
2. Fill in registration form:
   - Email address
   - Username
   - Password
   - Accept terms
3. Check email for verification link
4. Click verification link to activate account

### Step 2: Get API Key

1. Log in to OpenWeatherMap: https://openweathermap.org/api
2. Click on your account (top-right) → "My API Keys"
3. Default API key should be visible
4. **Copy the API key** (long string of characters)

### Step 3: Verify API Key Works

Test in browser (replace YOUR_KEY):
```
https://api.openweathermap.org/data/2.5/weather?lat=44.4268&lon=26.1025&units=metric&appid=YOUR_KEY
```

You should see JSON response with weather data.

### Step 4: Add to secrets.h

In `esp32s3_central/secrets.h`:

```cpp
#define OPENWEATHERMAP_API_KEY "your_actual_api_key_here_no_quotes"
```

Example:
```cpp
#define OPENWEATHERMAP_API_KEY "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6"
```

### API Features & Limits

| Feature | Free Tier | Cost |
|---------|-----------|------|
| Current weather | ✅ 1000/day | |
| Forecasts (5-day) | ✅ 1000/day | |
| Forecasts (16-day) | ❌ | €4.99/month |
| Forecasts (8-day climate) | ❌ | €4.99/month |
| Hourly forecasts | ❌ | €9.99/month |
| Alerts | ❌ | €9.99/month |

**Free Tier:** 1000 API calls per day (sufficient for personal use)

### API Rate Limiting

The system respects rate limits:
- Default: 15-minute update interval
- Maximum: 1 hour (if limited)
- Never exceeds 1000 calls/day

**Manual Refresh:** Display button refreshes immediately, counts toward daily limit.

### Data Returned

Current weather:
- Temperature (°C)
- Feels like (°C)
- Humidity (%)
- Pressure (hPa)
- Weather condition (Clear, Cloudy, Rainy, etc.)
- Wind speed (m/s)
- Cloudiness (%)
- Visibility (m)

5-Day forecast:
- Daily min/max temperature
- Probability of precipitation
- Weather condition for each day
- Humidity
- Wind speed

---

## Tomorrow.io API

### Step 1: Create Account

1. Visit: https://www.tomorrow.io/weather-api/
2. Click "Get Free Account"
3. Fill registration:
   - Email
   - Password
   - Company/Organization (optional)
   - Country
4. Verify email

### Step 2: Get API Key

1. Log in to Tomorrow.io dashboard
2. Go to "API Keys" section
3. Default API key provided
4. **Copy the API key**

### Step 3: Verify API Key

Test in browser (replace YOUR_KEY):
```
https://api.tomorrow.io/v4/weather/realtime?location=44.4268,26.1025&apikey=YOUR_KEY
```

You should see JSON response with weather data.

### Step 4: Add to secrets.h

In `esp32s3_central/secrets.h`:

```cpp
#define TOMORROW_IO_API_KEY "your_actual_api_key_here"
```

### API Features & Limits

| Feature | Free Tier | Cost |
|---------|-----------|------|
| Current weather | ✅ 500/day | |
| Realtime (nowcast) | ✅ 500/day | |
| Hourly forecast (7 days) | ✅ 500/day | |
| Daily forecast (10 days) | ✅ 500/day | |
| Minute-by-minute (1 hour) | ✅ 500/day | |
| Advanced models | ❌ | $9-99/month |

**Free Tier:** 500 API calls per day

### Special Features

Tomorrow.io excels at:
- **Minute-by-minute precipitation forecasts** (perfect for rain alerts)
- **Real-time precipitation** (current rainfall)
- **High-resolution weather** (grid: 0.4 km)
- **Pollen forecasts** (if enabled)
- **Air quality** (if enabled)

### Data Returned

- Temperature
- Humidity
- Precipitation (mm/hour)
- Wind speed & direction
- Weather conditions
- UV index
- Visibility
- Pressure
- Cloud cover

---

## Dual-API Strategy

The system uses both APIs for redundancy and better predictions:

### Primary: OpenWeatherMap
- More stable and widely used
- Good 5-day forecasts
- Handles alerts

### Secondary/Backup: Tomorrow.io
- Better precipitation detail
- Minute-by-minute forecasting
- Pollen & air quality data

### Fallback Logic

```cpp
// System attempts in order:
1. OpenWeatherMap current weather
2. OpenWeatherMap 5-day forecast
3. If OWM fails → Tomorrow.io realtime (fallback)
4. If both fail → Display last known data + alert
```

---

## Location Configuration

Set your location in `secrets.h`:

```cpp
#define LATITUDE 44.4268      // Degrees, decimal format
#define LONGITUDE 26.1025     // Degrees, decimal format
#define LOCATION_NAME "Bucharest, RO"
```

### How to Find Coordinates

**Method 1: Google Maps**
1. Right-click on location → "What's here?"
2. Coordinates shown in search box

**Method 2: OpenStreetMap**
1. Visit https://www.openstreetmap.org
2. Search location
3. Coordinates in sidebar

**Method 3: Decimal Converter**
- Convert from DMS: 44°25'36.5"N → 44.4268°N
- Online converter: https://coordinates-converter.com

### Common Locations

| City | Latitude | Longitude |
|------|----------|-----------|
| Bucharest, Romania | 44.4268 | 26.1025 |
| London, UK | 51.5074 | -0.1278 |
| Paris, France | 48.8566 | 2.3522 |
| New York, USA | 40.7128 | -74.0060 |
| Tokyo, Japan | 35.6762 | 139.6503 |

---

## API Usage Monitoring

### OpenWeatherMap

Check API usage:
1. Login to openweathermap.org
2. Account → API Keys
3. Usage statistics shown below key

### Tomorrow.io

Check API usage:
1. Login to dashboard
2. "API Keys" → Select key
3. Usage shown in metrics

---

## Cost Estimation

### Monthly API Call Budget

**Activity Pattern:**
- Local sensors: read every 60 seconds
- API calls: every 15 minutes
- Remote nodes: poll every 5 minutes
- Data logging: every 5 minutes

**Daily Calls:**
- OpenWeatherMap: 96 calls (24h × 4 calls/hour)
- Tomorrow.io: 96 calls (same pattern)

**Monthly:**
- OpenWeatherMap: ~2,880 calls (well under 30,000 free limit)
- Tomorrow.io: ~2,880 calls (well under 15,000 free limit)

**Cost:** $0 (both free tiers sufficient)

---

## Advanced Configuration

### Custom Weather Codes

The system maps standard weather codes to icons:

| Code | Condition | Icon |
|------|-----------|------|
| 200-299 | Thunderstorm | ⚡ |
| 300-399 | Drizzle | 🌧️ |
| 500-599 | Rain | 🌧️ |
| 600-699 | Snow | ❄️ |
| 700-799 | Fog | 🌫️ |
| 800 | Clear | ☀️ |
| 801 | Partly Cloudy | ⛅ |
| 802+ | Cloudy | ☁️ |

### Caching Strategy

API responses cached locally:
- **TTL (Time To Live):** 15 minutes
- **Force refresh:** Manual display button
- **Auto-refresh:** On schedule

### Error Handling

If API fails:
- Last known data displayed
- System attempts fallback API
- Alert shown to user
- Continues operating with cached data

---

## Troubleshooting

### "API Key Invalid"

1. [ ] Verify key copied exactly (no spaces)
2. [ ] Check key is in secrets.h (not in comments)
3. [ ] Regenerate key in OpenWeatherMap panel
4. [ ] Check for key expiration (free tier: never)

### "Too Many Requests" (429 error)

1. [ ] Check daily API call count
2. [ ] Increase update interval (15→30 min)
3. [ ] Disable manual refresh temporarily
4. [ ] Upgrade API plan if needed

### "Location Not Found"

1. [ ] Verify latitude/longitude correct (use Google Maps)
2. [ ] Check decimal format (not DMS)
3. [ ] Ensure location within API coverage (almost worldwide)

### No Weather Data Displayed

1. [ ] Check WiFi connected (serial output)
2. [ ] Verify API keys in secrets.h
3. [ ] Monitor Serial → watch for API errors
4. [ ] Test API manually in browser (see verification steps)

### Serial Output Shows API Errors

**Enable debugging in config.h:**
```cpp
#define DEBUG_WEATHER_API true
```

Then monitor Serial at 115200 baud to see:
- HTTP status codes
- JSON parse errors
- API response data

---

## API Reference Links

### OpenWeatherMap

- **API Docs:** https://openweathermap.org/api
- **Current Weather:** https://openweathermap.org/current
- **Forecast:** https://openweathermap.org/forecast5
- **API Console:** https://openweathermap.org/find
- **Status:** https://status.openweathermap.org

### Tomorrow.io

- **API Docs:** https://docs.tomorrow.io/reference/
- **Realtime:** https://docs.tomorrow.io/reference/realtime-weather-api
- **Forecast:** https://docs.tomorrow.io/reference/forecast-api
- **Nowcast:** https://docs.tomorrow.io/reference/nowcast-api
- **Status:** https://status.tomorrow.io

---

## Future API Integrations

Potential future additions:

- [ ] **WeatherAPI.com** - Astronomy data, air quality
- [ ] **Dark Sky (Powered)** - Hyperlocal forecasts
- [ ] **National Weather Services** - Official government data
- [ ] **Custom Local API** - Built from SD card historical data
- [ ] **MQTT Integration** - Subscribe to weather topics

---

## Data Privacy

**What data is sent to APIs:**
- Latitude/Longitude
- Timestamp of request
- API key (encrypted over HTTPS)

**Data NOT sent:**
- Indoor sensor readings (private)
- Logged data on SD card (stays local)
- Heart rate/SpO2 data (stays local)

**API Provider Policies:**
- OpenWeatherMap: https://openweathermap.org/privacy
- Tomorrow.io: https://www.tomorrow.io/privacy/

---

**Last Updated:** 2024-11-22
**Status:** Current as of API versions Nov 2024
