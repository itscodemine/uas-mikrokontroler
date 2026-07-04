/*
 * Proyek Akhir Sistem Mikrokontroler
 * Topik: Anti-theft System untuk Kendaraan Berbasis IoT
 */

// Kredensial project Blynk
#define BLYNK_TEMPLATE_ID "TMPL6-xmiRzwI"
#define BLYNK_TEMPLATE_NAME "Anti Theft System untuk Kendaraan Berbasis IoT"
#define BLYNK_AUTH_TOKEN "rpTzPFvgsXKdETzeYeuDOdbwGS5kWCLA"

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

Adafruit_MPU6050 mpu;

// Deklarasi pin yang digunakan untuk aktuator
#define BUZZER_PIN 5
#define LED_PIN 4

// Variabel Global
bool systemArmed = false;
float base_x = 0, base_y = 0, base_z = 0;

void setup() {
  Serial.begin(115200);

  // Mengatur fungsi pin sebagai output
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Memulai koneksi ke Blynk
  Serial.println("Menghubungkan ke WiFi dan Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Inisialisasi Sensor MPU6050
  if (!mpu.begin()) {
    Serial.println("GAGAL: MPU6050 tidak terdeteksi. Cek kabel SDA/SCL!");
    while (1) { delay(10); }
  }
  Serial.println("MPU6050 Siap!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
}

// Fungsi kontrol dari dashboard Blynk
BLYNK_WRITE(V0) {
  systemArmed = param.asInt();

  if (systemArmed) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Mode Siaga AKTIF. Merekam posisi parkir...");
    Blynk.virtualWrite(V1, "🛡️ SIAGA (KONDISI AMAN)"); 
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    base_x = a.acceleration.x;
    base_y = a.acceleration.y;
    base_z = a.acceleration.z;
    
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("Mode Siaga DIMATIKAN.");
    Blynk.virtualWrite(V1, "⚪ SISTEM NON-AKTIF"); 
  }
}

void loop() {
  Blynk.run();

  if (systemArmed) {
    // Pembacaan data sensor MPU6050
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float diff_x = abs(a.acceleration.x - base_x);
    float diff_y = abs(a.acceleration.y - base_y);
    float diff_z = abs(a.acceleration.z - base_z);

    // Perhitungan kemiringan dan pergerakan untuk memicu alarm
    if (diff_x > 3.0 || diff_y > 3.0 || diff_z > 3.0) {
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("BAHAYA! Pergerakan mencurigakan!");

      // Mengirim notifikasi dan status ke dashboard Blynk
      Blynk.logEvent("theft_alert", "Terdeteksi pergerakan mencurigakan pada kendaraan!");
      Blynk.virtualWrite(V1, "🚨 BAHAYA! KENDARAAN DIGESER! 🚨"); 
      
      for(int i = 0; i < 4; i++) {
        delay(1000);
        Blynk.run(); 
      }
      
      // Mematikan alarm dan kalibrasi ulang data sensor
      digitalWrite(BUZZER_PIN, LOW);
      Blynk.virtualWrite(V1, "🛡️ SIAGA (MENUNGGU PEMERIKSAAN)"); 
      
      sensors_event_t a2, g2, temp2;
      mpu.getEvent(&a2, &g2, &temp2);
      base_x = a2.acceleration.x;
      base_y = a2.acceleration.y;
      base_z = a2.acceleration.z;
    }
  }
}
