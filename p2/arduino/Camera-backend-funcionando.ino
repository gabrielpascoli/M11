#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Inteli.Iot";
const char *password = "@Intelix10T#";

// URL do backend onde as imagens serão enviadas
//const char* serverUrl = "http://10.128.0.83:8000/upload"; //Relativo ao IP do backend

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
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  // Conectando ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");

  // Configuração da câmera
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

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Inicializa a câmera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Erro ao inicializar a câmera: 0x%x", err);
    return;
  }
  
  Serial.println("Câmera inicializada com sucesso!");
}

void loop() {
  // Captura um frame da câmera
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Falha ao capturar imagem");
    return;
  }

  // Envia a imagem capturada para o backend
  sendImageToBackend(fb);

  // Libera o frame buffer da câmera
  esp_camera_fb_return(fb);

  delay(5000); // Aguardar 5 segundos antes de capturar a próxima imagem
}

void sendImageToBackend(camera_fb_t * fb) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String serverUrl = "http://10.128.0.83:8000/upload"; //Relativo ao IP do backend
    http.begin(serverUrl);

    // Boundary para o multipart/form-data
    String boundary = "--------------------------133747188241686651551404";
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);

    // Montando o corpo da requisição no formato multipart/form-data
    String bodyStart = "--" + boundary + "\r\n" +
                       "Content-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\n" +
                       "Content-Type: image/jpeg\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    // Monta o corpo completo da requisição
    int totalLength = bodyStart.length() + fb->len + bodyEnd.length();

    // Cria um buffer para armazenar todo o corpo da requisição
    uint8_t *requestBody = (uint8_t *)malloc(totalLength);
    if (requestBody == NULL) {
      Serial.println("Erro ao alocar memória para o corpo da requisição");
      return;
    }

    // Copia o bodyStart, a imagem, e o bodyEnd para o buffer
    memcpy(requestBody, bodyStart.c_str(), bodyStart.length());
    memcpy(requestBody + bodyStart.length(), fb->buf, fb->len);
    memcpy(requestBody + bodyStart.length() + fb->len, bodyEnd.c_str(), bodyEnd.length());

    // Envia o POST com o corpo multipart completo
    int httpResponseCode = http.POST(requestBody, totalLength);

    // Libera o buffer de memória
    free(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode); // Código de resposta do servidor
      Serial.println(response);         // Resposta do servidor
    } else {
      Serial.printf("Erro no envio da imagem: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // Finaliza a conexão HTTP
    http.end();
  } else {
    Serial.println("Conexão Wi-Fi perdida");
  }
}


