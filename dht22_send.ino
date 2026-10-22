#include <WiFi.h>          // Gunakan <ESP8266WiFi.h> jika menggunakan NodeMCU ESP8266
#include <PubSubClient.h>
#include <DHT.h>

// --- Konfigurasi Wi-Fi & MQTT ---
const char* ssid         = "TECNO CAMON 40 Pro 5G";     // Ganti dengan SSID WiFi Anda
const char* password     = "santrifiwa"; // Ganti dengan Password WiFi Anda
const char* mqtt_server  = "test.mosquitto.org"; // Contoh: "192.168.1.50"
const int mqtt_port      = 1883;

// --- Konfigurasi DHT22 ---
#define DHTPIN 14          // Hubungkan pin data DHT22 ke Pin 14
#define DHTTYPE DHT22      // Menentukan tipe sensor yaitu DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Variabel Timer ---
unsigned long lastMsg = 0;
const long interval = 5000; // Mengirim data setiap 5000 milidetik (5 detik)

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi untuk menghubungkan ke Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menghubungkan ulang ke MQTT Broker jika terputus
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected to Mosquitto");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin(); // Mengaktifkan sensor DHT22
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Pastikan tetap terhubung ke Mosquitto broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  // Mengirim data secara berkala sesuai nilai 'interval' tanpa memblokir program
  if (now - lastMsg > interval) {
    lastMsg = now;

    // 1. Membaca data dari sensor DHT22
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Cek apakah pembacaan sensor gagal
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // 2. Format data menjadi string JSON agar rapi
    // Contoh output: {"suhu": 27.50, "kelembapan": 65.20}
    String payload = "{\"suhu\": " + String(temperature) + ", \"kelembapan\": " + String(humidity) + "}";
    
    Serial.print("Sending data: ");
    Serial.println(payload);

    // 3. Mengirim data ke Mosquitto Broker
    // Topic: /santrifiwa
    if (client.publish("/santrifiwa", payload.c_str())) {
      Serial.println("MQTT Publish Success!");
    } else {
      Serial.println("MQTT Publish Failed!");
    }
  }
}
