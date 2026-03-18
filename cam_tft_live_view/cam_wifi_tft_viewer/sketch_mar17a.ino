#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#define BUF 40000

// wifi
const char* ssid = "ssid";
const char* password = "password";


const char* camIP = "ip from cam_wifi_http_server serial";

TFT_eSPI tft = TFT_eSPI();

uint8_t *buf = NULL;


bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  TJpgDec.setCallback(tft_output);
  TJpgDec.setJpgScale(1);

  buf = (uint8_t*) malloc(BUF);
  if (!buf) {
    Serial.println("not enough RAM for buffer");
    while (1);
  }

  WiFi.begin(ssid, password);
  Serial.print("WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nOK!");
}

void loop() {
  HTTPClient http;

  String url = String("http://") + camIP + "/capture";
  http.begin(url);

  int httpCode = http.GET();
  Serial.println(httpCode);

  if (httpCode == 200) {
    WiFiClient *stream = http.getStreamPtr();

    int index = 0;
    unsigned long start = millis();

    while (http.connected() && millis() - start < 30) {
      int available = stream->available();

      if (available) {
        int read = stream->readBytes(buf + index, available);
        index += read;

      }
    }

    Serial.print("JPG size: ");
    Serial.println(index);

    if (index > 0) {
      TJpgDec.drawJpg(0, 0, buf, index);
    }
  }
 
  http.end();

  delay(30);
}
