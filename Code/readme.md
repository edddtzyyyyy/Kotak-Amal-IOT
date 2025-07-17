# 💰 Kotak Amal Pintar (Smart Donation Box)

merupakan proyek IoT berbasis ESP32 untuk mendeteksi dan mencatat donasi uang tunai secara otomatis. Sistem ini mendeteksi nominal uang berdasarkan warna, mencatat data ke Firebase, dan mengirimkan notifikasi ke Telegram secara real-time.

---

## ⚙️ Fitur Utama

- 🔍 Deteksi warna uang menggunakan sensor TCS34725
- 🚫 Deteksi benda asing/non-uang dengan sensor IR TCRT5000
- 📟 Tampilan informasi real-time di layar OLED
- ☁️ Sinkronisasi data otomatis ke Firebase Realtime Database
- 📲 Notifikasi Telegram dengan integrasi Bot API
- 🧠 Penyimpanan data di RTC memory untuk pemulihan setelah reboot
- 🔧 Perintah Telegram untuk kontrol & monitoring sistem

---

## 🧰 Komponen Perangkat Keras

| Komponen         | Deskripsi                               |
|------------------|-----------------------------------------|
| ESP32 DevKit     | Mikrokontroler utama                    |
| TCS34725         | Sensor warna untuk mendeteksi uang      |
| TCRT5000         | Sensor IR untuk deteksi benda asing     |
| OLED 0.96" I2C   | Menampilkan jumlah uang & status        |
| Buzzer           | Alarm notifikasi                        |
| Breadboard & kabel jumper | Koneksi antar komponen         |

---

## 🚀 Instalasi dan Penggunaan

### 1. Clone Proyek

clone projek atau unduh zip code

### 2. Instal Library Arduino

Buka **Library Manager** di Arduino IDE dan instal:

- `Adafruit GFX`
- `Adafruit SSD1306`
- `Adafruit TCS34725`
- `ArduinoJson`
- `WiFi`
- `HTTPClient`
- `ezTime`

### 3. Konfigurasi Data

Edit file `.ino` dan ubah bagian berikut:

```cpp
const char* ssid = "Nama_Wifi_Anda";
const char* password = "Password_Wifi_Anda";

String chat_id = "Chat_ID_Telegram_Anda";
String bot_token = "Bot_Token_Telegram_Anda";

String firebaseUrl = "https://ALAMAT-FIREBASE/";
String firebaseApiKey = "API_KEY_FIREBASE";
```


### 4. Upload ke ESP32

- Buka proyek di Arduino IDE
- Pilih board: `ESP32 Dev Module`
- Pilih port COM yang sesuai
- Klik **Upload**

---

## 🤖 Perintah Telegram yang Didukung

| Perintah              | Fungsi                                           |
|-----------------------|--------------------------------------------------|
| `/saldo`              | Tampilkan saldo terakhir                         |
| `/status`             | Info sistem: saldo, jumlah uang, dan benda asing |
| `/reset`              | Reset saldo dan hitungan benda asing             |
| `/setinterval <ms>`   | Atur interval sensor (500–10000 ms)              |
| `/reboot`             | Reboot perangkat ESP32                           |

---



## 📬 Kontak

Pengembang: [Sayid]  
Email: saedabrar321@gmail.com