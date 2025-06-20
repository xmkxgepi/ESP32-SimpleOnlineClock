#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SSD1306Wire.h>

#include <NTPClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


SSD1306Wire display(0x3c, 23, 22);

HTTPClient http;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 8 *3600);

String city;
String temp;
String text;

int hours = 0;
int minutes = 0;
int seconds = 0;

unsigned long lastUpdateTime = 0;
long updateInterval = 3 * 60 * 1000;

void netConfig() {


  int count = 0;
  String ssid = "";
  String password = "";

  display.clear();

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED && count <= 20) {
    delay(500);
    count++;

    String dots = "";
    for (int i = 0; i < count; i++) {
      dots += ".";
    }
    
    display.clear();
    display.drawString(0, 0, "CONNECT TO\n" + String(ssid) + "\n" + String(dots));
    display.display();
  }

  display.clear();

  if(WiFi.status() == WL_CONNECTED) {
    display.drawString(0, 0, "CONNECTED\nDownload data in 1.5s");
    display.display();
    delay(1500);
    
    syncWeather();
    syncTime();

  } else {
    display.drawString(0, 0, "CONNECT FAILED\nPlease check router\nRetry in 5s");
    display.display();
    delay(5000);
    netConfig();
  }
}

void syncTime() {
  timeClient.begin();
  if(timeClient.update()) {
    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
    seconds = timeClient.getSeconds();
    
    Serial.printf("NTP时间同步成功: %02d:%02d:%02d\n", hours, minutes, seconds);

  } else {
    Serial.println("NTP时间同步失败");
  }
}

void syncWeather() {

  String apiurl = "https://api.seniverse.com/v3/weather/now.json?";
  String location = "";
  String key = "";
  String language = "en";
  String url = apiurl + "key=" + key + "&location=" + location + "&language=" + language + "&unit=c";

  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {

    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    city = String(doc["results"][0]["location"]["name"]);
    temp = String(doc["results"][0]["now"]["temperature"]);
    text = String(doc["results"][0]["now"]["text"]);

    displayWeather();
  } else {

    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "WEATHER DATA\nUNAVAILABLE");

  }
}

void displayWeather() {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "TEMP:" + String(temp) + "\n" + String(text));
    display.display();
}

void setup() {
  Serial.begin(9600);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "[ChuMai Clock]");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 30, "©QuanHexFata");
  display.display();

  delay(3000);
  netConfig();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentMillis;
    syncWeather();
    syncTime();
  }

}