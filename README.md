# Weather and Time Display on ESP32C2 SuperMini

This project displays the current weather and time on an SSD1306 OLED display using an ESP32. It fetches weather data from OpenWeatherMap and the current time from Google's NTP servers.

[Watch the video demonstration](https://github.com/sanyarud/ESP32C3_SuperMini/blob/main/IMG_2744.MOV)

![ESP32C3 SuperMini](https://github.com/sanyarud/ESP32C3_SuperMini/blob/main/img.webp)


## Components

- ESP32С3 Super Mini
- SSD1306 OLED Display
- WiFi connection

## Libraries Required

Install the following libraries in the Arduino IDE:

   - [WiFi](https://www.arduino.cc/en/Reference/WiFi)
   - [HTTPClient](https://www.arduino.cc/reference/en/libraries/httpclient/)
   - [ArduinoJson](https://arduinojson.org/)
   - [U8g2](https://github.com/olikraus/u8g2)
   - [NTPClient](https://github.com/arduino-libraries/NTPClient)
   - [WiFiUdp](https://www.arduino.cc/en/Reference/WiFiUDP)

## Connections

| ESP32 Pin | OLED Pin |
|-----------|----------|
| GND       | GND      |
| VCC       | VCC      |
| SDA       | SDA      |
| SCL       | SCL      |

## How to Use

1. Connect your ESP32 to your computer.
2. Open the Arduino IDE and install the required libraries.
3. Copy and paste the code into the Arduino IDE.
4. Replace `YOUR_SSID`, `YOUR_PASSWORD`, `YOUR_API_KEY`, and `YOUR_CITY` with your actual WiFi credentials, [OpenWeatherMap](https://openweathermap.org/) API key, and desired city.
5. Upload the code to your ESP32.
6. Open the Serial Monitor to check the status of your connection and data fetching.
