#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ===========================
// CAMERA MODEL
// ===========================
#include "board_config.h"

// ===========================
// WIFI
// ===========================
const char* ssid = "Mino";
const char* password = "AGoodPass";

// ===========================
// PUSHOVER
// ===========================
const char* PUSHOVER_USER  = "uhyyrgecr47jqjvbest4xyjddzhazu";
const char* PUSHOVER_TOKEN = "adsmxbvbnkkkoicphswtbhbyuwryvb";

// ===========================
// PIR
// ===========================
#define PIR_PIN 13
unsigned long lastMotionTime = 0;
const unsigned long motionCooldown = 10000; // 10 sec
bool lastPirState = LOW;
bool sendingInProgress = false;

void startCameraServer();
void setupLedFlash();

// ===========================
// SEND PUSHOVER PHOTO
// ===========================
void sendPushover(camera_fb_t* fb) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("❌ Wi-Fi not connected, skipping alert");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    if (!client.connect("api.pushover.net", 443)) {
        Serial.println("❌ Pushover connection failed");
        return;
    }

    String boundary = "ESP32CAM";

    String header =
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"token\"\r\n\r\n" +
        String(PUSHOVER_TOKEN) + "\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"user\"\r\n\r\n" +
        String(PUSHOVER_USER) + "\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"message\"\r\n\r\n"
        "⚠️ Motion detected!\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"attachment\"; filename=\"motion.jpg\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n";

    String footer = "\r\n--" + boundary + "--\r\n";

    client.print("POST /1/messages.json HTTP/1.1\r\n");
    client.print("Host: api.pushover.net\r\n");
    client.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
    client.print("Content-Length: " + String(header.length() + fb->len + footer.length()) + "\r\n\r\n");

    client.print(header);
    client.write(fb->buf, fb->len);
    client.print(footer);

    // wait for server response
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break;  // end of headers
    }

    Serial.println("📸 Pushover alert sent with image");
}



// ===========================
// SETUP
// ===========================
void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  // ===========================
  // CAMERA CONFIG
  // ===========================
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;  // MUCH safer
  config.jpeg_quality = 12;
  config.fb_count = 1;

  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Camera init failed");
    return;
  }

#if defined(LED_GPIO_NUM)
  setupLedFlash();
#endif

  // ===========================
  // WIFI
  // ===========================
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected");
  Serial.print("📡 IP: ");
  Serial.println(WiFi.localIP());

  // ===========================
  // START SERVER
  // ===========================
  startCameraServer();

  Serial.println("📷 Camera Web Server ready");
}

// ===========================
// LOOP
// ===========================
void loop() {
  bool pirState = digitalRead(PIR_PIN);

  if (!sendingInProgress &&
      pirState == HIGH &&
      lastPirState == LOW) {

    unsigned long now = millis();

    if (now - lastMotionTime > motionCooldown) {
      lastMotionTime = now;
      sendingInProgress = true;

      Serial.println("🚨 Motion detected,!");

      camera_fb_t* fb = esp_camera_fb_get();
      if (fb) {
        sendPushover(fb);
        esp_camera_fb_return(fb);
      }

      sendingInProgress = false;
    }
  }

  lastPirState = pirState;
  delay(50);
}
