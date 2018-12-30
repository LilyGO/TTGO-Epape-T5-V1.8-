/*
 Name:    dht22-demo.ino
 Created: 28.12.2018 16:52:02
 Author:  mgr
*/

#include "DHT.h"
#include <SPI.h>
#include "epd2in13.h"
#include "epdpaint.h"

// time used to deep sleep
#define SLEEP_TIME 5e6

#define COLORED     0
#define UNCOLORED   1
/************************* Temp sensor config ********************************/
#define DHTPIN 21
#define DHTTYPE DHT22

// This variable will survive deep sleep, they are stored in RTC memory
RTC_DATA_ATTR float temp = 0;
RTC_DATA_ATTR float humi = 0;

char tbs[16];

// painitng buffer size, now, it's defined to support entire screen at a time, you can decrese it's size
// but then, remember not to pay attention during painting, to not to move over the array.
unsigned char image[32000];
Paint paint(image, 0, 0);

DHT dht(DHTPIN, DHTTYPE);
Epd epd;

void setup() {
  Serial.begin(115200);
  Serial.println();

  if (temp == 0)
  {
    Serial.println("Full update");
    // First boot, clear entire display
    if (epd.Init(lut_full_update) != 0) {
      Serial.println("e-Paper init failed");
      return;
    }

    epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
    epd.DisplayFrame();
    epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
    epd.DisplayFrame();
  }
  else
  {
    // Boot from deep sleep, no need to update entire display, just update partial.
    if (epd.Init(lut_partial_update) != 0) {
      Serial.println("e-Paper init failed");
      return;
    }
  }

  dht.begin();

  float h, t;
  do
  {
    h = dht.readHumidity();
    t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      delay(2500);
    }
  } while ((isnan(h) || isnan(t)));

  sprintf(tbs, "%.1f °C %.f %%", t, h);
  Serial.println("\nCurrent sensor values: ");
  Serial.println(tbs);

  if (temp != t || humi != h)
  {
    temp = t;
    humi = h;

    Serial.println("Need update and upload...");
    paint.SetRotate(ROTATE_0);
    paint.SetWidth(120);    // width should be the multiple of 8 
    paint.SetHeight(250);
    paint.Clear(UNCOLORED);
    paint.SetRotate(ROTATE_90);

    sprintf(tbs, "Temp: %.1f*C", t);
    paint.DrawStringAt(0, 10, tbs, &Font24, COLORED);

    sprintf(tbs, "Humidity: %.f%%", h);
    paint.DrawStringAt(0, 48, tbs, &Font24, COLORED);

    // there are two data buffers inside display, to get stable image we need to send it twice.
    epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();
    epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();
  }

  Serial.println("Setup ESP32 to sleep...");
  delay(500);

  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() 
{
}
