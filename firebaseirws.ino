#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_NeoPixel.h>

// Konfigurasi WiFi
#define WIFI_SSID "hm"
#define WIFI_PASSWORD "123490016"

// Konfigurasi Firebase
#define API_KEY "AIzaSyDQta2iiCWHvrjzJW649XK5MEEL2asfOvU"
#define DATABASE_URL "https://pwiotdwifa-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Pin sensor dan WS2812
#define IR_SENSOR_PIN 18  // Ganti dengan pin sensor infrared
#define WS2812_PIN 37  // Ganti dengan pin yang sesuai untuk WS2812
#define NUM_LEDS 1

Adafruit_NeoPixel strip(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);

unsigned long sendDataPrevMillis = 0;

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // Firebase Setup
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    Firebase.signUp(&config, &auth, "", "");
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    pinMode(IR_SENSOR_PIN, INPUT);
    
    strip.begin();
    strip.show(); // Matikan semua LED awalnya
}

void setColor(uint8_t red, uint8_t green, uint8_t blue) {
    strip.setPixelColor(0, strip.Color(red, green, blue));
    strip.show();
}

void loop() {
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000)) {
        sendDataPrevMillis = millis();
        
        int sensorValue = digitalRead(IR_SENSOR_PIN);
        Serial.print("Sensor IR: ");
        Serial.println(sensorValue);

        // Logika WS2812: Merah jika objek terdeteksi, Hijau jika tidak
        if (sensorValue == LOW) {  // Objek terdeteksi
            setColor(255, 0, 0); // Merah
            Serial.println("Lampu Merah (Objek terdeteksi)");
        } else {  // Tidak ada objek
            setColor(0, 255, 0); // Hijau
            Serial.println("Lampu Hijau (Tidak ada objek)");
        }

        // Kirim data ke Firebase
        FirebaseJson json;
        json.set("object_detected", sensorValue == LOW ? "Yes" : "No");
        json.set("lamp_status", sensorValue == LOW ? "Red" : "Green");

        if (Firebase.RTDB.setJSON(&fbdo, "/records/" + String(millis()), &json)) {
            Serial.println("Data berhasil dikirim ke Firebase");
        } else {
            Serial.println("Gagal mengirim data");
            Serial.printf("Error: %s\n", fbdo.errorReason().c_str());
        }
    }
    delay(1000);
}
