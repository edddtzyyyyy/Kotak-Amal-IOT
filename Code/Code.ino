#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <Adafruit_TCS34725.h>
#include <ezTime.h>
#include <map>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor warna
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_600MS, TCS34725_GAIN_1X);

// IR Sensor
#define TCRT_PIN 25

// Buzzer
#define BUZZER_PIN 13

// WiFi & Telegram
const char* ssid = "<GANTI_DENGAN_NAMA_WIFI_ANDA>";
const char* password = "<GANTI_DENGAN_PASSWORD_WIFI_ANDA>"; 
String chat_id = "<GANTI_DENGAN_CHAT_ID_ANDA>";
String bot_token = "<GANTI_DENGAN_BOT_TOKEN_ANDA>";


// Firebase
String firebaseUrl = "<GANTI_DENGAN_FIREBASE_URL_ANDA>";
String firebaseApiKey = "<GANTI_DENGAN_FIREBASE_API_KEY_ANDA>";


// ezTime
Timezone myTZ;

// Gunakan RTC untuk menyimpan data
RTC_DATA_ATTR int totalSaldo = 0;
RTC_DATA_ATTR int totalUangTerdeteksi = 0;
RTC_DATA_ATTR int totalBendaAsing = 0;
RTC_DATA_ATTR int lastUpdateId = 0;
unsigned long lastCheck = 0;
long interval = 1500;
const long minFreeHeap = 20000;
bool wifiConnected = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TCRT_PIN, INPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.print("Last Update ID dari RTC: ");
  Serial.println(lastUpdateId);
  Serial.print("Saldo dari RTC: ");
  Serial.println(totalSaldo);
  Serial.print("Uang Terdeteksi dari RTC: ");
  Serial.println(totalUangTerdeteksi);
  Serial.print("Benda Asing dari RTC: ");
  Serial.println(totalBendaAsing);

  Wire.begin(18, 19); // OLED
  Wire1.begin(21, 22); // TCS34725
  Wire1.setTimeout(100);
  Wire1.setClock(100000);

  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Terhubung ke WiFi!");
    wifiConnected = true;
    waitForSync();
    myTZ.setLocation("Asia/Jakarta");
    Serial.println("✅ Waktu WIB: " + myTZ.dateTime("d/m/Y H:i:s"));
    Serial.println("Epoch time (WIB): " + String(myTZ.now()));
    ambilDataDariFirebase();
  } else {
    Serial.println("\n❌ Gagal terhubung ke WiFi. Menggunakan data RTC.");
    wifiConnected = false;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED gagal");
    while (1);
  }

  if (!tcs.begin(0x29, &Wire1)) {
    Serial.println("❌ TCS34725 gagal");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor Error");
    display.display();
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Sensor OK!");
  display.display();
  delay(1000);
}

void ambilDataDariFirebase() {
  if (!wifiConnected) {
    Serial.println("❌ WiFi tidak terhubung.");
    return;
  }

  HTTPClient http;
  String url = firebaseUrl + "saldo.json?auth=" + firebaseApiKey;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      totalSaldo = doc["totalSaldo"] | 0;
      totalUangTerdeteksi = doc["totalUangTerdeteksi"] | 0;
      totalBendaAsing = doc["totalBendaAsing"] | 0;
      Serial.println("Data dari Firebase:");
      Serial.print("Saldo: "); Serial.println(totalSaldo);
      Serial.print("Uang: "); Serial.println(totalUangTerdeteksi);
      Serial.print("Benda: "); Serial.println(totalBendaAsing);
    } else {
      Serial.println("❌ Gagal parsing JSON: " + String(error.c_str()));
    }
    doc.clear();
  } else {
    Serial.println("❌ Gagal ambil Firebase. Kode: " + String(httpCode));
  }
  http.end();
}

void kirimDataKeFirebase(String nominal, int nilai, bool isUang) {
  if (!wifiConnected) {
    Serial.println("❌ WiFi tidak terhubung.");
    return;
  }

  unsigned long epochTime = myTZ.now();
  if (epochTime < 1741890000) { // Validasi: epoch < 13 Juni 2025
    Serial.println("❌ Epoch time invalid: " + String(epochTime));
    waitForSync();
    myTZ.setLocation("Asia/Jakarta");
    epochTime = myTZ.now();
  }
  Serial.println("Mengirim data dengan epoch time (WIB): " + String(epochTime));
  Serial.println("Waktu WIB: " + myTZ.dateTime("d/m/Y H:i:s"));

  DynamicJsonDocument doc(256);
  doc["totalSaldo"] = totalSaldo;
  doc["totalUangTerdeteksi"] = totalUangTerdeteksi;
  doc["totalBendaAsing"] = totalBendaAsing;

  String json;
  serializeJson(doc, json);

  HTTPClient http;
  String url = firebaseUrl + "saldo.json?auth=" + firebaseApiKey;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setConnectTimeout(10000);
  int httpCode = http.PUT(json);
  if (httpCode == 200) {
    Serial.println("✅ Saldo ke Firebase: " + json);
  } else {
    Serial.println("❌ Gagal saldo Firebase. Kode: " + String(httpCode));
  }

  doc.clear();
  doc["timestamp"] = String(epochTime * 1000LL);
  doc["nominal"] = nominal;
  doc["nilai"] = nilai;
  doc["type"] = isUang ? "uang" : "benda";
  serializeJson(doc, json);

  String transaksiUrl = firebaseUrl + "transaksi/" + String(epochTime * 1000LL) + ".json?auth=" + firebaseApiKey;
  http.begin(transaksiUrl);
  http.addHeader("Content-Type", "application/json");
  httpCode = http.PUT(json);
  if (httpCode == 200) {
    Serial.println("✅ Transaksi ke Firebase: " + json);
  } else {
    Serial.println("❌ Gagal transaksi Firebase. Kode: " + String(httpCode));
  }
  http.end();
}

void kirimDataKeTelegram(String nominal, bool isUang) {
  String dataMsg = isUang ? "✅ Uang masuk: " + nominal : "🚨 Benda asing terdeteksi! Periksa kotak amal.";
  kirimKeTelegram(dataMsg);
  Serial.println("Data ke Telegram: " + dataMsg);
}

int nominalKeInt(String nominal) {
  if (nominal == "Rp 1.000") return 1000;
  if (nominal == "Rp 2.000") return 2000;
  if (nominal == "Rp 5.000") return 5000;
  if (nominal == "Rp 10.000") return 10000;
  if (nominal == "Rp 20.000") return 20000;
  if (nominal == "Rp 50.000") return 50000;
  if (nominal == "Rp 100.000") return 100000;
  return 0;
}

String deteksiUang(uint16_t r, uint16_t g, uint16_t b) {
  if (r > 1000 && r < 13000 && g > 1000 && g < 13000 && b > 1000 && b < 13000) {
    return "Rp 1.000";
  } else if (r > 3000 && r < 4200 && g > 2600 && g < 3300 && b > 1700 && b < 2350) {
    return "Rp 2.000";
  } else if (r > 6000 && r < 6555 && g > 3600 && g < 4600 && b > 2000 && b < 3400) {
    return "Rp 5.000";
  } else if (r > 6000 && r < 6555 && g > 3600 && g < 4600 && b > 2000 && b < 3400) {
    return "Rp 10.000";
  } else if (r > 6000 && r < 6555 && g > 3600 && g < 4600 && b > 2000 && b < 3400) {
    return "Rp 20.000";
  } else if (r > 3120 && r < 5300 && g > 3240 && g < 5200 && b > 2680 && b < 4520) {
    return "Rp 50.000";
  } else if (r > 3120 && r < 5300 && g > 3240 && g < 5200 && b > 2680 && b < 4520) {
    return "Rp 100.000";
  } else {
    return "Tidak dikenali";
  }
}
// SESUAIKAN DENGAN NILAI RGB YANG TERDETEKSI PADA SENSOR KAMU //

void bunyikanBuzzer(int durasi) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(durasi);
  digitalWrite(BUZZER_PIN, LOW);
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Mencoba reconnect WiFi...");
    WiFi.reconnect();
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    wifiConnected = WiFi.status() == WL_CONNECTED;
    if (wifiConnected) {
      Serial.println("\n✅ WiFi tersambung!");
      waitForSync();
      myTZ.setLocation("Asia/Jakarta");
      Serial.println("✅ Waktu WIB: " + myTZ.dateTime("d/m/Y H:i:s"));
      Serial.println("Epoch time (WIB): " + String(myTZ.now()));
      ambilDataDariFirebase();
    } else {
      Serial.println("\n❌ Gagal reconnect WiFi.");
    }
  } else {
    wifiConnected = true;
  }
}

void kirimKeTelegram(String pesan) {
  reconnectWiFi();
  if (!wifiConnected) {
    Serial.println("❌ WiFi tidak terhubung.");
    return;
  }

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + bot_token + "/sendMessage";
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String body = "chat_id=" + chat_id + "&text=" + urlencode(pesan);
  http.setConnectTimeout(5000);
  int httpCode = http.POST(body);
  if (httpCode > 0) {
    Serial.println("✅ Telegram terkirim. Kode: " + String(httpCode));
  } else {
    Serial.println("❌ Gagal Telegram. Kode: " + String(httpCode));
  }
  http.end();
}

String urlencode(String str) {
  String encoded = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == '\n') {
      encoded += "%0A";
    } else {
      char hex[4];
      snprintf(hex, sizeof(hex), "%%%02X", c);
      encoded += hex;
    }
  }
  return encoded;
}

void clearTelegramQueue(int updateId) {
  if (!wifiConnected) return;
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + bot_token + "/getUpdates?offset=" + String(updateId + 1) + "&limit=1";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    Serial.println("✅ Antrian Telegram dibersihkan untuk update_id: " + String(updateId));
  } else {
    Serial.println("❌ Gagal clear Telegram. Kode: " + String(httpCode));
  }
  http.end();
}

String getUptime() {
  unsigned long uptime = millis();
  int seconds = (uptime / 1000) % 60;
  int minutes = (uptime / (1000 * 60)) % 60;
  int hours = (uptime / (1000 * 60 * 60)) % 24;
  int days = uptime / (1000 * 60 * 60 * 24);
  return String(days) + " hari, " + String(hours) + " jam, " + String(minutes) + " menit, " + String(seconds) + " detik";
}

void handleReset() {
  totalSaldo = 0;
  totalUangTerdeteksi = 0;
  totalBendaAsing = 0;
  kirimDataKeFirebase("Reset", 0, false);
  kirimKeTelegram("🔄 Saldo dan statistik direset.");
}

void handleSaldo() {
  kirimKeTelegram("💰 Saldo saat ini: Rp " + String(totalSaldo));
}

void handleStatus() {
  char statusMsg[256];
  unsigned long uptime = millis();
  int seconds = (uptime / 1000) % 60;
  int minutes = (uptime / (1000 * 60)) % 60;
  int hours = (uptime / (1000 * 60 * 60)) % 24;
  int days = uptime / (1000 * 60 * 60 * 24);

  snprintf(statusMsg, sizeof(statusMsg),
           "📊 Status Sistem:\n"
           "💰 Saldo: Rp %d\n"
           "📈 Uang Terdeteksi: %d\n"
           "📉 Benda Asing: %d\n"
           "📡 WiFi: %s\n"
           "🔋 Sumber: USB\n"
           "⏰ Uptime: %d hari, %d jam, %d menit, %d detik",
           totalSaldo, totalUangTerdeteksi, totalBendaAsing,
           wifiConnected ? "Terhubung" : "Terputus",
           days, hours, minutes, seconds);

  kirimKeTelegram(String(statusMsg));
  Serial.println("Status dikirim: " + String(statusMsg));
}

void handleSetInterval(String messageText) {
  int spaceIndex = messageText.indexOf(' ');
  if (spaceIndex == -1 || spaceIndex == messageText.length() - 1) {
    kirimKeTelegram("❌ Format: /setinterval <ms>");
    return;
  }
  String intervalStr = messageText.substring(spaceIndex + 1);
  long newInterval = intervalStr.toInt();
  if (newInterval >= 500 && newInterval <= 10000) {
    interval = newInterval;
    kirimKeTelegram("⏱ Interval: " + String(interval) + " ms");
    Serial.println("Interval diubah: " + String(interval) + " ms");
  } else {
    kirimKeTelegram("❌ Interval: 500-10000 ms");
  }
}

void handleReboot(int updateId) {
  kirimKeTelegram("🔄 Reboot dalam 5 detik...");
  Serial.println("🔄 Reboot untuk update_id: " + String(updateId));
  clearTelegramQueue(updateId);
  delay(5000);
  ESP.restart();
}

std::map<std::string, void(*)()> commandHandlers = {
  {"/reset", handleReset},
  {"/saldo", handleSaldo},
  {"/status", handleStatus},
  {"/setinterval", []() { handleSetInterval(""); }},
  {"/reboot", []() { handleReboot(lastUpdateId); }}
};

void cekPerintahTelegram() {
  reconnectWiFi();
  if (!wifiConnected) {
    Serial.println("❌ WiFi tidak terhubung.");
    return;
  }

  if (ESP.getFreeHeap() < minFreeHeap) {
    Serial.println("❌ Heap rendah: " + String(ESP.getFreeHeap()) + " bytes.");
    return;
  }

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + bot_token + "/getUpdates?offset=" + String(lastUpdateId + 1) + "&limit=1";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      JsonArray result = doc["result"];
      for (JsonObject update : result) {
        int update_id = update["update_id"];
        if (update_id <= lastUpdateId) {
          Serial.println("Melewati update_id: " + String(update_id));
          continue;
        }
        lastUpdateId = update_id;
        Serial.println("Update ID baru: " + String(lastUpdateId));
        String messageText = update["message"]["text"].as<String>();
        if (messageText.length() == 0) continue;
        Serial.println("Perintah: " + messageText);
        if (messageText.startsWith("/setinterval")) {
          handleSetInterval(messageText);
        } else {
          auto handler = commandHandlers.find(messageText.c_str());
          if (handler != commandHandlers.end()) {
            handler->second();
          } else {
            kirimKeTelegram("❓ Perintah: /saldo, /status, /reset, /setinterval <ms>, /reboot");
          }
        }
      }
    } else {
      Serial.println("❌ Gagal parsing JSON: " + String(error.c_str()));
    }
    doc.clear();
  } else {
    Serial.println("❌ Gagal ambil Telegram. Kode: " + String(httpCode));
  }
  http.end();
}

void loop() {
  if (wifiConnected) {
    events();
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastCheck >= interval) {
    lastCheck = currentMillis;
    cekPerintahTelegram();

    int statusIR = digitalRead(TCRT_PIN);
    if (statusIR == LOW) {
      Serial.println("🚩 Objek terdeteksi");
      uint16_t r, g, b, c;
      tcs.getRawData(&r, &g, &b, &c);
      String nominal = deteksiUang(r, g, b);
      int nilai = nominalKeInt(nominal);

      Serial.print("R: "); Serial.print(r);
      Serial.print(" G: "); Serial.print(g);
      Serial.print(" B: "); Serial.print(b);
      Serial.println(" => " + nominal);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Deteksi Warna:");
      display.print("R: "); display.println(r);
      display.print("G: "); display.println(g);
      display.print("B: "); display.println(b);
      display.print("Uang: "); display.println(nominal);

      if (nilai > 0) {
        totalUangTerdeteksi++;
        totalSaldo += nilai;
        kirimDataKeFirebase(nominal, nilai, true);
        kirimDataKeTelegram(nominal, true);
        display.print("Saldo: "); display.println(totalSaldo);
        display.display();
        bunyikanBuzzer(200);
      } else {
        totalBendaAsing++;
        kirimDataKeFirebase("Benda Asing", 0, false);
        kirimDataKeTelegram("", false);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("‼ PERINGATAN");
        display.println("Benda asing!");
        display.display();
        bunyikanBuzzer(1000);
      }
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Menunggu objek...");
      display.print("Saldo: Rp "); display.println(totalSaldo);
      display.display();
    }
  }
}