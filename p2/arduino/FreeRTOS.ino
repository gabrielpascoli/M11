#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

const char *ssid = "Inteli.Iot";
const char *password = "@Intelix10T#";

#define CAMERA_MODEL_AI_THINKER

SemaphoreHandle_t semImageReady;
SemaphoreHandle_t semImageSent;
camera_fb_t* fb = NULL;


// URL do backend onde as imagens serão enviadas
const char* serverUrl = "http://10.128.0.83:8000/upload";

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  Serial.println("Camera init succeeded");

  semImageReady = xSemaphoreCreateBinary();
  semImageSent = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(captureImageTask, "CaptureImageTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(sendImageTask, "SendImageTask", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(receiveDetectionTask, "ReceiveDetectionTask", 2048, NULL, 1, NULL, 0);
}

void loop() {
  // Não precisa usar o loop
  delay(10000);
}

void captureImageTask(void *pvParameters) {
  for (;;) {
    fb = esp_camera_fb_get();
    if (fb) {
      xSemaphoreGive(semImageReady);
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
}

void sendImageTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(semImageReady, portMAX_DELAY) == pdTRUE) {
      sendImageToBackend();
      fb = NULL;
      xSemaphoreGive(semImageSent);
    }
  }
}

void receiveDetectionTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(semImageSent, portMAX_DELAY) == pdTRUE) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://10.128.0.83:8000/get_faces");  // Substitua pela URL correta
        int httpResponseCode = http.GET();
        
        if (httpResponseCode == 200) {
          String response = http.getString();
          Serial.println(response); // Aqui você processaria a resposta para extrair as coordenadas
        } else {
          Serial.print("GET request failed with error: ");
          Serial.println(httpResponseCode);
        }
        
        http.end();
      } else {
        Serial.println("WiFi not connected");
      }
    }
  }
}


void sendImageToBackend() {
  if (WiFi.status() == WL_CONNECTED && fb != NULL) {
    HTTPClient http;
    http.begin(serverUrl);

    String boundary = "------------------------14737809831466499882746641449";
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);

    String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--" + boundary + "--\r\n";

    size_t allLen = head.length() + fb->len + tail.length();
    uint8_t *allBuf = (uint8_t *)malloc(allLen);
    if (!allBuf) {
      Serial.println("Failed to allocate memory");
      return;
    }

    memcpy(allBuf, head.c_str(), head.length());
    memcpy(allBuf + head.length(), fb->buf, fb->len);
    memcpy(allBuf + head.length() + fb->len, tail.c_str(), tail.length());

    int httpResponseCode = http.POST(allBuf, allLen);
    free(allBuf);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("HTTP error: ");
      Serial.println(httpResponseCode);
    }

    http.end();
    esp_camera_fb_return(fb);
  } else {
    Serial.println("WiFi not connected");
  }
}