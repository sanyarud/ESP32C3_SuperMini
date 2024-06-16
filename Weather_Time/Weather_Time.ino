#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const char* ssid = "Name_WIFI";
const char* password = "Passsssss";
const char* apiKey = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* city = "Vinnytsia";

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

#define SUN 0
#define SUN_CLOUD 1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4

uint8_t symbol = SUN;
float temp = 0;
char weatherData[256] = "Loading weather...";  // Змінна для збереження даних про погоду
bool weatherUpdated = false;  // Флаг для перевірки, чи оновлено дані про погоду

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.google.com", 10800, 60000); // Часовий пояс Київ (UTC+3)

int16_t scrollOffset = 0;

void drawWeatherSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol) {
  switch (symbol) {
    case SUN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 69);
      break;
    case SUN_CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 65);
      break;
    case CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 64);
      break;
    case RAIN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 67);
      break;
    case THUNDER:
      u8g2.setFont(u8g2_font_open_iconic_embedded_6x_t);
      u8g2.drawGlyph(x, y, 67);
      break;
  }
}

void drawWeather(uint8_t symbol, int degree) {
  drawWeatherSymbol(0, 48, symbol);
  u8g2.setFont(u8g2_font_logisoso32_tf);
  u8g2.setCursor(48 + 3, 42);
  u8g2.print(degree);
  u8g2.print("°C");
}

void drawScrollString(int16_t offset, const char *s) {
  static char buf[36];
  size_t len;
  size_t char_offset = 0;
  u8g2_uint_t dx = 0;
  size_t visible = 0;

  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 49, u8g2.getDisplayWidth() - 1, u8g2.getDisplayHeight() - 1);
  u8g2.setDrawColor(1);

  len = strlen(s);
  if (offset < 0) {
    char_offset = (-offset) / 8;
    dx = offset + char_offset * 8;
    if (char_offset >= u8g2.getDisplayWidth() / 8)
      return;
    visible = u8g2.getDisplayWidth() / 8 - char_offset + 1;
    strncpy(buf, s, visible);
    buf[visible] = '\0';
    u8g2.setFont(u8g2_font_8x13_mf);
    u8g2.drawStr(char_offset * 8 - dx, 62, buf);
  } else {
    char_offset = offset / 8;
    if (char_offset >= len)
      return;
    dx = offset - char_offset * 8;
    visible = len - char_offset;
    if (visible > u8g2.getDisplayWidth() / 8 + 1)
      visible = u8g2.getDisplayWidth() / 8 + 1;
    strncpy(buf, s + char_offset, visible);
    buf[visible] = '\0';
    u8g2.setFont(u8g2_font_8x13_mf);
    u8g2.drawStr(-dx, 62, buf);
  }
}

void displayWeatherInfo() {
  static int16_t offset = scrollOffset;
  int16_t len = strlen(weatherData);

  u8g2.clearBuffer();
  drawWeather(symbol, temp);
  drawScrollString(offset, weatherData);
  u8g2.sendBuffer();

  delay(5);
  offset += 6;  // Прискорити скролінг в 3 рази
  if (offset > len * 8) {
    offset = -(int16_t)u8g2.getDisplayWidth();
  }
  scrollOffset = offset;
}

void displayMessage(const char *message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_mf); // Менший шрифт
  int16_t x = (u8g2.getDisplayWidth() - u8g2.getStrWidth(message)) / 2;
  int16_t y = (u8g2.getDisplayHeight() / 2) - (u8g2.getMaxCharHeight() / 2);
  u8g2.drawStr(x, y, message);
  u8g2.sendBuffer();
}

void displayTime() {
  u8g2.clearBuffer();
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  u8g2.setFont(u8g2_font_logisoso24_tf);  // Зменшений шрифт для часу
  int16_t timeX = (u8g2.getDisplayWidth() - u8g2.getStrWidth(formattedTime.c_str())) / 2;
  u8g2.drawStr(timeX, 32, formattedTime.c_str());

  u8g2.setFont(u8g2_font_6x10_mf);
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime(&rawtime);

  char dateBuffer[20];
  strftime(dateBuffer, sizeof(dateBuffer), "%d-%m-%Y", ti);
  int16_t dateX = (u8g2.getDisplayWidth() - u8g2.getStrWidth(dateBuffer)) / 2;
  u8g2.drawStr(dateX, 50, dateBuffer);

  char dayBuffer[20];
  strftime(dayBuffer, sizeof(dayBuffer), "%A", ti);
  int16_t dayX = (u8g2.getDisplayWidth() - u8g2.getStrWidth(dayBuffer)) / 2;
  u8g2.drawStr(dayX, 60, dayBuffer);

  u8g2.sendBuffer();
}

void setup() {
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);

  digitalWrite(10, 0);
  digitalWrite(9, 0);

  Serial.begin(115200);
  u8g2.begin();
  u8g2.enableUTF8Print();

  displayMessage("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
  char connectedMsg[64];
  snprintf(connectedMsg, sizeof(connectedMsg), "Connected to %s", ssid);
  displayMessage(connectedMsg);
  delay(1000); // Пауза для відображення повідомлення

  timeClient.begin();
}

void loop() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastSwitch = 0;
  const unsigned long updateInterval = 300000; // Оновлювати кожні 5 хвилин
  const unsigned long displayInterval = 30000; // Перемикатися кожні 10 секунд

  if (millis() - lastUpdate >= updateInterval || !weatherUpdated) {
    lastUpdate = millis();

    if ((WiFi.status() == WL_CONNECTED)) {
      displayMessage("Updating Weather...");
      HTTPClient http;
      String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&units=metric&appid=" + apiKey;
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println(payload); // Друк отриманих даних для перевірки
        DynamicJsonDocument doc(2048); // Збільшений розмір буфера
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
        } else {
          temp = doc["main"]["temp"];
          float temp_min = doc["main"]["temp_min"];
          float temp_max = doc["main"]["temp_max"];
          float wind_speed = doc["wind"]["speed"];
          const char* description = doc["weather"][0]["description"];

          snprintf(weatherData, sizeof(weatherData), "Desc: %s, Wind: %.2f m/s, Min: %.2f°C, Max: %.2f°C",
                   description, wind_speed, temp_min, temp_max);

          const char* weather = doc["weather"][0]["main"];
          if (strcmp(weather, "Clear") == 0) {
            symbol = SUN;
          } else if (strcmp(weather, "Clouds") == 0) {
            symbol = CLOUD;
          } else if (strcmp(weather, "Rain") == 0) {
            symbol = RAIN;
          } else if (strcmp(weather, "Thunderstorm") == 0) {
            symbol = THUNDER;
          } else {
            symbol = SUN_CLOUD;
          }
          weatherUpdated = true;
        }
      } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpCode);
      }

      http.end();
    } else {
      Serial.println("WiFi not connected");
    }
  }

  // Перемикання між погодою та часом
  if (millis() - lastSwitch >= displayInterval) {
    lastSwitch = millis();
    static bool showWeather = true;
    showWeather = !showWeather;

    if (showWeather) {
      displayWeatherInfo();
    } else {
      displayTime();
    }
  } else {
    if (millis() - lastSwitch < displayInterval / 2) {
      displayWeatherInfo();
    } else {
      displayTime();
    }
  }
}
