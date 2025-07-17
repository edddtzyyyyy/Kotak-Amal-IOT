# 🌐 Web Monitoring Kotak Amal Pintar

Web Monitoring ini adalah antarmuka berbasis web yang terhubung langsung ke Firebase untuk menampilkan informasi real-time dari Kotak Amal Pintar. Menampilkan saldo, jumlah transaksi, deteksi benda asing, serta grafik mingguan dan bulanan.

---

## 🔥 Teknologi yang Digunakan

- **Firebase Realtime Database**
- **Chart.js** untuk grafik statistik
- **TailwindCSS** untuk tampilan modern dan responsif
- **JavaScript (Modular)** dengan Firebase SDK v10

---

## 📂 Struktur Tampilan

- **Saldo Saat Ini**: Menampilkan jumlah total saldo, jumlah uang, dan benda asing yang terdeteksi
- **Riwayat Transaksi**: Tabel transaksi 10 terakhir (uang & benda asing)
- **Grafik Statistik**:
  - Uang masuk per minggu (7 hari terakhir)
  - Uang masuk per bulan (30 hari terakhir)

---

## 🚀 Cara Menggunakan

1. **Clone Repo** (atau simpan file HTML-nya):
 
2. **Edit Konfigurasi Firebase** di dalam `<script type="module">`:
   ```javascript
   const firebaseConfig = {
     apiKey: "...",
     authDomain: "...",
     databaseURL: "...",
     projectId: "...",
     storageBucket: "...",
     messagingSenderId: "...",
     appId: "..."
   };
   ```
   > Gunakan konfigurasi dari Firebase Console milikmu.

3. **Buka file HTML langsung di browser**  
   Tidak membutuhkan server lokal, cukup klik dua kali file `.html` untuk membukanya di browser modern.

---

## 🔄 Fitur Realtime

- Firebase Realtime Database digunakan untuk menampilkan data saldo dan transaksi secara langsung.
- Grafik diperbarui setiap 60 detik untuk memastikan data selalu terbaru.

---

## 🧪 Contoh Data di Firebase

```json
"saldo": {
  "totalSaldo": 75000,
  "totalUangTerdeteksi": 6,
  "totalBendaAsing": 1
},
"transaksi": {
  "-Nabc123xyz": {
    "timestamp": 1721152125123,
    "nominal": 10000,
    "type": "uang"
  },
  "-Nabc456xyz": {
    "timestamp": 1721152151123,
    "type": "benda"
  }
}
```
