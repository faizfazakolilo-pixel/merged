/*
  v22.13: UPLOAD WALLPAPER (FOTO) LEWAT BLUETOOTH dari aplikasi MotorNav, tanpa mode WiFi. Paket kontrol baru pada karakteristik
  peta: [0xFF][0x10][ukuran u32] mulai (ESP32 menghapus slot), [0xFF][0x11][offset u32][data...] isi, [0xFF][0x12][crc32 u32] selesai.
  Isi = 240x240 RGB565 little-endian (115200 byte), sama dengan unggahan WiFi. Status dibaca aplikasi dari karakteristik yang sama
  (1 byte: 0 diam, 1 menghapus, 2 siap, 3 menerima, 4 sukses, 5 gagal). Foto langsung tampil tanpa restart. Jalur WiFi tetap ada.

  v22.12: APLIKASI ANDROID MotorNav menggantikan Chronos. Aplikasi memakai layanan BLE peta yang sama (UUID 4d4e0001/0002),
  menggambar peta + banner belokan + kecepatan + ETA di HP lalu mengirim JPEG 240x240 (layar penuh). Ditambah paket kontrol
  [0xFF][cmd]: jam (cmd 1) dan status navigasi/heartbeat (cmd 2). Chronos tetap bisa dipakai seperti sebelumnya.
  Perubahan: isLinked() = Chronos ATAU aplikasi tersambung, jam dari aplikasi, halaman Nav menampilkan frame aplikasi tanpa overlay.

  v22.11: PETA ASLI (hanya ESP32 biasa, bukan C3). HP menjalankan halaman peta_ble.html (Chrome Android, Web Bluetooth)
  yang menggambar peta (tile CARTO/OSM) di sekitar posisi GPS, memutarnya mengikuti arah jalan, lalu mengirimnya sebagai
  JPEG 240x240 lewat BLE ke ESP32 (layanan khusus, bersama koneksi Chronos). Navigasi belok-demi-belok tetap dari
  Chronos/Google Maps: banner, jarak, nama jalan, ETA, kecepatan ditumpuk di atas peta asli. Menu TAMPILAN NAV punya
  mode ke-3 "LIVE". Jika frame peta berhenti >4 dtk, otomatis kembali ke tampilan peta bergaya.

  v22.10: TAMPILAN NAVIGASI BERGAYA PETA (seperti Google Maps mode gelap). Latar biru tua dengan jalan, garis rute biru
  dari bawah sampai persimpangan lalu membelok kiri/kanan/lurus, titik posisi, banner atas (panah belok besar, jarak
  ke belokan, nama jalan, instruksi), chip bawah (jam tiba + sisa waktu) dan lingkaran kecepatan. Ini ilustrasi dari
  data Chronos (bukan peta jalan asli). Persimpangan mendekat sesuai jarak. Cincin progress & warna hijau/oranye/merah
  tetap. Menu pengaturan: item baru TAMPILAN NAV untuk memilih PETA atau 3D (tampilan jalan + motor yang lama).

  v22.9: PERBAIKAN BOOT BERULANG SETELAH UPLOAD VIDEO. Penyebab: library JPEGDEC butuh objek ~18 KB, sedangkan sisa RAM
  hanya ~14 KB (blok terbesar) -> alokasi gagal -> ESP32 restart terus. Kini video didecode oleh dekoder JPEG di ROM
  chip (rom/tjpgd.h, kerja ~5 KB, dialokasi di awal boot), library JPEGDEC tidak dipakai lagi. Tabel frame dibaca
  langsung dari flash (hemat 5 KB RAM), buffer upload dialokasi hanya saat mode upload (hemat 4 KB), dan penyangga
  latar jam (18 KB) dihapus. Jika video gagal didecode, layar memakai foto bawaan (tidak restart).
  Catatan: dekoder ROM lebih lambat dari JPEGDEC; fps tampil bisa lebih rendah dari fps video bila C3 tidak kuat.

  v22.8: halaman upload punya pilihan FPS (otomatis / 10 / 15 / 20 / 30). Fps tinggi = durasi yang muat lebih pendek;
  jika layar tersendat pada fps tinggi, pilih fps lebih rendah.

  Baru (v22.7): SATU SLOT FOTO/VIDEO SAJA, tapi KAPASITASNYA SELURUH PARTISI DATA (~2,3 MB, 5x lebih besar dari
  slot lama 470 KB). Menu pengaturan kini: MULAI UPLOAD, TEMA, STANDBY, KELUAR (tidak ada lagi pilihan slot).
  Halaman upload WiFi hanya punya satu kotak pilih file (foto ATAU video) dan satu tombol "Kirim ke layar".
  - Foto besar (berapa pun resolusinya) otomatis dipotong persegi dan diperkecil ke 240x240 di browser HP.
  - Video panjang: browser HP memilih sendiri fps (15 turun sampai 6) dan kualitas JPEG supaya seluruh video
    muat di 2,3 MB; maks 900 frame. Kira-kira 40 detik @15 fps, atau sampai ~2 menit @6 fps (tergantung isi video).
    Video lebih panjang dari kapasitas dipotong dari awal. Video sangat pendek diberi kualitas lebih tinggi.
  - Format lama (MNVIDEO1, header 4 KB) di awal partisi tetap terbaca; unggahan baru memakai MNVIDEO2 (header 8 KB).
  - Tidak perlu mengubah partitions.csv. Tetap flash ulang penuh bila sebelumnya belum memakai partitions.csv ini.

  Baru di v23:
  - Notifikasi kini menampilkan: nama aplikasi (chip oranye), pengirim/judul (putih besar), dan ISI PESAN
    (hingga 3 baris, dipotong "~" jika panjang). Emoji/karakter non-ASCII dibuang karena font tidak mendukung.
    Lama tampil 8 detik; sentuh untuk menutup lebih cepat.
  - Menu game: SENTUH SINGKAT di layar judul = pindah ke menu berikutnya, TAHAN 0,7 dtk = mulai main.
    Di layar game over / jeda: sentuh = ulang/lanjut, TAHAN 1 dtk = keluar ke menu berikutnya.
  - Perbaikan: animasi standby yang melewati area wajah (balon, helm, roket, hantu, sepeda, dll.) tidak lagi
    meninggalkan sisa gambar yang menumpuk. Gambar dipotong ke area aslinya (persegi wajah / lingkaran dalam cincin).
  - MENU PENGATURAN: di standby TAHAN sensor 4 dtk (bunyi 3x) lalu lepas. Di menu: KLIK = item berikutnya,
    TAHAN 0,7 dtk = pilih/ubah. Isi: MULAI UPLOAD (WiFi, ke satu slot foto/video), TEMA (gelap/terang), STANDBY (wajah/foto), KELUAR.
    Slot kosong = foto bawaan. Tema terang tersimpan di memori.
  - VIDEO LATAR standby mode 1: loop video (tanpa suara, sampai 900 frame, fps otomatis 6-15) menggantikan foto.
    Disimpan sebagai JPEG per frame di partisi data. Unggah lewat halaman WiFi yang sama (bagian "Video latar"):
    HP memotong video jadi frame lalu mengirimnya. Butuh: file partitions.csv
    di folder sketch (otomatis dipakai Arduino IDE; flash ulang file .merged.bin penuh di alamat 0x0).
  - Info lagu: saat lagu diputar di HP (Spotify dll., lewat Chronos), standby menampilkan satu baris kecil
    "Judul - Artis" di bawah jam (teks panjang bergeser pelan). Hilang saat jeda/berhenti/Bluetooth putus.
    Di standby wajah, baris ini menggantikan baris voltase aki selama lagu diputar.
  - Standby foto saat lagu diputar: judul + artis, detik berjalan, garis gelombang mengalir, dan visualizer pelangi
    tembus pandang (simulasi, tidak mengikuti suara asli). Chronos tidak mengirim durasi lagu, jadi detik dihitung
    sejak lagu terdeteksi/berganti (perkiraan) dan garis tidak berupa progress sampai akhir lagu.
  - Warna dinaikkan kecerahannya (abu-abu, hijau, oranye, merah lebih terang). Kecerahan lampu latar (BL)
    ditentukan wiring: sambungkan pin BL langsung ke 3V3 agar maksimal.

  Baru di v22: STANDBY MODE 1 (foto + jam transparan + cincin pelangi berputar)
  - Standby punya 2 mode: 0 = wajah hidup (lama), 1 = foto latar + jam putih transparan + cincin pelangi berputar.
  - Saat standby, TAHAN sentuhan 1,2 detik untuk ganti mode (bunyi 2x). Pilihan tersimpan di memori,
    jadi setelah dimatikan lalu dihidupkan lagi, mode terakhir dipakai. Sentuh singkat = bangun seperti biasa.
  - Foto ada di file standby_photo.h (satu folder dengan .ino). Ganti foto: python3 foto_ke_header.py fotoku.png
  - Tombol BOOT di ESP32 biasa (GPIO0) kini berfungsi sama seperti sensor TP223 (tidak untuk ESP32-C3).
    Standby: tap = bangun | tahan 1,2 dtk (bunyi 1x) lalu lepas = ganti mode | tahan 4 dtk (bunyi 3x) lalu lepas = mode upload foto.
    PENTING: jangan tahan BOOT saat menyalakan ESP32 (itu masuk mode flash). Pakai cara tahan 4 dtk di standby.
  - GANTI FOTO LEWAT HP (tanpa kabel): matikan lalu NYALAKAN motor/ESP32 sambil TAHAN sensor sentuh ~2 detik.
    Layar menampilkan "UPLOAD FOTO". Sambungkan HP ke WiFi "MotorNav-Foto" (sandi 12345678), buka browser
    ke http://192.168.4.1 , pilih foto dari galeri, tekan "Kirim ke layar". Selesai -> ESP32 restart sendiri.
    Foto disimpan di partisi flash "spiffs" (pilih Tools > Partition Scheme: "Default 4MB with spiffs"
    atau "Minimal SPIFFS"; JANGAN "Huge APP"). Tanpa foto upload, dipakai foto bawaan standby_photo.h.
  - Mode 1 butuh TILT_UI true (kanvas penuh). Jika tidak, otomatis memakai wajah.

  Layar Navigasi Motor v15 - tema hitam/putih/hijau/oranye/merah + standby hidup + animasi notifikasi
  ESP32 biasa (DevKit 30/38 pin) ATAU ESP32-C3 Super Mini + GC9A01 (240x240 bulat) + sensor TP223
  Kode otomatis memilih pin sesuai board yang dipilih di Arduino IDE.
  Data navigasi Google Maps dari aplikasi Chronos (fbiego) lewat BLE.

  Warna rute dan cincin menurut jarak ke belokan:
    > 250 m  hijau   |   100-250 m  oranye   |   < 100 m  merah

  Mode standby: jika tidak ada navigasi selama 20 detik, layar menampilkan
  wajah animasi (mata berkedip, melirik, sesekali tersenyum, tertidur setelah
  1 menit, merah saat Bluetooth terputus). Sentuh TP223 untuk bangun.
  Saat navigasi dimulai, layar bangun sendiri.

  Library: ChronosESP32, GFX Library for Arduino
  Board ESP32 biasa : "ESP32 Dev Module"
    Pin: SCK 18, MOSI 23, CS 5, DC 2, RST 4, TP223 OUT 27, BL ke 3V3, VCC ke 3V3, GND ke GND
    (jangan pakai GPIO 6-11, itu terhubung ke flash di ESP32 biasa)
  Board ESP32-C3    : "ESP32C3 Dev Module", USB CDC On Boot: Enabled
    Pin: SCK 4, MOSI 6, CS 7, DC 3, RST 2, TP223 OUT 5, BL ke 3V3

  Standby hidup (30 fps lewat kanvas, tanpa kedip layar): semua gerakan dihaluskan
  (mata, alis, mulut, pipi), wajah bernapas dan memantul, kedip kadang dobel,
  mata melirik ke segala arah. 12 aksi acak: senang, jatuh cinta, kedip sebelah,
  kaget, penasaran, pusing, musik, kacamata hitam, bersin, VROOM, melet, dan
  melirik jam. Menguap lalu tidur (Z melayang, gelembung). Titik berekor
  mengorbit di cincin, bintang berkelip. Saat Bluetooth putus: wajah sedih
  yang mencari-cari HP.

  Notifikasi HP: lonceng berayun + cincin oranye berputar selama 4,5 detik,
  menampilkan nama aplikasi dan judul. Sentuh TP223 untuk menutup lebih cepat.
  Berlaku di semua mode (navigasi, jam, info perjalanan, standby).

  Halaman (sentuh TP223 untuk ganti): lihat daftar "Halaman sekarang" di catatan v18 di atas

  Baru di v19:
  - Halaman sekarang hanya: Navigasi, Game (3 game), dan Standby (otomatis saat tidak ada aktivitas).
    Info perjalanan, Speedometer, Voltase aki, dan Sudut miring sebagai halaman DIHAPUS.
  - Standby menampilkan jam kecil dan voltase aki (hanya jika pin aki tersambung, tegangan terbaca >= 5 V).
  - SELURUH tampilan (navigasi, standby, game, notifikasi, panggilan) berputar mengikuti sudut miring motor
    dari MPU6050. Arah terbalik? ubah TILT_UI_SIGN ke -1. Matikan dengan TILT_UI false.
    Motor tegak tapi tampilan miring? Di halaman Navigasi TAHAN sentuhan 1,5 detik untuk nol-kan (tersimpan).
    Catatan: butuh ~115 KB RAM kosong untuk kanvas penuh. Jika Serial menulis "Kanvas penuh gagal", set TILT_UI false.

  Baru di v18:
  - Halaman Jam DIHAPUS. Jam kini tampil kecil di bawah wajah saat standby (mode saat tidak ada aktivitas).
    Jam disimpan sendiri sehingga tetap berjalan walau Bluetooth terputus (setelah pernah tersambung sekali).
  - Standby kini juga aktif di halaman Game saat tidak bermain (layar judul / game over / jeda) dan
    tidak disentuh 20 detik. Halaman Sudut miring tetap tanpa standby.
  Halaman sekarang: 0 Navigasi, 1 Info perjalanan, 2 Speedometer, 3 Voltase aki, 4 Sudut miring, 5-7 Game.

  Baru di v17:
  - 3 halaman GAME yang dimainkan dengan 1 sensor sentuh TP223:
      6 MOTOR RUN : sentuh = lompat (tahan = lebih tinggi), hindari kerucut.
      7 TERBANG   : sentuh = kepak, lewati celah pipa.
      8 PAS!      : sentuh untuk menghentikan penanda di dalam zona hijau; zona makin sempit.
    Di layar judul / game over / jeda: sentuh = mulai, TAHAN 1 detik = keluar ke halaman berikutnya.
    Skor tertinggi tiap game tersimpan di memori. Buzzer: GAME_SOUND false untuk mematikan.
    Notifikasi/panggilan masuk menjeda game otomatis. Titik halaman dirapatkan agar muat 9 titik.
  Halaman sekarang: 0 Navigasi, 1 Jam, 2 Info perjalanan, 3 Speedometer, 4 Voltase aki, 5 Sudut miring, 6-8 Game.

  Baru di v15:
  - 8 aksi standby baru: kembang api, kaca pembesar, main game, hantu lewat, telinga kucing
    + MEOW, petir, lampu lalu lintas 3-2-1 GO, dan makan donat. Total 43 aksi.
  - FPS lebih tinggi: batas frame 8 ms (hingga ~120 fps jika SPI sanggup), data navigasi tidak
    lagi disalin tiap putaran loop, dan ada penghitung FPS lewat Serial (SHOW_FPS true).
  - Transisi lebih halus: iris menutup dengan easing lebih rapat, lalu halaman baru TIMBUL
    perlahan (fade-in palet warna) saat ganti halaman, bangun dari standby, dan selesai notifikasi/panggilan.
  - Antar aksi standby: fade masuk/keluar lebih panjang dengan kurva halus, jeda antar aksi lebih
    pendek, dan 30% kemungkinan aksi langsung tersambung ke aksi berikutnya lewat kedipan.

  Baru di v14:
  - Halaman Voltase Aki: baca tegangan aki 12V lewat pembagi tegangan, tampil angka besar
    + status (MENGISI / BAIK / CUKUP / LEMAH) dan cincin yang mengikuti level.
  - Buzzer: bunyi pendek saat notifikasi, dering berulang saat panggilan masuk.
  - Halaman Sudut Miring: sensor MPU6050, gauge +/-60 derajat, angka sudut, catatan maksimum
    kiri/kanan. TAHAN sentuhan 1,2 detik di halaman ini untuk nol-kan (motor tegak) dan reset
    maksimum; nilai nol tersimpan di memori.
  Halaman sekarang: 0 Navigasi, 1 Jam, 2 Info perjalanan, 3 Speedometer, 4 Voltase aki, 5 Sudut miring.

  WIRING TAMBAHAN
  ESP32 biasa : Aki (+) -> sekering 1A -> R1 47k -> GPIO 34 -> R2 10k -> GND  (kapasitor 100nF paralel R2,
                dioda zener 3.3V paralel R2 sangat disarankan). Buzzer aktif GPIO 25 (+) / GND (-).
                MPU6050: VCC 3V3, GND, SDA GPIO 21, SCL GPIO 22.
  ESP32-C3    : voltase GPIO 0, buzzer GPIO 10, MPU6050 SDA GPIO 8, SCL GPIO 9.
  Semua GND harus disatukan dengan GND aki/motor.

  Baru di v13:
  - 8 aksi standby baru: mencari sinyal (WiFi), baterai habis, hujan + payung, roket meluncur,
    main pong (mata mengikuti bola), bendera finish, bensin menipis, dan balon terbang.
  - Notifikasi punya 3 gaya animasi: lonceng, bubble chat (WhatsApp/Telegram/SMS dll),
    dan amplop surat (Gmail/Mail/Outlook dll). Gaya dipilih dari nama aplikasi.
  - Panggilan masuk: layar telepon hijau dengan gagang bergetar, gelombang melebar, dan nama
    penelepon. Berhenti otomatis saat diangkat/ditolak, atau sentuh TP223 untuk menutup.
    Memakai watch.setRingerCallback() dari ChronosESP32.

  Baru di v12:
  - 8 aksi wajah baru: ngakak (HA HA), menangis, mata bintang, pakai helm, motor melintas,
    ngopi, melambai "HAI!", dan radar (menyapu dengan garis hijau).
  - 3 latar baru standby: planet mengorbit, hujan meteor, salju. Total 7 gaya latar bergantian.
  - Halaman baru: Speedometer (gauge setengah lingkaran, warna hijau/oranye/merah menurut kecepatan).
    Halaman sekarang: 0 Navigasi, 1 Jam, 2 Info perjalanan, 3 Speedometer.

  Baru di v11:
  - FPS standby dan notifikasi naik (target ~60 fps, SPI 80 MHz, loop tanpa delay besar).
  - Latar belakang standby: cahaya napas di belakang wajah + 6 gaya (kunang-kunang,
    gelombang, riak, warp bintang, gelembung saat tidur, hujan saat Bluetooth putus),
    berganti dengan crossfade halus.
  - 6 aksi baru: malu, marah (uap + tanda kesal), mata berkilau, kiss, berpikir, joget.
  - Transisi: efek aksi fade masuk/keluar, kedip otomatis di pergantian aksi, warna
    cahaya latar berubah halus mengikuti aksi, mata membuka perlahan saat standby mulai.
  - Transisi iris (menutup ke tengah) saat ganti halaman, bangun dari standby, masuk standby.
  - Notifikasi digambar lewat kanvas (tanpa kedip): lonceng muncul membal, gelombang
    melebar, teks fade, lalu memudar di akhir.
*/

#include <Arduino_GFX_Library.h>
#include <ChronosESP32.h>
#include <math.h>
#include <Wire.h>
#include <Preferences.h>
#include "standby_photo.h"
#include <WiFi.h>
#include <WebServer.h>
#include <esp_partition.h>
#include <esp_idf_version.h>
#if __has_include(<rom/tjpgd.h>)
  #include <rom/tjpgd.h>            // dekoder JPEG di ROM ESP32-C3/S3: tidak makan flash program, RAM kerja cuma ~5 KB
  #define HAVE_ROM_JPEG 1
#else
  #define HAVE_ROM_JPEG 0
#endif

#if CONFIG_IDF_TARGET_ESP32C3
  #define PIN_SCK   4
  #define PIN_MOSI  6
  #define PIN_CS    7
  #define PIN_DC    3
  #define PIN_RST   2
  #define PIN_TOUCH 5
  #define PIN_BOOT  -1       // GPIO9 (tombol BOOT C3) dipakai sebagai SCL MPU6050, jadi tombol BOOT tidak dipakai
  #define PIN_BATT  0
  #define PIN_BUZZ  10
  #define PIN_SDA   8
  #define PIN_SCL   9
#else                       // ESP32 biasa (VSPI)
  #define PIN_SCK   18
  #define PIN_MOSI  23
  #define PIN_CS    5
  #define PIN_DC    2
  #define PIN_RST   4
  #define PIN_TOUCH 27
  #define PIN_BOOT  0        // tombol BOOT di board ESP32 = GPIO0 (aktif LOW), berfungsi sama seperti TP223
  #define PIN_BATT  34       // input-only ADC1, aman dipakai bersama BLE
  #define PIN_BUZZ  25
  #define PIN_SDA   21
  #define PIN_SCL   22
#endif

#if !CONFIG_IDF_TARGET_ESP32C3 && HAVE_ROM_JPEG
  #define MAP_BLE 1          // peta asli dari HP lewat BLE (butuh RAM ~30 KB: hanya untuk ESP32 biasa)
#else
  #define MAP_BLE 0
#endif

#define ICON_MSB_FIRST   true   // ubah ke false jika arah dari ikon salah baca
#define RING_START_ANGLE 270    // sesuaikan jika cincin mulai dari sisi yang salah
#define RING_FULL_M      500.0f
#define MID_M            250.0f
#define NEAR_M           100.0f

#define STANDBY_MS       20000UL   // masuk standby setelah tanpa navigasi selama ini
#define SLEEP_MS         60000UL   // wajah tertidur setelah standby selama ini

#define NOTIF_MS         8000UL    // lama animasi notifikasi (cukup untuk membaca isi pesan)
#define FRAME_MS         8         // jeda minimal antar frame animasi (8 = batas ~120 fps)
#define SHOW_FPS         false     // true: cetak FPS standby ke Serial Monitor tiap 2 detik
#define SPI_HZ           80000000UL // turunkan ke 40000000UL jika layar muncul noise

// --- Tampilan miring mengikuti sudut motor (semua mode) ---
#define TILT_UI       true     // true: seluruh tampilan diputar sesuai sudut miring (butuh ~115 KB RAM untuk kanvas penuh)
                               // false: matikan (kembali ke tampilan tegak, hemat memori)
#define TILT_UI_SIGN  1        // 1: motor miring kanan = tampilan berputar searah jarum jam; -1 jika arahnya terbalik
#define TILT_MAX_DEG  40.0f    // batas sudut putar tampilan
// --- Voltase aki di standby ---
#define BATT_ENABLED  true     // false: tidak pernah menampilkan voltase (jika pin aki tidak dipasang)

// --- Voltase aki ---
#define BATT_R1   47000.0f   // resistor dari (+) aki ke pin ADC (ohm)
#define BATT_R2   10000.0f   // resistor dari pin ADC ke GND (ohm)
#define BATT_CAL  1.00f      // kalibrasi: voltase multimeter / voltase tampil, lalu kalikan ke sini
// --- Buzzer (aktif: bunyi saat pin HIGH) ---
#define BUZZER_ON true
// --- MPU6050 / sudut miring ---
#define MPU_ADDR   0x68
#define TILT_AXIS  0         // 0: sensor menghadap depan (miring = putar sumbu X), 1: sensor diputar 90 derajat
#define TILT_SIGN  1         // ganti ke -1 jika kiri/kanan terbalik
#define TILT_ALPHA 0.99f     // makin besar makin mengandalkan gyro (lebih halus, sedikit drift)

#define HOR 84    // garis cakrawala
#define BOT 215   // tepi bawah jalan

// Kanvas penuh 240x240 dibagi dua blok 240x120 (2 x 57,6 KB). ESP32 biasa tidak punya satu blok RAM
// berurutan sebesar 115 KB (maks ~113 KB), sehingga kanvas tunggal gagal dialokasi: wajah, notifikasi,
// panggilan, dan game tidak tampil. Dua blok kecil selalu muat.
class SplitCanvas : public Arduino_GFX {
 public:
  static const int SH = 30;          // tinggi tiap blok (baris)
  static const int SN = 8;           // jumlah blok: 8 x 30 baris = 240 baris
  uint16_t *s[SN] = {nullptr};
  // Batas gambar per baris [lo..hi]. Mode 0 = bebas, 1 = persegi area animasi (x 36..203, y 56..191 = CVX/CVY/CVW/CVH),
  // 2 = lingkaran di dalam cincin (r 112). Dengan batas ini animasi yang keluar area tidak meninggalkan sisa gambar.
  uint8_t clipMode = 0;
  const int16_t *cLo = nullptr, *cHi = nullptr;
  int16_t clipLoT[3][240], clipHiT[3][240];
  SplitCanvas() : Arduino_GFX(240, 240) {
    for (int y = 0; y < 240; y++) {
      clipLoT[0][y] = 0; clipHiT[0][y] = 239;
      bool in = (y >= 56 && y < 56 + 136);
      clipLoT[1][y] = in ? 36 : 1; clipHiT[1][y] = in ? (36 + 168 - 1) : 0;
      float dy = y - 119.5f, r2 = 112.0f * 112.0f - dy * dy;
      if (r2 < 0) { clipLoT[2][y] = 1; clipHiT[2][y] = 0; }
      else { float q = sqrtf(r2); clipLoT[2][y] = (int16_t)ceilf(119.5f - q); clipHiT[2][y] = (int16_t)floorf(119.5f + q); }
    }
    setClip(0);
  }
  void setClip(uint8_t m) { clipMode = m; cLo = clipLoT[m]; cHi = clipHiT[m]; }
  bool begin(int32_t speed = GFX_NOT_DEFINED) override {
    (void)speed;
    for (int i = 0; i < SN; i++) {
      if (s[i]) continue;
      s[i] = (uint16_t *)malloc(240 * SH * 2);
      if (!s[i]) {
        for (int j = 0; j < SN; j++) { free(s[j]); s[j] = nullptr; }
        return false;
      }
    }
    return true;
  }
  inline uint16_t *row(int y) { return s[y / SH] + (y % SH) * 240; }
  void writePixelPreclipped(int16_t x, int16_t y, uint16_t c) override { if (x >= cLo[y] && x <= cHi[y]) row(y)[x] = c; }
  void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) override {
    for (int j = 0; j < h; j++) {
      int yy = y + j;
      int x0 = (x > cLo[yy]) ? x : cLo[yy];
      int x1 = (x + w - 1 < cHi[yy]) ? (x + w - 1) : cHi[yy];
      if (x1 < x0) continue;
      uint16_t *p = row(yy);
      for (int i = x0; i <= x1; i++) p[i] = c;
    }
  }
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t c) override {
    if (x < 0 || x >= 240 || h <= 0) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > 240) h = 240 - y;
    for (int j = 0; j < h; j++) { int yy = y + j; if (x >= cLo[yy] && x <= cHi[yy]) row(yy)[x] = c; }
  }
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t c) override {
    if (y < 0 || y >= 240 || w <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > 240) w = 240 - x;
    int x0 = (x > cLo[y]) ? x : cLo[y];
    int x1 = (x + w - 1 < cHi[y]) ? (x + w - 1) : cHi[y];
    if (x1 < x0) return;
    uint16_t *p = row(y);
    for (int i = x0; i <= x1; i++) p[i] = c;
  }
};
#if TILT_UI
typedef SplitCanvas CanvasT;
#else
typedef Arduino_Canvas CanvasT;
#endif

Arduino_DataBus *bus = new Arduino_ESP32SPI(PIN_DC, PIN_CS, PIN_SCK, PIN_MOSI, GFX_NOT_DEFINED);
Arduino_GFX *panel = new Arduino_GC9A01(bus, PIN_RST, 0, true);   // layar fisik
Arduino_GFX *gfx = panel;          // target gambar: kanvas penuh (TILT_UI) atau layar langsung
CanvasT *fb = nullptr;      // kanvas penuh 240x240 (hanya jika TILT_UI)
void present();                    // dorong kanvas ke layar (dengan putaran jika TILT_UI)
void syncFB();                     // TILT_UI: tampilkan kanvas sekarang; selain itu kosong
int clockMinutes(bool connected);   // dideklarasikan di sini karena dipakai renderFace sebelum definisinya
#if TILT_UI
inline void useClip(uint8_t m) { if (fb) fb->setClip(m); }   // 0 bebas | 1 persegi area wajah/game | 2 lingkaran dalam cincin
#else
inline void useClip(uint8_t) {}
#endif
void asciiClean(const String &in, char *out, size_t cap);   // didefinisikan di bagian notifikasi

ChronosESP32 watch("Motor Nav");

// v22.12: HP memakai aplikasi MotorNav (bukan Chronos). Paket kontrol dari aplikasi: [0xFF][cmd][...]
//   cmd 1 = jam  [jam][menit][detik]   cmd 2 = status [navAktif 0/1] (dikirim tiap ~2 dtk = tanda aplikasi tersambung)
volatile bool     appNav = false;
volatile uint32_t appStamp = 0;                                    // millis() paket status terakhir (0 = belum pernah)
volatile bool     appClockNew = false;
volatile uint8_t  appH = 0, appM = 0, appS = 0;
volatile uint32_t appClockMs = 0;
static inline bool appLinked() { return appStamp != 0 && millis() - appStamp < 8000; }
static inline bool appNavOn()  { return appLinked() && appNav; }
static inline bool isLinked()  { return watch.isConnected() || appLinked(); }

enum Page { PAGE_NAV, PAGE_GAME, PAGE_GAME2, PAGE_GAME3, PAGE_COUNT };
enum Mood { MOOD_HAPPY, MOOD_SLEEP, MOOD_OFF };

uint8_t  page = PAGE_NAV;
bool     lastActive = false;
bool     lastTouch = false;
uint32_t lastTap = 0;
String   lastSig = "";

// Standby
const uint16_t *photoPtr = STANDBY_PHOTO;   // foto aktif: bawaan (flash program) atau hasil upload (partisi spiffs)
bool     standby = false;
bool     settingsActive = false, setDirty = false, lightTheme = false;   // menu pengaturan, tema terang
uint8_t  setIdx = 0;                                                     // item menu pengaturan
uint32_t setLastMs = 0;
uint8_t  standbyMode = 0;      // 0 = wajah hidup, 1 = foto + jam + cincin pelangi (tersimpan di memori)
uint32_t lastActivity = 0, standbyStart = 0;
uint32_t nextBlink = 0, blinkStart = 0, nextGaze = 0, nextHappy = 0, happyUntil = 0, lastFrame = 0;
bool     blinking = false;
float    gaze = 0, gazeTarget = 0;
int64_t  lastFaceKey = -1;
bool     faceConnected = true;

// Notifikasi
volatile bool notifPending = false;
char     notifApp[32] = "", notifTitle[40] = "", notifMsg[120] = "";
bool     notifActive = false;
uint32_t notifStart = 0, lastNotifFrame = 0;
long     lastNotifRing = -1;
uint8_t  notifStyle = 0, notifCount = 0;                // 0 lonceng, 1 chat, 2 amplop
// Panggilan masuk
volatile bool callStartPending = false, callEndPending = false;
char     callName[24] = "";
bool     callActive = false;
uint32_t callStart = 0, lastCallFrame = 0;
long     lastCallRing = -1;

uint16_t COL_BG, COL_MAPBG, COL_ROAD, COL_EDGE, COL_HORIZON, COL_RING_BG;
uint16_t COL_BLD, COL_WIN1, COL_WIN2, COL_CHIP;
uint16_t COL_TEXT, COL_DIM;
uint16_t COL_GREEN, COL_ORANGE, COL_RED, COL_CHEEK;
uint16_t COL_GLOW_GREEN, COL_GLOW_ORANGE, COL_GLOW_RED;
uint16_t COL_TIRE, COL_JACKET, COL_HELMET, COL_VISOR, COL_TAIL;

// ---------- Helper ----------
String clip(const String &s, uint8_t maxLen) {
  if (s.length() <= maxLen) return s;
  return s.substring(0, maxLen - 1) + "~";
}

void drawCentered(const String &s, int y, uint8_t size, uint16_t color) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->setTextSize(size);
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  gfx->setTextColor(color);
  gfx->setCursor(120 - (int)w / 2 - x1, y);
  gfx->print(s);
}

void drawChipText(const String &s, int y, uint8_t size, uint16_t color, uint16_t bg) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->setTextSize(size);
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  gfx->fillRoundRect(120 - (int)w / 2 - 8, y - 3, w + 16, h + 6, 9, bg);
  gfx->setTextColor(color);
  gfx->setCursor(120 - (int)w / 2 - x1, y);
  gfx->print(s);
}

float parseMeters(String s) {
  s.replace(',', '.');
  float v = s.toFloat();
  String l = s;
  l.toLowerCase();
  if (l.indexOf("km") >= 0) v *= 1000.0f;
  return v;
}

// -1 = kiri, 0 = lurus, 1 = kanan
int turnDirection(const Navigation &nav) {
  String d = nav.directions;
  if (d.length() == 0) d = nav.title;
  d.toLowerCase();
  if (d.indexOf("left") >= 0 || d.indexOf("kiri") >= 0)   return -1;
  if (d.indexOf("right") >= 0 || d.indexOf("kanan") >= 0) return 1;

  // Teks tidak menyebut belokan (mis. "Ambil arah barat laut"): ikon hanya dipakai jika jarak sudah diketahui
  if (nav.hasIcon && nav.distance.length() > 0) {
    int minX = 48, maxX = -1;
    for (int y = 0; y < 48; y++) {
      for (int x = 0; x < 48; x++) {
        int idx = y * 48 + x;
        uint8_t b = nav.icon[idx / 8];
        bool on = ICON_MSB_FIRST ? (b & (0x80 >> (idx % 8))) : (b & (1 << (idx % 8)));
        if (on) {
          if (x < minX) minX = x;
          if (x > maxX) maxX = x;
        }
      }
    }
    if (maxX >= 0) {
      int leftExt = 24 - minX;
      int rightExt = maxX - 24;
      if (leftExt - rightExt > 12) return -1;
      if (rightExt - leftExt > 12) return 1;
    }
  }
  return 0;
}

void drawPageDots() {
  for (int i = 0; i < PAGE_COUNT; i++) {
    int x = 120 + (int)((i - (PAGE_COUNT - 1) / 2.0f) * (PAGE_COUNT > 7 ? 10 : 14));
    if (i == page) gfx->fillCircle(x, 229, 3, COL_TEXT);
    else           gfx->drawCircle(x, 229, 3, COL_DIM);
  }
}

void drawRing(float progress, uint16_t color) {
  gfx->fillArc(120, 120, 119, 113, 0, 360, COL_RING_BG);
  if (progress <= 0.01f) return;
  float start = RING_START_ANGLE;
  float end = start + 360.0f * progress;
  if (end <= 360.0f) {
    gfx->fillArc(120, 120, 119, 113, start, end, color);
  } else {
    gfx->fillArc(120, 120, 119, 113, start, 360, color);
    gfx->fillArc(120, 120, 119, 113, 0, end - 360.0f, color);
  }
}

// Transisi: iris menutup ke tengah dengan halus, lalu tampilan baru muncul
void irisClose() {                                                  // menutup dengan easing (smoothstep)
  int prev = 119;
  for (int i = 1; i <= 14; i++) {
    float t = i / 14.0f;
    int r = (int)(119.0f * (1.0f - t * t * (3.0f - 2.0f * t)));
    gfx->fillArc(120, 120, prev + 1, r, 0, 360, COL_BG);
    prev = r;
    syncFB();
  }
  gfx->fillCircle(120, 120, prev + 1, COL_BG);
  syncFB();
}

// ---------- Adegan jalan perspektif ----------
int hw(int y) { return 12 + (y - HOR) * 63 / (BOT - HOR); }  // setengah lebar jalan
int rw(int y) { return 3 + (y - HOR) * 16 / (BOT - HOR); }   // setengah lebar rute

void quad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c) {
  gfx->fillTriangle(x0, y0, x1, y1, x2, y2, c);
  gfx->fillTriangle(x0, y0, x2, y2, x3, y3, c);
}

void drawBlock(int x, int y, int w, int h) {
  gfx->fillRoundRect(x, y, w, h, 4, COL_BLD);
  for (int wy = y + 5; wy < y + h - 5; wy += 9) {
    for (int wx = x + 5; wx < x + w - 5; wx += 8) {
      bool alt = (((wx - x) / 8 + (wy - y) / 9) % 2) != 0;
      gfx->fillRect(wx, wy, 3, 4, alt ? COL_WIN2 : COL_WIN1);
    }
  }
}

void drawRoute(int dir, int jy, int extra, uint16_t color) {
  int w = rw(jy) + extra;
  if (dir == 0) {
    quad(120 - 3 - extra, HOR + 4, 120 + 3 + extra, HOR + 4, 120 + w, jy, 120 - w, jy, color);
  }
  quad(120 - w, jy, 120 + w, jy, 120 + rw(BOT) + extra, BOT, 120 - rw(BOT) - extra, BOT, color);
  if (dir < 0)      gfx->fillRect(40, jy - w, 120 + w - 40, 2 * w, color);
  else if (dir > 0) gfx->fillRect(120 - w, jy - w, 200 - (120 - w), 2 * w, color);
}

void drawRider(int cx, int cy, uint16_t accent, uint16_t glow) {
  gfx->fillCircle(cx, cy + 12, 26, glow);                          // cahaya di bawah motor
  gfx->fillRoundRect(cx - 6, cy - 4, 12, 34, 5, COL_TIRE);         // ban belakang
  gfx->fillRoundRect(cx - 9, cy - 18, 18, 20, 7, accent);          // bodi
  gfx->fillRoundRect(cx - 5, cy - 14, 10, 4, 2, COL_TAIL);         // lampu belakang
  gfx->fillRoundRect(cx - 14, cy - 34, 28, 18, 8, COL_JACKET);     // jaket
  gfx->fillRect(cx - 22, cy - 26, 44, 3, COL_DIM);                 // setang
  gfx->fillCircle(cx, cy - 42, 8, COL_HELMET);                     // helm
  gfx->fillRect(cx - 6, cy - 45, 12, 3, COL_VISOR);                // visor
}

void drawScene(int dir, float meters, uint16_t accent, uint16_t glow) {
  float frac = constrain(meters, 0.0f, RING_FULL_M) / RING_FULL_M;
  int jy = 160 - (int)(frac * 48.0f);          // 160 = dekat, 112 = jauh
  int hs = 5 + (jy - HOR) * 18 / 100;          // setengah lebar jalan samping

  gfx->fillRect(0, HOR, 240, 240 - HOR, COL_MAPBG);
  gfx->drawFastHLine(0, HOR, 240, COL_HORIZON);

  drawBlock(14, 112, 34, 30);
  drawBlock(14, 156, 30, 34);
  drawBlock(192, 112, 34, 30);
  drawBlock(196, 156, 30, 34);

  quad(120 - hw(HOR), HOR, 120 + hw(HOR), HOR, 120 + hw(BOT), BOT, 120 - hw(BOT), BOT, COL_ROAD);
  gfx->drawLine(120 - hw(HOR), HOR, 120 - hw(BOT), BOT, COL_EDGE);
  gfx->drawLine(120 - hw(HOR) + 1, HOR, 120 - hw(BOT) + 1, BOT, COL_EDGE);
  gfx->drawLine(120 + hw(HOR), HOR, 120 + hw(BOT), BOT, COL_EDGE);
  gfx->drawLine(120 + hw(HOR) - 1, HOR, 120 + hw(BOT) - 1, BOT, COL_EDGE);
  gfx->fillRect(0, jy - hs, 120 - hw(jy), 2 * hs, COL_ROAD);
  gfx->fillRect(120 + hw(jy), jy - hs, 240 - (120 + hw(jy)), 2 * hs, COL_ROAD);
  gfx->fillRect(120 - hw(jy) - 1, jy - hs + 1, 4, 2 * hs - 2, COL_ROAD);
  gfx->fillRect(120 + hw(jy) - 2, jy - hs + 1, 4, 2 * hs - 2, COL_ROAD);

  drawRoute(dir, jy, 4, glow);
  drawRoute(dir, jy, 0, accent);
  int hh = rw(jy) + 9;
  if (dir < 0)      gfx->fillTriangle(16, jy, 42, jy - hh, 42, jy + hh, accent);
  else if (dir > 0) gfx->fillTriangle(224, jy, 198, jy - hh, 198, jy + hh, accent);
  else              gfx->fillTriangle(120, HOR, 106, HOR + 24, 134, HOR + 24, accent);

  drawRider(120, 186, accent, glow);
}

// ---------- Tampilan navigasi bergaya peta ----------
uint8_t navStyle = 0;                                              // 0 = peta (gaya Google Maps), 1 = jalan 3D + motor (lama)

bool mapDecode(const uint8_t *p, uint32_t len);                   // didefinisikan di bagian video (dekoder JPEG ROM)

#if MAP_BLE
void bleWpBegin(uint32_t size);                                    // v22.13: upload wallpaper lewat BLE (didefinisikan dekat wallpaperLoad)
void bleWpData(uint32_t off, const uint8_t *d, size_t n);
void bleWpEnd(uint32_t crc);
void bleWpTick();
bool bleWpBusy();
static NimBLECharacteristic *mapChr = nullptr;
// Frame peta dari HP: paket = [id u8][offset u16 LE][total u16 LE][data...]; frame lengkap -> JPEG 240x240.
#define MAP_SVC_UUID "4d4e0001-7a1e-4c1b-9d52-6f2d4e1a0001"
#define MAP_CHR_UUID "4d4e0002-7a1e-4c1b-9d52-6f2d4e1a0001"
#define MAP_BUF_SZ   12000
static uint8_t *mapBuf[2] = {nullptr, nullptr};
static volatile uint8_t  mapAct = 0;                               // buffer berisi frame lengkap terakhir
static volatile uint16_t mapLen[2] = {0, 0};
static volatile uint32_t mapStamp = 0;                             // millis() frame lengkap terakhir (0 = belum ada)
static uint8_t  mapFid = 255, mapFill = 1;
static uint16_t mapGot = 0, mapTot = 0;
static volatile int mapConn = -1;                                  // handle koneksi HP-peta (dikenali dari tulisan pertama)
static int mapTotalConn = 0;

class MapCharCb : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &ci) override {
    mapConn = ci.getConnHandle();
    NimBLEAttValue v = c->getValue();
    const uint8_t *d = v.data();
    size_t n = v.size();
    if (n >= 3 && d[0] == 0xFF) {                                  // paket kontrol dari aplikasi MotorNav
      if (d[1] == 1 && n >= 5) { appH = d[2]; appM = d[3]; appS = d[4]; appClockMs = millis(); appClockNew = true; }
      else if (d[1] == 2) { appNav = d[2] != 0; appStamp = millis(); }
      else if (d[1] == 0x10 && n >= 6) bleWpBegin(d[2] | (d[3] << 8) | (d[4] << 16) | ((uint32_t)d[5] << 24));
      else if (d[1] == 0x11 && n >= 7) bleWpData(d[2] | (d[3] << 8) | (d[4] << 16) | ((uint32_t)d[5] << 24), d + 6, n - 6);
      else if (d[1] == 0x12 && n >= 6) bleWpEnd(d[2] | (d[3] << 8) | (d[4] << 16) | ((uint32_t)d[5] << 24));
      return;
    }
    if (bleWpBusy()) return;                                       // sedang upload wallpaper: frame peta diabaikan
    if (n < 6 || !mapBuf[0] || !mapBuf[1]) return;
    uint8_t fid = d[0];
    uint16_t off = d[1] | (d[2] << 8), tot = d[3] | (d[4] << 8);
    if (tot == 0 || tot > MAP_BUF_SZ) return;
    if (fid != mapFid || tot != mapTot) { mapFid = fid; mapTot = tot; mapGot = 0; mapFill = mapAct ^ 1; }
    size_t pl = n - 5;
    if ((size_t)off + pl > tot) return;
    memcpy(mapBuf[mapFill] + off, d + 5, pl);
    mapGot += pl;
    if (mapGot >= mapTot) {                                        // frame lengkap
      mapLen[mapFill] = mapTot; mapAct = mapFill; mapStamp = millis();
      mapFid = 255; mapGot = 0; mapTot = 0;
    }
  }
};
static MapCharCb mapCharCb;

// Chronos menganggap tiap koneksi = HP Chronos. Koneksi HP-peta tidak boleh mengubah status "Chronos tersambung".
class MapSrvCb : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *s, NimBLEConnInfo &ci) override {
    mapTotalConn++;
    static_cast<NimBLEServerCallbacks &>(watch).onConnect(s, ci);
  }
  void onDisconnect(NimBLEServer *s, NimBLEConnInfo &ci, int reason) override {
    if (mapTotalConn > 0) mapTotalConn--;
    if ((int)ci.getConnHandle() == mapConn) { mapConn = -1; mapStamp = 0; }
    int chronosConns = mapTotalConn - (mapConn >= 0 ? 1 : 0);
    if (chronosConns <= 0) static_cast<NimBLEServerCallbacks &>(watch).onDisconnect(s, ci, reason);
    else NimBLEDevice::startAdvertising();
  }
};
static MapSrvCb mapSrvCb;

static bool mapFresh() { return mapStamp != 0 && millis() - mapStamp < 4000; }
#endif

static void mapArrow(int x, int y, int dir, uint16_t c) {          // panah belok putih dalam kotak 36x36
  if (dir < 0) {
    gfx->fillRect(x + 17, y + 20, 6, 16, c); gfx->fillRect(x + 8, y + 17, 15, 6, c);
    gfx->fillTriangle(x + 1, y + 20, x + 13, y + 7, x + 13, y + 33, c);
  } else if (dir > 0) {
    gfx->fillRect(x + 13, y + 20, 6, 16, c); gfx->fillRect(x + 13, y + 17, 15, 6, c);
    gfx->fillTriangle(x + 35, y + 20, x + 23, y + 7, x + 23, y + 33, c);
  } else {
    gfx->fillRect(x + 15, y + 14, 6, 22, c);
    gfx->fillTriangle(x + 18, y + 1, x + 5, y + 16, x + 31, y + 16, c);
  }
}

static void mapText(const char *t, int x, int y, uint8_t sz, uint16_t c) {
  gfx->setTextSize(sz); gfx->setTextColor(c); gfx->setCursor(x, y); gfx->print(t);
}

static void drawNavMap(const Navigation &nav, int dir, bool noDist, float meters, float progress, uint16_t accent, bool real) {
  const uint16_t MBG = gfx->color565(12, 28, 64), MROAD = gfx->color565(30, 62, 120), MROAD2 = gfx->color565(21, 44, 90);
  const uint16_t RT_O = gfx->color565(20, 90, 200), RT = gfx->color565(100, 185, 255);
  const uint16_t BAN = gfx->color565(22, 104, 158), CH = gfx->color565(10, 20, 46), CWHT = 0xFFFF;
  float frac = constrain(noDist ? RING_FULL_M : meters, 0.0f, RING_FULL_M) / RING_FULL_M;
  int jy = 150 - (int)(frac * 50.0f);                              // persimpangan: dekat (150) ... jauh (100)

  bool drew = false;
#if MAP_BLE
  if (real) drew = mapDecode(mapBuf[mapAct], mapLen[mapAct]);        // peta asli dari HP (posisi/arah sudah digambar HP)
#endif
  if (!drew) {
  gfx->fillScreen(MBG);
  for (int k = -2; k <= 2; k++) if (k) gfx->fillRect(0, jy + k * 64 - 4, 240, 8, MROAD2);   // jalan lain (bergeser saat mendekat)
  gfx->fillRect(28, 0, 8, 240, MROAD2);
  gfx->fillRect(204, 0, 8, 240, MROAD2);
  gfx->fillRect(0, jy - 11, 240, 22, MROAD);                       // jalan yang disilang
  gfx->fillRect(107, 0, 26, 240, MROAD);                           // jalan utama

  for (int pass = 0; pass < 2; pass++) {                           // rute: garis tepi dulu, lalu isi
    int w = pass ? 10 : 16; uint16_t c = pass ? RT : RT_O;
    gfx->fillRect(120 - w / 2, jy - w / 2, w, 240 - jy + w / 2, c);
    if (dir < 0)      gfx->fillRect(44, jy - w / 2, 120 - 44 + w / 2, w, c);
    else if (dir > 0) gfx->fillRect(120 - w / 2, jy - w / 2, 196 - 120 + w / 2, w, c);
    else              gfx->fillRect(120 - w / 2, 100, w, jy - 100, c);
    gfx->fillCircle(120, jy, w / 2, c);
    int e = pass ? 0 : 3;                                          // kepala panah di ujung rute
    if (dir < 0)      gfx->fillTriangle(30 - e, jy, 50 + e, jy - 11 - e, 50 + e, jy + 11 + e, c);
    else if (dir > 0) gfx->fillTriangle(210 + e, jy, 190 - e, jy - 11 - e, 190 - e, jy + 11 + e, c);
    else              gfx->fillTriangle(120, 92 - e, 108 - e, 110 + e, 132 + e, 110 + e, c);
  }

  gfx->fillTriangle(120, 150, 105, 173, 135, 173, gfx->color565(50, 105, 215));   // posisi saya: kerucut arah + titik
  gfx->fillCircle(120, 178, 11, CWHT);
  gfx->fillCircle(120, 178, 8, gfx->color565(66, 133, 244));
  }

  char t1[40], t2[40];
  asciiClean(nav.title, t1, sizeof(t1));
  asciiClean(nav.directions, t2, sizeof(t2));
  String street = t1[0] ? String(t1) : String(t2);
  String instr  = (t1[0] && strcmp(t1, t2) != 0) ? String(t2) : String("");
  String distTxt = nav.distance;
  if (noDist) distTxt = (dir < 0) ? "Kiri" : (dir > 0 ? "Kanan" : "Lurus");

  gfx->fillRoundRect(44, 40, 152, 56, 12, BAN);                    // banner atas
  mapArrow(50, 50, dir, CWHT);
  char b[24];
  strlcpy(b, clip(distTxt, 8).c_str(), sizeof(b));  mapText(b, 94, 46, 2, accent);
  strlcpy(b, clip(street, 16).c_str(), sizeof(b));  mapText(b, 94, 66, 1, CWHT);
  if (instr.length()) { strlcpy(b, clip(instr, 16).c_str(), sizeof(b)); mapText(b, 94, 78, 1, gfx->color565(190, 225, 245)); }

  String etaT = nav.eta; int cm = etaT.indexOf(','); if (cm > 0) etaT = etaT.substring(0, cm);
  String bt = etaT.length() ? String("ETA ") + etaT : String("");
  if (nav.duration.length()) bt += (bt.length() ? "  " : "") + nav.duration;
  if (bt.length()) drawChipText(clip(bt, 24), 206, 1, COL_TEXT, CH);

  String num = "";                                                 // lingkaran kecepatan
  for (unsigned i = 0; i < nav.speed.length(); i++) {
    char ch = nav.speed[i];
    if (ch >= '0' && ch <= '9') num += ch; else if (num.length()) break;
  }
  if (num.length()) {
    gfx->fillCircle(190, 162, 21, CH); gfx->drawCircle(190, 162, 21, CWHT);
    int nw = num.length() * 12;
    mapText(num.c_str(), 190 - nw / 2, 152, 2, CWHT);
    mapText("km/j", 190 - 12, 171, 1, gfx->color565(150, 190, 230));
  }
  drawRing(progress, accent);
  drawPageDots();
}

void drawNavPage(const Navigation &nav, bool connected) {
  gfx->fillScreen(COL_BG);
#if MAP_BLE
  if (appNavOn()) {                                                // v22.12: aplikasi MotorNav menggambar seluruh layar (peta + banner + kecepatan)
    if (mapFresh() && mapDecode(mapBuf[mapAct], mapLen[mapAct])) return;
    drawRing(1.0f, COL_ORANGE);
    drawCentered("Menunggu", 85, 3, COL_TEXT);
    drawCentered("peta...", 118, 3, COL_DIM);
    drawPageDots();
    return;
  }
#endif

  if (!connected) {
    drawRing(1.0f, COL_RED);
    drawCentered("Hubungkan", 85, 3, COL_RED);
    drawCentered("aplikasi", 118, 3, COL_TEXT);
    drawPageDots();
    return;
  }
  if (!nav.active) {
    drawRing(1.0f, COL_GREEN);
    if (appLinked()) {
      drawCentered("Pilih", 85, 3, COL_TEXT);
      drawCentered("tujuan di aplikasi", 118, 2, COL_DIM);
    } else {
      drawCentered("Buka", 85, 3, COL_TEXT);
      drawCentered("Google Maps", 118, 2, COL_DIM);
    }
    drawPageDots();
    return;
  }

  float meters = parseMeters(nav.distance);
  bool noDist = (nav.distance.length() == 0 || meters <= 0.0f);     // mis. awal rute "Ambil arah ...": belum ada jarak belokan
  float progress = noDist ? 0.0f : 1.0f - constrain(meters / RING_FULL_M, 0.0f, 1.0f);
  uint16_t accent = COL_GREEN, glow = COL_GLOW_GREEN;
  if (!noDist && meters < NEAR_M)      { accent = COL_RED;    glow = COL_GLOW_RED; }
  else if (!noDist && meters < MID_M)  { accent = COL_ORANGE; glow = COL_GLOW_ORANGE; }
  int dir = turnDirection(nav);

  if (navStyle != 1) {
    bool real = false;
#if MAP_BLE
    real = (navStyle == 2) && mapFresh();
#endif
    drawNavMap(nav, dir, noDist, meters, progress, accent, real);
    return;
  }
  drawScene(dir, noDist ? RING_FULL_M : meters, accent, glow);   // tanpa jarak: persimpangan digambar jauh di depan
  drawRing(progress, accent);

  String spd = nav.speed.length() ? nav.speed : String("--");
  drawCentered(clip(spd, 4), 8, 3, COL_TEXT);
  String distTxt = nav.distance;
  if (noDist) distTxt = (dir < 0) ? "Kiri" : (dir > 0 ? "Kanan" : "Lurus");
  drawCentered(clip(distTxt, 7), 31, 3, accent);

  String ttl = nav.title.length() ? nav.title : nav.directions;
  if (ttl.length() <= 11) {
    drawChipText(ttl, 57, 2, COL_TEXT, COL_CHIP);
  } else {                                                           // teks panjang: ukuran kecil, maksimal 2 baris
    String l1 = ttl, l2 = "";
    if (ttl.length() > 22) {
      int cut = ttl.lastIndexOf(' ', 22);
      if (cut < 6) cut = 22;
      l1 = ttl.substring(0, cut);
      l2 = ttl.substring(cut);
      l2.trim();
      l2 = clip(l2, 22);
    }
    int16_t x1, y1; uint16_t w1, h1, w2 = 0;
    gfx->setTextSize(1);
    gfx->getTextBounds(l1, 0, 0, &x1, &y1, &w1, &h1);
    if (l2.length()) { gfx->getTextBounds(l2, 0, 0, &x1, &y1, &w2, &h1); }
    uint16_t wm = max(w1, w2);
    int ch = l2.length() ? 26 : 16;
    gfx->fillRoundRect(120 - (int)wm / 2 - 8, 55, wm + 16, ch, 8, COL_CHIP);
    gfx->setTextColor(COL_TEXT);
    gfx->getTextBounds(l1, 0, 0, &x1, &y1, &w1, &h1);
    gfx->setCursor(120 - (int)w1 / 2 - x1, 59);
    gfx->print(l1);
    if (l2.length()) {
      gfx->getTextBounds(l2, 0, 0, &x1, &y1, &w2, &h1);
      gfx->setCursor(120 - (int)w2 / 2 - x1, 69);
      gfx->print(l2);
    }
  }
  drawPageDots();
}

// ---------- Voltase aki ----------
float    battV = 0;
uint32_t lastBattRead = 0;
bool     battInit = false;

void updateBatt() {
  if (battInit && millis() - lastBattRead < 200) return;
  lastBattRead = millis();
  uint32_t sum = 0;
  for (int i = 0; i < 16; i++) sum += analogReadMilliVolts(PIN_BATT);
  float v = (sum / 16.0f / 1000.0f) * ((BATT_R1 + BATT_R2) / BATT_R2) * BATT_CAL;
  battV = battInit ? battV * 0.8f + v * 0.2f : v;                 // haluskan angka
  battInit = true;
}

// ---------- Mode standby: wajah hidup (kanvas, 30 fps) ----------
#define CVX 36
#define CVY 56
#define CVW 168
#define CVH 136
#if TILT_UI
  #define OX 0                // kanvas penuh: koordinat layar = koordinat kanvas
  #define OY 0
#else
  #define OX CVX              // kanvas kecil di posisi (CVX, CVY)
  #define OY CVY
#endif

#if defined(GFX_SKIP_OUTPUT_BEGIN)
  #define CANVAS_BEGIN_ARG GFX_SKIP_OUTPUT_BEGIN
#elif defined(GFX_SKIP_DISPLAY_BEGIN)
  #define CANVAS_BEGIN_ARG GFX_SKIP_DISPLAY_BEGIN
#else
  #define CANVAS_BEGIN_ARG GFX_NOT_DEFINED
#endif

CanvasT *cv = nullptr;
bool canvasOK = false;
inline void cvClear() { cv->fillRect(CVX - OX, CVY - OY, CVW, CVH, COL_BG); }   // hapus area animasi saja
inline void cvClearAll() { cv->fillRect(0, 0, 240, 240, COL_BG); }                 // hapus seluruh kanvas (dibatasi mode clip)
uint16_t COL_MOUTH, COL_TONGUE, COL_TEAR;

enum FaceEv { EV_NONE, EV_HAPPY, EV_LOVE, EV_WINK, EV_SURPRISE, EV_CURIOUS, EV_DIZZY,
              EV_MUSIC, EV_COOL, EV_SNEEZE, EV_VROOM, EV_TONGUE, EV_PEEK,
              EV_SHY, EV_ANGRY, EV_SPARKLE, EV_KISS, EV_THINK, EV_DANCE,
              EV_LAUGH, EV_CRY, EV_STAREYES, EV_HELMET, EV_BIKE, EV_COFFEE, EV_HELLO, EV_RADAR,
              EV_SIGNAL, EV_BATT, EV_RAIN, EV_ROCKET, EV_PONG, EV_FLAG, EV_FUEL, EV_BALLOON,
              EV_FIREWORK, EV_MAGNIFY, EV_GAME, EV_GHOST, EV_CAT, EV_THUNDER, EV_TRAFFIC, EV_EAT,
              EV_YAWN, EV_COUNT };
const uint16_t EV_DUR[EV_COUNT] = {0, 1500, 2800, 1000, 1500, 3400, 2400, 3800, 3400, 1900, 2400, 1700, 2200, 2600, 2800, 2600, 2400, 3400, 3800,
                                  2400, 3200, 2800, 3000, 3600, 3800, 2600, 3400,
                                  3200, 3400, 3400, 3200, 3800, 3000, 3200, 3400,
                                  3600, 3600, 3800, 3000, 3400, 3000, 4000, 3600, 3200};

struct FaceP {
  float openL, openR, happy, lookX, lookY, browY, browR, browTilt, browA,
        smile, mouthOpen, mouthW, tongue, blush, love, spiral, jump, shake;
};
FaceP Fc = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0.5f, 0, 28, 0, 0.3f, 0, 0, 0, 0};
FaceP Ft = Fc;

int      curEv = EV_NONE, lastEv = EV_NONE;
uint32_t evStartMs = 0, evEndMs = 0, nextEvMs = 0;
bool     yawned = false, dblBlink = false;
float    gzTx = 0, gzTy = 0;
int      fxEv = EV_NONE;          // aksi yang efek tambahannya sedang digambar (bisa fade keluar)
float    fxFade = 0, frameDt = 20;
int      bgCur = 0, bgPrev = 0;   // gaya latar belakang sekarang / sebelumnya
float    bgMix = 1;
uint32_t bgNext = 0;
float    glR = 10, glG = 60, glB = 28;   // warna cahaya latar (dihaluskan)
float    bellScale = 1;

const uint8_t STAR_X[5] = {70, 170, 120, 88, 152};
const uint8_t STAR_Y[5] = {38, 42, 26, 206, 204};
int8_t   starSz[5] = {-1, -1, -1, -1, -1};
int      orbPX[4], orbPY[4];
bool     orbOn = false;

void resetStandbyFx() {
  orbOn = false;
  for (int i = 0; i < 5; i++) starSz[i] = -1;
}

uint16_t mix565(uint16_t a, uint16_t b, float t) {
  t = constrain(t, 0.0f, 1.0f);
  int ar = (a >> 11) & 31, ag = (a >> 5) & 63, ab = a & 31;
  int br = (b >> 11) & 31, bgc = (b >> 5) & 63, bb = b & 31;
  return (uint16_t)((((int)(ar + (br - ar) * t)) << 11) | (((int)(ag + (bgc - ag) * t)) << 5) | (int)(ab + (bb - ab) * t));
}

float ap(float cur, float tgt, float tau, float dt) {
  return cur + (tgt - cur) * (1.0f - expf(-dt / tau));
}

inline uint16_t fx(uint16_t c) { float f = fxFade; return mix565(COL_BG, c, f * f * (3.0f - 2.0f * f)); }   // fade efek aksi

// Pembungkus gambar: koordinat layar penuh, otomatis digeser ke kanvas
inline void cCircle(int x, int y, int r, uint16_t c) { cv->fillCircle(x - OX, y - OY, r, c); }
inline void cRRect(int x, int y, int w, int h, int r, uint16_t c) { cv->fillRoundRect(x - OX, y - OY, w, h, r, c); }
inline void cRect(int x, int y, int w, int h, uint16_t c) { cv->fillRect(x - OX, y - OY, w, h, c); }
inline void cLine(int x0, int y0, int x1, int y1, uint16_t c) { cv->drawLine(x0 - OX, y0 - OY, x1 - OX, y1 - OY, c); }
inline void cArc(int x, int y, int r1, int r2, float a0, float a1, uint16_t c) { cv->fillArc(x - OX, y - OY, r1, r2, a0, a1, c); }

void cText(const char *s, int x, int y, uint8_t size, uint16_t col) {
  cv->setTextSize(size);
  cv->setTextColor(col);
  cv->setCursor(x - OX, y - OY);
  cv->print(s);
}

void cTextCenter(const char *s, int y, uint8_t size, uint16_t col) {
  int16_t x1, y1;
  uint16_t w, h;
  cv->setTextSize(size);
  cv->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  cv->setTextColor(col);
  cv->setCursor(120 - OX - (int)w / 2 - x1, y - OY);
  cv->print(s);
}

void cHeart(int cx, int cy, int s, uint16_t c) {
  cCircle(cx - s / 2, cy - s / 3, s / 2 + 1, c);
  cCircle(cx + s / 2, cy - s / 3, s / 2 + 1, c);
  cv->fillTriangle(cx - s - 1 - OX, cy - s / 4 - OY, cx + s + 1 - OX, cy - s / 4 - OY, cx - OX, cy + s - OY, c);
}

void drawNote(int x, int y, uint16_t c) {
  cCircle(x, y, 3, c);
  cLine(x + 3, y, x + 3, y - 12, c);
  cLine(x + 4, y, x + 4, y - 12, c);
  cLine(x + 3, y - 12, x + 9, y - 8, c);
  cLine(x + 4, y - 12, x + 10, y - 8, c);
}

void drawSpiral(int cx, int cy, float a0, float scale, uint16_t col) {
  int px = cx, py = cy;
  for (int k = 1; k <= 26; k++) {
    float ang = a0 + k * 0.55f;
    float r = k * 0.65f * scale;
    int x = cx + (int)(cosf(ang) * r), y = cy + (int)(sinf(ang) * r);
    cLine(px, py, x, y, col);
    cLine(px + 1, py, x + 1, y, col);
    px = x; py = y;
  }
}

// Bintang lima titik (dipakai mata bintang)
void cStar(int cx, int cy, int R, int r, float rot, uint16_t c) {
  int px[10], py[10];
  for (int i = 0; i < 10; i++) {
    float a = rot + i * 0.62832f - 1.5708f;
    int rr = (i & 1) ? r : R;
    px[i] = cx + (int)(cosf(a) * rr);
    py[i] = cy + (int)(sinf(a) * rr);
  }
  for (int i = 0; i < 10; i++) {
    int j = (i + 1) % 10;
    cv->fillTriangle(cx - OX, cy - OY, px[i] - OX, py[i] - OY, px[j] - OX, py[j] - OY, c);
  }
}

// ---------- Latar belakang standby ----------
// Dihitung dari waktu (tanpa array status); warna dicampur ke hitam agar bisa fade/crossfade
inline uint16_t bgc(uint16_t c, float a) { return mix565(COL_BG, c, constrain(a, 0.0f, 1.0f)); }

void drawBgLayer(int style, float a, uint32_t t, uint16_t c1, uint16_t c2) {
  if (a < 0.02f) return;
  float ft = (float)t;
  switch (style) {
    case 0:                                                        // kunang-kunang naik
      for (int i = 0; i < 12; i++) {
        float u = fmodf(ft * 0.00005f * (0.6f + 0.12f * (i % 5)) + i * 0.173f, 1.0f);
        int y = CVY + CVH - (int)(u * CVH);
        int x = CVX + 6 + (int)fmodf(i * 53.7f, CVW - 12) + (int)(sinf(ft * 0.0012f + i * 1.7f) * 8.0f);
        float tw = 0.5f + 0.5f * sinf(ft * 0.004f + i);
        cCircle(x, y, (i % 3 == 0) ? 2 : 1, bgc((i & 1) ? c2 : c1, a * sinf(u * 3.1416f) * (0.4f + 0.6f * tw)));
      }
      break;
    case 1: {                                                      // gelombang mengalir
      static const int base[3] = {64, 186, 178};
      for (int w = 0; w < 3; w++) {
        uint16_t col = bgc(w == 2 ? c2 : c1, a * (w == 2 ? 0.3f : 0.45f));
        int px = 0, py = 0;
        for (int x = CVX; x <= CVX + CVW; x += 8) {
          int y = base[w] + (int)(sinf(x * 0.045f + ft * 0.0018f * (w + 1) + w * 2.0f) * 6.0f);
          if (x > CVX) cLine(px, py, x, y, col);
          px = x; py = y;
        }
      }
      break;
    }
    case 2:                                                        // riak melebar dari tengah
      for (int k = 0; k < 3; k++) {
        float ph = fmodf(ft / 2600.0f + k / 3.0f, 1.0f);
        cv->drawCircle(120 - OX, 124 - OY, 14 + (int)(ph * 54.0f), bgc(c1, a * (1.0f - ph) * 0.8f));
      }
      break;
    case 3:                                                        // warp bintang keluar dari tengah
      for (int i = 0; i < 16; i++) {
        float ph = fmodf(ft / 3000.0f + i * 0.0625f, 1.0f);
        float ang = i * 2.39996f;
        float r = 6.0f + ph * ph * 84.0f;
        int x = 120 + (int)(cosf(ang) * r * 1.25f), y = 124 + (int)(sinf(ang) * r * 0.85f);
        if (x < CVX || x >= CVX + CVW || y < CVY || y >= CVY + CVH) continue;
        cCircle(x, y, ph > 0.65f ? 2 : 1, bgc(c1, a * ((ph < 0.2f) ? ph * 5.0f : 1.0f)));
      }
      break;
    case 4:                                                        // gelembung (tidur)
      for (int i = 0; i < 8; i++) {
        float u = fmodf(ft * 0.00003f * (0.7f + 0.1f * (i % 4)) + i * 0.125f, 1.0f);
        int y = CVY + CVH - (int)(u * CVH);
        int x = CVX + 10 + (int)fmodf(i * 37.3f, CVW - 20) + (int)(sinf(ft * 0.001f + i) * 6.0f);
        cv->drawCircle(x - OX, y - OY, 2 + (i % 3) * 2, bgc(c1, a * sinf(u * 3.1416f) * 0.8f));
      }
      break;
    case 6:                                                        // planet mengorbit
      for (int i = 0; i < 6; i++) {
        float ang = ft * 0.0009f * ((i & 1) ? -1.0f : 1.0f) + i * 1.0472f;
        int x = 120 + (int)(cosf(ang) * (70 + (i % 3) * 6)), y = 124 + (int)(sinf(ang) * (44 + (i % 3) * 5));
        cCircle(x, y, (i & 1) ? 2 : 3, bgc((i & 1) ? c2 : c1, a * 0.8f));
      }
      break;
    case 7:                                                        // hujan meteor
      for (int i = 0; i < 4; i++) {
        float u = fmodf(ft / 2200.0f + i * 0.25f, 1.0f);
        int x = CVX + CVW - 10 - (int)(u * 150.0f) - i * 23;
        int y = CVY + 4 + (i * 29) % 50 + (int)(u * 90.0f);
        uint16_t mc = bgc(c2, a * sinf(u * 3.1416f));
        cLine(x, y, x + 12, y - 7, mc);
        cCircle(x, y, 1, bgc(COL_TEXT, a * sinf(u * 3.1416f)));
      }
      break;
    case 8:                                                        // salju turun pelan
      for (int i = 0; i < 14; i++) {
        float u = fmodf(ft * 0.00006f * (0.6f + 0.1f * (i % 5)) + i * 0.0714f, 1.0f);
        int y = CVY + (int)(u * CVH);
        int x = CVX + 4 + (int)fmodf(i * 47.9f, CVW - 8) + (int)(sinf(ft * 0.0015f + i) * 7.0f);
        cCircle(x, y, (i % 4 == 0) ? 2 : 1, bgc(COL_TEXT, a * 0.6f));
      }
      break;
    default:                                                       // hujan (Bluetooth putus)
      for (int i = 0; i < 14; i++) {
        float u = fmodf(ft * 0.0007f * (0.8f + 0.1f * (i % 4)) + i * 0.071f, 1.0f);
        int y = CVY + (int)(u * CVH);
        int x = CVX + 4 + (int)fmodf(i * 41.3f, CVW - 8);
        cLine(x, y, x - 1, y + 6, bgc(c1, a * 0.7f));
      }
      break;
  }
}

void drawBg(uint32_t now, int mood, float dt) {
  int want = (mood == MOOD_SLEEP) ? 4 : (mood == MOOD_OFF) ? 5 : bgCur;
  static const uint8_t BGL[7] = {0, 1, 2, 3, 6, 7, 8};             // gaya latar untuk suasana senang
  bool happyStyle = (bgCur < 4 || bgCur >= 6);
  if (mood == MOOD_HAPPY && !happyStyle) want = 0;
  else if (mood == MOOD_HAPPY && now >= bgNext) {
    int k = 0;
    while (k < 7 && BGL[k] != bgCur) k++;
    want = BGL[(k + 1) % 7];
  }
  if (want != bgCur) {
    bgPrev = bgCur; bgCur = want; bgMix = 0;
    bgNext = now + random(11000, 17000);
  }
  bgMix = min(1.0f, bgMix + dt / 1200.0f);
  float s = bgMix * bgMix * (3.0f - 2.0f * bgMix);                // smoothstep
  uint16_t c1 = (mood == MOOD_OFF) ? COL_RED : (mood == MOOD_SLEEP) ? COL_DIM : COL_GREEN;
  uint16_t c2 = (mood == MOOD_HAPPY) ? COL_ORANGE : c1;
  if (bgMix < 1.0f) drawBgLayer(bgPrev, 1.0f - s, now, c1, c2);
  drawBgLayer(bgCur, s, now, c1, c2);
}

// Cahaya napas di belakang wajah; warnanya berubah halus mengikuti suasana/aksi
void drawGlow(uint32_t now, int mood, float dt) {
  float tr = 10, tg = 60, tb = 28;
  if (mood == MOOD_SLEEP)      { tr = 6;  tg = 14; tb = 44; }
  else if (mood == MOOD_OFF)   { tr = 70; tg = 14; tb = 12; }
  else switch (fxEv) {
    case EV_LOVE: case EV_KISS: case EV_SHY:             tr = 80; tg = 18; tb = 34; break;
    case EV_ANGRY:                                       tr = 96; tg = 12; tb = 8;  break;
    case EV_MUSIC: case EV_DANCE: case EV_VROOM:
    case EV_BIKE: case EV_HELMET: case EV_COFFEE:
    case EV_FUEL: case EV_ROCKET: case EV_FLAG:          tr = 84; tg = 50; tb = 10; break;
    case EV_HAPPY: case EV_SPARKLE: case EV_TONGUE:
    case EV_LAUGH: case EV_STAREYES: case EV_HELLO:
    case EV_BALLOON: case EV_PONG: case EV_FIREWORK: case EV_THUNDER: tr = 70; tg = 62; tb = 14; break;
    case EV_GAME: case EV_GHOST:                         tr = 14; tg = 30; tb = 80; break;
    case EV_TRAFFIC: case EV_CAT: case EV_EAT:           tr = 84; tg = 50; tb = 10; break;
    case EV_COOL: case EV_THINK: case EV_CRY: case EV_RAIN: case EV_SIGNAL: tr = 14; tg = 40; tb = 80; break;
    default: break;
  }
  glR = ap(glR, tr, 260, dt); glG = ap(glG, tg, 260, dt); glB = ap(glB, tb, 260, dt);
  float br = (0.8f + 0.2f * sinf(now / 1100.0f)) * ((mood == MOOD_SLEEP) ? 0.6f : 1.0f);
  static const uint8_t RAD[3] = {64, 50, 36};
  static const float K[3] = {0.35f, 0.65f, 1.0f};
  for (int i = 0; i < 3; i++) {
    uint16_t gcol;
    if (lightTheme) {                                              // tema terang: cahaya berupa semburat pastel tipis
      gcol = mix565(COL_BG, gfx->color565((uint8_t)min(255.0f, glR * 3.5f), (uint8_t)min(255.0f, glG * 3.5f), (uint8_t)min(255.0f, glB * 3.5f)),
                    0.10f + 0.16f * br * K[i]);
    } else {
      gcol = gfx->color565((uint8_t)(glR * br * K[i]), (uint8_t)(glG * br * K[i]), (uint8_t)(glB * br * K[i]));
    }
    cCircle(120, 124, RAD[i], gcol);
  }
}

// Efek tambahan untuk 6 aksi baru (fade ikut fxFade)
void drawFxNew(float p, int dx, int dy, int cxL, int cxR, int ey) {
  switch (fxEv) {
    case EV_SPARKLE: {
      int s = (int)(7.0f * (0.7f + 0.3f * sinf(p * 0.02f)));
      uint16_t w = fx(COL_TEXT);
      int sx[2] = {cxL - 4, cxR - 4};
      for (int k = 0; k < 2; k++) {                                // kilau di mata
        cLine(sx[k] - s, ey - 8, sx[k] + s, ey - 8, w);
        cLine(sx[k], ey - 8 - s, sx[k], ey - 8 + s, w);
        cCircle(sx[k] + 9, ey + 6, 2, w);
      }
      static const int px[4] = {48, 192, 62, 180};
      static const int py[4] = {88, 92, 160, 164};
      for (int i = 0; i < 4; i++) {                                // bintang kecil berkelip
        int r = (int)(sinf(fmodf(p + i * 350.0f, 1400.0f) / 1400.0f * 3.1416f) * 6.0f);
        if (r < 1) continue;
        uint16_t c = fx((i & 1) ? COL_ORANGE : COL_TEXT);
        cLine(px[i] - r, py[i] + dy, px[i] + r, py[i] + dy, c);
        cLine(px[i], py[i] + dy - r, px[i], py[i] + dy + r, c);
      }
      break;
    }
    case EV_ANGRY: {
      for (int side = 0; side < 2; side++)                         // uap naik dari kepala
        for (int i = 0; i < 3; i++) {
          float q = fmodf(p + i * 300.0f + side * 150.0f, 900.0f) / 900.0f;
          cCircle((side ? 192 : 48) + (side ? 1 : -1) * (int)(q * 8.0f), 108 + dy - (int)(q * 30.0f),
                  2 + (int)(q * 5.0f), mix565(COL_BG, COL_DIM, (1.0f - q) * fxFade));
        }
      uint16_t rc = mix565(COL_BG, COL_RED, (0.6f + 0.4f * sinf(p * 0.03f)) * fxFade);
      for (int k = 0; k < 4; k++) {                                // tanda kesal di dahi
        int sx = (k & 1) ? 1 : -1, sy = (k & 2) ? 1 : -1;
        int ax = 178 + sx * 3, ay = 70 + dy + sy * 3;
        cLine(ax, ay, ax + sx * 6, ay, rc);
        cLine(ax, ay + 1, ax + sx * 6, ay + 1, rc);
        cLine(ax, ay, ax, ay + sy * 6, rc);
        cLine(ax + 1, ay, ax + 1, ay + sy * 6, rc);
      }
      break;
    }
    case EV_KISS:
      if (p > 600 && p < 1900) {                                   // hati terbang dari mulut
        float q = (p - 600.0f) / 1300.0f;
        int hx = 128 + (int)(q * 52.0f), hy = 140 + dy - (int)(q * 62.0f) - (int)(sinf(q * 9.0f) * 4.0f);
        cHeart(hx, hy, 4 + (int)(q * 5.0f), fx(q > 0.75f ? mix565(COL_RED, COL_BG, (q - 0.75f) * 4.0f) : COL_RED));
      }
      break;
    case EV_THINK: {
      float g = constrain(p / 300.0f, 0.0f, 1.0f);
      cCircle(179, 82, 2, fx(COL_DIM));
      cCircle(182, 77, 3, fx(COL_DIM));
      int r = (int)(13.0f * g);
      if (r > 2) cCircle(188, 70, r, mix565(COL_BG, COL_DIM, 0.4f * fxFade));
      int n = ((int)(p / 450.0f)) % 4;                             // titik-titik muncul bergantian
      for (int i = 0; i < n && g > 0.9f; i++) cCircle(180 + i * 8, 70, 2, fx(COL_TEXT));
      break;
    }
    case EV_DANCE:
      for (int i = 0; i < 12; i++) {                               // konfeti jatuh
        int x = CVX + 6 + (int)fmodf(i * 61.7f, CVW - 12) + (int)(sinf(p * 0.006f + i) * 4.0f);
        uint16_t cc = (i % 4 == 0) ? COL_ORANGE : (i % 4 == 1) ? COL_GREEN : (i % 4 == 2) ? COL_TEXT : COL_RED;
        cRect(x, CVY + (int)fmodf(p * 0.05f + i * 37.0f, (float)CVH), 3, 3, fx(cc));
      }
      break;
    case EV_SHY: {
      uint16_t rc = fx(COL_RED);
      for (int k = 0; k < 3; k++) {                                // garis pipi merona
        cLine(42 + k * 5, 134 + dy, 46 + k * 5, 142 + dy, rc);
        cLine(182 + k * 5, 134 + dy, 186 + k * 5, 142 + dy, rc);
      }
      break;
    }
    default: break;
  }
}

// Efek di belakang wajah (digambar sebelum mata)
void drawFxBack(float p) {
  if (fxEv != EV_RADAR) return;
  int cx = 120, cy = 124;
  uint16_t gc = mix565(COL_BG, COL_GREEN, 0.5f * fxFade);
  cv->drawCircle(cx - OX, cy - OY, 62, gc);
  cv->drawCircle(cx - OX, cy - OY, 40, mix565(COL_BG, COL_GREEN, 0.25f * fxFade));
  float ang = p * 0.006f;
  for (int k = 6; k >= 0; k--) {                                   // garis sapuan dengan ekor memudar
    float a = ang - k * 0.15f;
    cLine(cx, cy, cx + (int)(cosf(a) * 62.0f), cy + (int)(sinf(a) * 62.0f),
          mix565(COL_BG, COL_GREEN, (1.0f - k / 7.0f) * 0.8f * fxFade));
  }
  static const float BA[3] = {0.9f, 2.6f, 4.4f};                   // titik target menyala saat tersapu
  static const int BRAD[3] = {50, 34, 56};
  for (int i = 0; i < 3; i++) {
    float d = fmodf(fmodf(ang - BA[i], 6.2832f) + 6.2832f, 6.2832f);
    float al = 1.0f - d / 6.2832f;
    cCircle(cx + (int)(cosf(BA[i]) * BRAD[i]), cy + (int)(sinf(BA[i]) * BRAD[i]), 3,
            mix565(COL_BG, COL_ORANGE, al * al * fxFade));
  }
}

// Efek depan untuk 8 aksi v12
void drawFxNew2(float p, int dx, int dy, int cxL, int cxR, int ey) {
  float dur = (fxEv >= 0 && fxEv < EV_COUNT) ? (float)EV_DUR[fxEv] : 1.0f;
  switch (fxEv) {
    case EV_LAUGH: {
      cTextCenter("HA HA", 62 + (int)(sinf(p * 0.03f) * 3.0f), 2, fx(COL_ORANGE));
      for (int side = 0; side < 2; side++)                         // air mata tawa menyembur
        for (int i = 0; i < 2; i++) {
          float q = fmodf(p + i * 350.0f + side * 120.0f, 700.0f) / 700.0f;
          int x = (side ? cxR + 24 : cxL - 24) + (side ? 1 : -1) * (int)(q * 16.0f);
          int y = ey + 4 - (int)(q * 12.0f) + (int)(q * q * 34.0f);
          cCircle(x, y + dy, 2, mix565(COL_BG, COL_TEAR, (1.0f - q * 0.5f) * fxFade));
        }
      break;
    }
    case EV_CRY: {
      for (int side = 0; side < 2; side++) {
        int ex = side ? cxR : cxL;
        cRect(ex - 3 + (side ? 6 : -6), ey + 20 + dy, 2, 10, mix565(COL_BG, COL_TEAR, 0.6f * fxFade));
        for (int i = 0; i < 2; i++) {                              // tetes air mata jatuh
          float q = fmodf(p * 1.0f + i * 420.0f + side * 200.0f, 840.0f) / 840.0f;
          int y = ey + 22 + dy + (int)(q * 56.0f);
          cCircle(ex + (side ? 6 : -6), y, 3 - (int)(q * 1.5f), mix565(COL_BG, COL_TEAR, (1.0f - q * 0.6f) * fxFade));
        }
      }
      break;
    }
    case EV_STAREYES: {
      for (int k = 0; k < 4; k++) {                                // bintang kecil mengorbit
        float a = p * 0.003f + k * 1.5708f;
        cStar(120 + (int)(cosf(a) * 74.0f), 118 + dy + (int)(sinf(a) * 40.0f), 5, 2, a, fx((k & 1) ? COL_ORANGE : COL_TEXT));
      }
      break;
    }
    case EV_HELMET: {
      float off;
      if (p < 380.0f)               { float t = 1.0f - p / 380.0f; off = -90.0f * t * t; }
      else if (p < 560.0f)          off = -sinf((p - 380.0f) / 180.0f * 3.1416f) * 5.0f;
      else if (p > dur - 400.0f)    off = -((p - (dur - 400.0f)) / 400.0f) * 100.0f;
      else                          off = 0;
      int oy = dy + (int)off;
      uint16_t w = COL_TEXT;
      cRRect(48 + dx, 62 + oy, 144, 16, 8, w);                     // ubun-ubun helm
      cRRect(44 + dx, 70 + oy, 12, 84, 6, w);                      // pelindung pipi
      cRRect(184 + dx, 70 + oy, 12, 84, 6, w);
      cRRect(70 + dx, 154 + oy, 100, 6, 3, w);                     // tali dagu
      cRect(70 + dx, 68 + oy, 100, 3, COL_ORANGE);                 // garis oranye
      break;
    }
    case EV_BIKE: {
      float q = constrain(p / dur, 0.0f, 1.0f);
      int bx = CVX - 34 + (int)(q * (CVW + 70)), by = 70;
      for (int i = 0; i < 4; i++)                                  // asap knalpot
        cCircle(bx - 6 - i * 7, by + 10 - i, 1 + i / 2, mix565(COL_BG, COL_DIM, (1.0f - i * 0.22f) * fxFade));
      for (int w = 0; w < 2; w++) {
        int wx = bx + w * 26;
        cCircle(wx, by + 8, 7, COL_TEXT);
        cCircle(wx, by + 8, 4, COL_BG);
        float a = p * 0.03f;
        cLine(wx, by + 8, wx + (int)(cosf(a) * 4.0f), by + 8 + (int)(sinf(a) * 4.0f), COL_DIM);
      }
      cLine(bx, by + 8, bx + 12, by, COL_ORANGE);                  // rangka
      cLine(bx + 12, by, bx + 26, by + 8, COL_ORANGE);
      cLine(bx + 12, by, bx + 22, by - 3, COL_ORANGE);
      cRRect(bx + 6, by - 4, 12, 5, 2, COL_TEXT);                  // jok
      cCircle(bx + 14, by - 12, 4, COL_TEXT);                      // pengendara
      cLine(bx + 12, by - 8, bx + 10, by - 1, COL_TEXT);
      cLine(bx + 14, by - 8, bx + 22, by - 3, COL_TEXT);
      break;
    }
    case EV_COFFEE: {
      float t = constrain(p / 350.0f, 0.0f, 1.0f);
      float u = (p > dur - 350.0f) ? (p - (dur - 350.0f)) / 350.0f : 0.0f;
      int x = 156 + (int)((1.0f - t + u) * 60.0f), y = 148 + dy;
      cRRect(x, y, 26, 22, 5, COL_TEXT);
      cRRect(x + 3, y + 3, 20, 5, 2, mix565(COL_BG, COL_ORANGE, 0.55f));
      cCircle(x + 29, y + 11, 6, COL_TEXT);
      cCircle(x + 29, y + 11, 3, COL_BG);
      cRRect(x - 3, y + 21, 32, 4, 2, COL_DIM);                    // tatakan
      for (int i = 0; i < 3; i++) {                                // uap naik bergelombang
        float q = fmodf(p + i * 400.0f, 1200.0f) / 1200.0f;
        cCircle(x + 6 + i * 7 + (int)(sinf(q * 8.0f + i) * 3.0f), y - 4 - (int)(q * 26.0f), 2,
                mix565(COL_BG, COL_DIM, (1.0f - q) * fxFade));
      }
      break;
    }
    case EV_HELLO: {
      cTextCenter("HAI!", 60 + (int)(sinf(p * 0.012f) * 3.0f), 3, fx(COL_ORANGE));
      float a = sinf(p * 0.014f) * 0.55f;
      int wx = 196, wy = 176 + dy;
      int hx = wx + (int)(sinf(a) * 16.0f), hy = wy - (int)(cosf(a) * 16.0f);
      cLine(wx, wy, hx, hy, fx(COL_TEXT));
      cLine(wx + 1, wy, hx + 1, hy, fx(COL_TEXT));
      cCircle(hx, hy, 7, fx(COL_TEXT));
      for (int k = 0; k < 4; k++) {                                // jari-jari terbuka
        float fa = a + (k - 1.5f) * 0.3f;
        int fx2 = hx + (int)(sinf(fa) * 16.0f), fy2 = hy - (int)(cosf(fa) * 16.0f);
        cLine(hx, hy, fx2, fy2, fx(COL_TEXT));
        cLine(hx + 1, hy, fx2 + 1, fy2, fx(COL_TEXT));
      }
      break;
    }
    default: break;
  }
}

inline void cTri(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c) {
  cv->fillTriangle(x0 - OX, y0 - OY, x1 - OX, y1 - OY, x2 - OX, y2 - OY, c);
}

// Efek depan untuk 8 aksi v13
void drawFxNew3(float p, int dx, int dy, int cxL, int cxR, int ey) {
  float dur = (fxEv >= 0 && fxEv < EV_COUNT) ? (float)EV_DUR[fxEv] : 1.0f;
  float q = constrain(p / dur, 0.0f, 1.0f);
  switch (fxEv) {
    case EV_SIGNAL: {
      int lit = ((int)(p / 260.0f)) % 4;                           // busur menyala bergiliran
      for (int k = 0; k < 3; k++)
        cArc(120, 90, 10 + k * 9 + 4, 10 + k * 9, 225, 315, lit > k ? fx(COL_ORANGE) : fx(COL_RING_BG));
      cCircle(120, 90, 3, fx(COL_ORANGE));
      break;
    }
    case EV_BATT: {
      float lvl = 0.6f - 0.55f * q;
      cRRect(96, 60, 48, 20, 4, fx(COL_TEXT));
      cRRect(98, 62, 44, 16, 3, COL_BG);
      cRect(144, 66, 4, 8, fx(COL_TEXT));
      if (!(lvl < 0.2f && ((int)(p / 200.0f)) % 2)) {
        int w = max(2, (int)(40.0f * lvl));
        cRRect(100, 64, w, 12, 2, fx(lvl < 0.3f ? COL_RED : COL_ORANGE));
      }
      break;
    }
    case EV_RAIN: {
      cArc(120, 84, 30, 24, 180, 360, fx(COL_ORANGE));             // payung
      cLine(120, 84, 120, 98, fx(COL_DIM));
      for (int i = 0; i < 12; i++) {
        int x = 40 + i * 13 + (i % 3) * 3;
        int y = 58 + (int)fmodf(p * 0.28f + i * 41.0f, 134.0f);
        cLine(x, y, x - 2, y + 6, mix565(COL_BG, COL_TEAR, 0.7f * fxFade));
      }
      break;
    }
    case EV_ROCKET: {
      int x = 196 + (int)(sinf(p * 0.02f) * 2.0f), y = 206 - (int)(q * 160.0f);
      for (int i = 0; i < 5; i++)                                  // jejak asap
        cCircle(x + (int)(sinf(p * 0.01f + i) * 2.0f), y + 16 + i * 9, 2 + i / 2, mix565(COL_BG, COL_DIM, (1.0f - i * 0.18f) * fxFade));
      int fl = 8 + (int)(sinf(p * 0.06f) * 3.0f);
      cTri(x - 4, y + 12, x + 4, y + 12, x, y + 12 + fl, COL_ORANGE);
      cRRect(x - 6, y - 18, 12, 30, 5, COL_TEXT);
      cTri(x - 6, y - 16, x + 6, y - 16, x, y - 30, COL_RED);
      cTri(x - 6, y + 4, x - 12, y + 14, x - 6, y + 12, COL_RED);
      cTri(x + 6, y + 4, x + 12, y + 14, x + 6, y + 12, COL_RED);
      cCircle(x, y - 6, 3, COL_TEAR);
      break;
    }
    case EV_PONG: {
      float tri = fabsf(fmodf(p / 800.0f, 2.0f) - 1.0f);
      int bx = 62 + (int)(tri * 116.0f), by = 76 + (int)(sinf(p * 0.011f) * 8.0f);
      cRRect(52, by - 10, 4, 20, 2, fx(COL_TEXT));
      cRRect(184, 66 + (int)(sinf(p * 0.009f) * 8.0f), 4, 20, 2, fx(COL_TEXT));
      cCircle(bx, by, 4, fx(COL_ORANGE));
      break;
    }
    case EV_FLAG: {
      cLine(150, 62, 150, 102, fx(COL_DIM));
      cLine(151, 62, 151, 102, fx(COL_DIM));
      for (int i = 0; i < 6; i++)
        for (int j = 0; j < 3; j++) {
          int off = (int)(sinf(p * 0.012f + i * 0.8f) * 3.0f);
          cRect(152 + i * 8, 62 + j * 7 + off, 8, 7, fx(((i + j) & 1) ? COL_TEXT : COL_RING_BG));
        }
      break;
    }
    case EV_FUEL: {
      float lvl = 0.5f - 0.45f * q;
      cArc(120, 92, 30, 24, 180, 360, fx(COL_RING_BG));
      cArc(120, 92, 30, 24, 180, 180 + 180.0f * lvl, fx(lvl < 0.25f ? COL_RED : COL_GREEN));
      float a = 3.1416f + 3.1416f * lvl;
      cLine(120, 92, 120 + (int)(cosf(a) * 22.0f), 92 + (int)(sinf(a) * 22.0f), fx(COL_TEXT));
      cCircle(120, 92, 3, fx(COL_TEXT));
      if (lvl < 0.25f && ((int)(p / 250.0f)) % 2) cCircle(172, 66, 4, fx(COL_RED));  // lampu peringatan
      break;
    }
    case EV_BALLOON: {
      for (int b = 0; b < 2; b++) {
        float u = constrain((p - b * 500.0f) / 2600.0f, 0.0f, 1.0f);
        if (u <= 0.0f) continue;
        int x = (b ? 176 : 64) + (int)(sinf(p * 0.004f + b * 2.0f) * 6.0f), y = 196 - (int)(u * 150.0f);
        cLine(x, y + 12, x + (int)(sinf(p * 0.005f + b) * 3.0f), y + 38, fx(COL_DIM));
        cCircle(x, y, 12, fx(b ? COL_ORANGE : COL_RED));
        cTri(x - 3, y + 17, x + 3, y + 17, x, y + 11, fx(b ? COL_ORANGE : COL_RED));
        cCircle(x - 4, y - 5, 3, mix565(COL_BG, COL_TEXT, 0.5f * fxFade));
      }
      break;
    }
    default: break;
  }
}

// Efek depan untuk 8 aksi v15
void drawFxNew4(float p, int dx, int dy, int cxL, int cxR, int ey) {
  float dur = (fxEv >= 0 && fxEv < EV_COUNT) ? (float)EV_DUR[fxEv] : 1.0f;
  float q = constrain(p / dur, 0.0f, 1.0f);
  switch (fxEv) {
    case EV_FIREWORK: {
      static const int BX[3] = {80, 160, 120}, BY[3] = {78, 72, 64};
      for (int b = 0; b < 3; b++) {
        float u = (p - b * 1000.0f) / 1200.0f;
        if (u <= 0.0f || u >= 1.0f) continue;
        if (u < 0.22f) {                                           // roket naik
          int y = 190 - (int)((u / 0.22f) * (190 - BY[b]));
          cCircle(BX[b], y, 2, fx(COL_TEXT));
          cLine(BX[b], y + 3, BX[b], y + 9, fx(COL_DIM));
        } else {                                                   // ledakan
          float e = (u - 0.22f) / 0.78f, r = e * (30 + b * 3);
          for (int k = 0; k < 12; k++) {
            float a = k * 0.5236f;
            uint16_t c = (k % 3 == 0) ? COL_ORANGE : (k % 3 == 1) ? COL_RED : COL_TEXT;
            cCircle(BX[b] + (int)(cosf(a) * r), BY[b] + (int)(sinf(a) * r + e * e * 14.0f), e < 0.6f ? 2 : 1,
                    mix565(COL_BG, c, (1.0f - e) * fxFade));
          }
        }
      }
      break;
    }
    case EV_MAGNIFY: {
      int lx = 120 + (int)(sinf(p * 0.0035f) * 52.0f), ly = 112 + dy;
      uint16_t w = fx(COL_TEXT), o = fx(COL_ORANGE);
      cArc(lx, ly, 26, 21, 0, 360, w);
      for (int k = 0; k < 3; k++) cLine(lx + 18 + (k == 1), ly + 18 + (k == 2), lx + 36 + (k == 1), ly + 36 + (k == 2), o);
      cLine(lx - 12, ly - 10, lx - 6, ly - 16, w);                 // kilau kaca
      break;
    }
    case EV_GAME: {
      int x = 44, y = 150 + dy;
      uint16_t w = fx(COL_TEXT);
      cRRect(x, y, 52, 24, 10, w);
      cRect(x + 8, y + 10, 12, 4, COL_BG);
      cRect(x + 12, y + 6, 4, 12, COL_BG);                         // tombol arah
      bool a = ((int)(p / 180.0f)) & 1;
      cCircle(x + 36, y + 14, 4, a ? fx(COL_RED) : fx(COL_DIM));
      cCircle(x + 44, y + 9, 4, a ? fx(COL_DIM) : fx(COL_GREEN));
      break;
    }
    case EV_GHOST: {
      int gx = 210 - (int)(q * 230.0f), gy = 76 + (int)(sinf(p * 0.01f) * 4.0f) + dy;
      uint16_t gc = mix565(COL_BG, COL_TEXT, 0.9f * fxFade);
      cCircle(gx, gy, 13, gc);
      cRect(gx - 13, gy, 26, 16, gc);
      for (int i = 0; i < 3; i++) cCircle(gx - 9 + i * 9, gy + 16, 4, gc);   // ujung bergelombang
      cCircle(gx - 5, gy - 2, 2, COL_BG); cCircle(gx + 5, gy - 2, 2, COL_BG); cCircle(gx, gy + 6, 2, COL_BG);
      break;
    }
    case EV_CAT: {
      uint16_t w = fx(COL_TEXT), pk = fx(COL_TONGUE);
      cTri(58, 86, 86, 86, 64, 58, w);   cTri(62, 84, 80, 84, 66, 66, pk);        // telinga
      cTri(154, 86, 182, 86, 176, 58, w); cTri(160, 84, 178, 84, 174, 66, pk);
      for (int s2 = -1; s2 <= 1; s2 += 2)                                          // kumis
        for (int k = 0; k < 3; k++)
          cLine(120 + s2 * 66, 138 + k * 6 + dy, 120 + s2 * 80, 132 + k * 12 + dy, w);
      cTri(116, 128 + dy, 124, 128 + dy, 120, 134 + dy, pk);                       // hidung
      cTextCenter("MEOW", 62 + (int)(sinf(p * 0.012f) * 2.0f), 2, fx(COL_ORANGE));
      break;
    }
    case EV_THUNDER: {
      uint16_t cc = fx(COL_DIM);
      cCircle(92, 76, 13, cc); cCircle(114, 68, 16, cc); cCircle(138, 72, 14, cc); cCircle(152, 78, 10, cc);
      cRRect(84, 76, 76, 14, 7, cc);                               // awan
      if (p > 700 && fmodf(p, 1100.0f) < 280.0f) {                 // sambaran petir
        static const int PX[4] = {124, 110, 122, 106}, PY[4] = {90, 114, 114, 146};
        for (int i = 0; i < 3; i++)
          for (int o = -1; o <= 1; o++) cLine(PX[i] + o, PY[i], PX[i + 1] + o, PY[i + 1], COL_ORANGE);
      }
      break;
    }
    case EV_TRAFFIC: {
      cRRect(176, 74, 22, 58, 6, fx(COL_CHIP));
      int ph = p < 900 ? 0 : p < 1800 ? 1 : p < 2700 ? 2 : 3;
      uint16_t lamp[3] = {COL_RED, COL_ORANGE, COL_GREEN};
      for (int i = 0; i < 3; i++) {
        bool on = (ph == i) || (ph == 3 && i == 2);
        cCircle(187, 88 + i * 16, 6, on ? fx(lamp[i]) : mix565(COL_BG, lamp[i], 0.18f * fxFade));
      }
      if (ph >= 2) cTextCenter("GO!", 62 + (int)(sinf(p * 0.02f) * 2.0f), 3, fx(COL_GREEN));
      break;
    }
    case EV_EAT: {
      if (p < 1100) {
        float e = constrain(p / 900.0f, 0.0f, 1.0f);
        e = e * e * (3.0f - 2.0f * e);
        int x = 180 - (int)(e * 50.0f), y = 164 - (int)(e * 22.0f) + dy;
        int r = p < 900 ? 12 : (int)(12.0f * (1.0f - (p - 900.0f) / 200.0f));
        if (r > 1) {                                               // donat
          cCircle(x, y, r, fx(COL_ORANGE));
          cCircle(x, y, r / 2, COL_BG);
          cCircle(x - r / 2, y - r / 2, 1, fx(COL_TEXT));
          cCircle(x + r / 2, y - r / 3, 1, fx(COL_RED));
          cCircle(x + r / 3, y + r / 2, 1, fx(COL_GREEN));
        }
      } else {
        for (int i = 0; i < 5; i++) {                              // remah jatuh
          float q2 = fmodf(p + i * 190.0f, 760.0f) / 760.0f;
          cCircle(114 + i * 3, 150 + dy + (int)(q2 * 22.0f), 1, mix565(COL_BG, COL_ORANGE, (1.0f - q2) * fxFade));
        }
      }
      break;
    }
    default: break;
  }
}

void drawEye(int ex, int ey, float open, float happy, uint16_t col, float lx, float ly) {
  int h = max(4, (int)(52.0f * open));
  int w = constrain(36 + (int)((1.0f - open) * 8.0f), 32, 44);   // melar saat menutup
  cRRect(ex - w / 2, ey - h / 2, w, h, min(16, h / 2), col);
  if (h > 26 && happy < 0.5f) {
    cCircle(ex - 6 + (int)(lx * 4.0f), ey - h / 2 + 11 + (int)(ly * 3.0f), 5, COL_TEXT);
    cCircle(ex + 8 + (int)(lx * 3.0f), ey + h / 4, 2, COL_TEXT);
  }
  if (happy > 0.02f) {                                              // kelopak bawah naik = mata ^ ^
    int cy0 = ey + h / 2 + 28, cy1 = ey - h / 2 + 34;
    cCircle(ex, cy0 + (int)((cy1 - cy0) * happy), 27, COL_BG);
  }
}

void drawBrow(int bx, int by, float tilt, bool isLeft, uint16_t col) {
  int inner = isLeft ? 14 : -14;
  for (int o = -1; o <= 1; o++) cLine(bx - inner, by - (int)tilt + o, bx + inner, by + (int)tilt + o, col);
}

void drawMouth(int cx, int my, float smile, float open, float w, float tongue) {
  if (open > 0.08f) {
    int mw = (int)(w * (0.55f + 0.45f * open));
    int mh = (int)(6.0f + open * 24.0f);
    cRRect(cx - mw / 2, my - 2, mw, mh, min(mw, mh) / 2, COL_MOUTH);
    if (tongue > 0.05f) {
      int th = max(2, (int)(8.0f * tongue));
      cRRect(cx - mw / 4, my - 2 + mh - th, mw / 2, th, min(th / 2, mw / 4), COL_TONGUE);
    }
  } else {
    if (tongue > 0.05f) cRRect(cx + 2, my + 2, 12, max(2, (int)(14.0f * tongue)), 5, COL_TONGUE);
    int px = 0, py = 0;
    for (int i = 0; i <= 8; i++) {
      float u = i / 8.0f;
      int x = cx - (int)(w / 2) + (int)(w * u);
      float s2 = 1.0f - (2.0f * u - 1.0f) * (2.0f * u - 1.0f);
      int y = my + (int)(smile * 7.0f * s2);
      if (i) {
        cLine(px, py, x, y, COL_TEXT);
        cLine(px, py + 1, x, y + 1, COL_TEXT);
      }
      px = x; py = y;
    }
  }
}

// Titik putih berekor mengorbit di cincin (4,5 detik per putaran)
void updateOrbit(bool connected) {
  uint16_t rc = connected ? COL_GREEN : COL_RED;
  float head = (millis() % 4500) / 4500.0f * 6.2832f - 1.5708f;
  int nx[4], ny[4];
  for (int i = 0; i < 4; i++) {
    float a = head - i * 0.09f;
    nx[i] = 120 + (int)lroundf(116.0f * cosf(a));
    ny[i] = 120 + (int)lroundf(116.0f * sinf(a));
  }
  if (orbOn && nx[0] == orbPX[0] && ny[0] == orbPY[0]) return;
  if (orbOn) for (int i = 3; i >= 0; i--) gfx->fillCircle(orbPX[i], orbPY[i], 3, rc);
  uint16_t cols[4] = {COL_TEXT, mix565(COL_TEXT, rc, 0.45f), mix565(COL_TEXT, rc, 0.7f), mix565(COL_TEXT, rc, 0.85f)};
  for (int i = 3; i >= 0; i--) {
    gfx->fillCircle(nx[i], ny[i], i == 0 ? 3 : 2, cols[i]);
    orbPX[i] = nx[i]; orbPY[i] = ny[i];
  }
  orbOn = true;
}

// Bintang berkelip di area kosong atas dan bawah wajah
void updateStars(int mood) {
  static const uint8_t wave[8] = {0, 1, 2, 3, 2, 1, 0, 0};
  uint32_t step = millis() / 220;
  for (int i = 0; i < 5; i++) {
    int sz = (mood == MOOD_HAPPY) ? wave[(step + i * 3) % 8] : 0;
    if (sz == starSz[i]) continue;
    starSz[i] = sz;
    int x = STAR_X[i], y = STAR_Y[i];
    gfx->fillRect(x - 5, y - 5, 11, 11, COL_BG);
    if (sz == 0) continue;
    uint16_t c = (i % 2) ? COL_ORANGE : COL_TEXT;
    gfx->drawFastHLine(x - sz - 1, y, 2 * sz + 3, c);
    gfx->drawFastVLine(x, y - sz - 1, 2 * sz + 3, c);
  }
}

void enterStandby(bool connected) {
  irisClose();
  uint32_t now = millis();
  standby = true;
  standbyStart = now;
  blinking = false;
  nextBlink = now + 1500;
  nextGaze = now + 1000;
  gzTx = 0; gzTy = 0;
  faceConnected = connected;
  lastFaceKey = -1;
  curEv = EV_NONE;
  lastEv = EV_NONE;
  yawned = false;
  nextEvMs = now + 3500;
  fxEv = EV_NONE; fxFade = 0;
  Fc.openL = Fc.openR = 0.05f;                 // mata membuka perlahan
  bgCur = bgPrev = 0; bgMix = 1; bgNext = now + 12000;
  resetStandbyFx();
  gfx->fillScreen(COL_BG);
  drawRing(1.0f, connected ? COL_GREEN : COL_RED);
}

void forceBlink(uint32_t now) {
  if (blinking) return;
  blinking = true; blinkStart = now; dblBlink = false;
}

void startEv(int ev, uint32_t now) {
  forceBlink(now);                             // kedip menyamarkan pergantian aksi
  fxEv = ev;
  curEv = ev;
  lastEv = ev;
  evStartMs = now;
  evEndMs = now + EV_DUR[ev];
  nextEvMs = evEndMs + random(1500, 3800);
}

// 0 = mata terbuka, 1 = tertutup. Kadang kedip dobel.
float blinkAmount(uint32_t now) {
  if (!blinking) {
    if (now < nextBlink) return 0.0f;
    blinking = true;
    blinkStart = now;
    dblBlink = (random(0, 100) < 25);
  }
  int32_t t1 = (int32_t)(now - blinkStart);
  float a = 0.0f;
  if (t1 < 170) a = sinf(3.14159f * t1 / 170.0f);
  if (dblBlink) {
    int32_t t2 = t1 - 260;
    if (t2 >= 0 && t2 < 170) a = max(a, sinf(3.14159f * t2 / 170.0f));
  }
  if (t1 >= (dblBlink ? 430 : 170)) {
    blinking = false;
    nextBlink = now + random(1800, 5200);
    a = 0.0f;
  }
  return a;
}

// Menentukan pose target; pose sebenarnya mengejar target dengan halus
void faceTargets(uint32_t now, int mood, uint32_t st) {
  float p = (curEv != EV_NONE) ? (float)(now - evStartMs) : 0.0f;
  float d = (curEv != EV_NONE) ? (float)EV_DUR[curEv] : 1.0f;
  float q = constrain(p / d, 0.0f, 1.0f);

  Ft.openL = 1; Ft.openR = 1; Ft.happy = 0; Ft.lookX = gzTx; Ft.lookY = gzTy;
  Ft.browY = 0; Ft.browR = 0; Ft.browTilt = 0; Ft.browA = 0;
  Ft.smile = 0.55f; Ft.mouthOpen = 0; Ft.mouthW = 28; Ft.tongue = 0; Ft.blush = 0.3f;
  Ft.love = 0; Ft.spiral = 0; Ft.jump = 0; Ft.shake = 0;

  if (mood == MOOD_SLEEP) {
    Ft.openL = 0.12f; Ft.openR = 0.12f; Ft.lookX = 0; Ft.lookY = 0;
    Ft.smile = 0; Ft.blush = 0.15f;
    return;
  }
  if (mood == MOOD_OFF) {                       // sedih, mencari-cari HP
    Ft.openL = 0.75f; Ft.openR = 0.75f;
    Ft.lookX = sinf(now / 1700.0f); Ft.lookY = 0.2f;
    Ft.smile = -0.7f; Ft.browA = 1; Ft.browY = 2; Ft.browTilt = -5; Ft.blush = 0;
    return;
  }

  float sl = constrain(((float)st - (SLEEP_MS - 9000)) / 5800.0f, 0.0f, 1.0f);   // mulai mengantuk
  if (sl > 0.0f) {
    float lid = 0.5f + 0.5f * sinf(now / 450.0f);
    Ft.openL = Ft.openR = 1.0f - 0.45f * sl * lid;
    Ft.smile = 0.3f;
  }

  switch (curEv) {
    case EV_HAPPY:
      Ft.happy = 1; Ft.smile = 1; Ft.blush = 1;
      Ft.jump = -fabsf(sinf(p * 0.011f)) * 7.0f * (1.0f - q);
      break;
    case EV_LOVE:
      Ft.love = 1; Ft.openL = 0; Ft.openR = 0; Ft.smile = 1; Ft.blush = 1;
      Ft.lookX = 0; Ft.lookY = 0;
      Ft.jump = -fabsf(sinf(p * 0.008f)) * 3.0f;
      break;
    case EV_WINK:
      Ft.smile = 1; Ft.blush = 0.7f; Ft.lookX = 0; Ft.lookY = 0; Ft.happy = 0.35f;
      Ft.browA = 1; Ft.browY = -2;
      if (p > 150 && p < 700) Ft.openR = 0.04f;
      break;
    case EV_SURPRISE:
      Ft.openL = 1.3f; Ft.openR = 1.3f; Ft.browA = 1; Ft.browY = -9;
      Ft.smile = 0; Ft.mouthOpen = 0.6f; Ft.mouthW = 14; Ft.blush = 0;
      Ft.lookX = 0; Ft.lookY = 0;
      Ft.jump = -fabsf(sinf(p * 0.012f)) * 10.0f * expf(-p / 500.0f);
      break;
    case EV_CURIOUS:
      Ft.smile = 0.2f; Ft.mouthW = 20; Ft.browA = 1; Ft.browR = -7; Ft.browY = -2;
      if (p < 800)       { Ft.lookX = -1; Ft.lookY = -0.6f; }
      else if (p < 1700) { Ft.lookX = 1;  Ft.lookY = -0.6f; }
      else if (p < 2500) { Ft.lookX = 0;  Ft.lookY = -1; }
      else               { Ft.lookX = 0;  Ft.lookY = 0; }
      break;
    case EV_DIZZY:
      Ft.spiral = 1; Ft.smile = sinf(p * 0.012f) * 0.8f; Ft.blush = 0.6f;
      Ft.shake = sinf(p * 0.02f) * 5.0f; Ft.jump = sinf(p * 0.02f) * 2.0f;
      Ft.lookX = 0; Ft.lookY = 0;
      break;
    case EV_MUSIC:
      Ft.happy = 1; Ft.smile = 1; Ft.blush = 0.8f; Ft.lookX = 0; Ft.lookY = 0;
      Ft.shake = sinf(p * 0.008f) * 6.0f;
      Ft.jump = -fabsf(sinf(p * 0.008f)) * 4.0f;
      break;
    case EV_COOL:
      Ft.smile = 0.9f; Ft.blush = 0.2f; Ft.lookX = 0; Ft.lookY = 0;
      Ft.jump = sinf(p * 0.004f) * 1.5f;
      break;
    case EV_SNEEZE:
      if (p < 750) {
        float u = p / 750.0f;
        Ft.lookY = -0.8f * u; Ft.openL = Ft.openR = 1.0f - 0.55f * u;
        Ft.mouthOpen = 0.7f * u; Ft.mouthW = 18; Ft.smile = 0;
        Ft.jump = -6.0f * u; Ft.browA = 1; Ft.browY = -4.0f * u;
      } else if (p < 1100) {
        Ft.openL = Ft.openR = 0.04f; Ft.mouthOpen = 0.9f; Ft.mouthW = 24; Ft.smile = 0;
        Ft.jump = 9; Ft.shake = (((int)(p / 35.0f)) % 2) ? 5.0f : -5.0f;
        Ft.lookY = 0.5f; Ft.browA = 1; Ft.browTilt = 4;
      } else {
        Ft.smile = 0.5f; Ft.blush = 0.6f;
      }
      break;
    case EV_VROOM:
      Ft.openL = Ft.openR = 0.55f; Ft.browA = 1; Ft.browTilt = 6; Ft.browY = 2;
      Ft.smile = 0.5f; Ft.mouthW = 30; Ft.lookX = 0; Ft.lookY = 0.1f; Ft.blush = 0.1f;
      Ft.shake = sinf(p * 0.09f) * 1.6f; Ft.jump = sinf(p * 0.13f) * 1.2f;
      break;
    case EV_TONGUE:
      Ft.happy = 0.5f; Ft.smile = 1; Ft.tongue = 1; Ft.mouthOpen = 0.25f; Ft.mouthW = 30; Ft.blush = 1;
      Ft.jump = -fabsf(sinf(p * 0.01f)) * 3.0f;
      if (p > 250 && p < 1400) Ft.openR = 0.05f;
      break;
    case EV_PEEK:                                  // melirik jam di bawah
      Ft.lookX = 0; Ft.lookY = 1; Ft.smile = 0.4f; Ft.browA = 1; Ft.browY = -2;
      break;
    case EV_SHY:
      Ft.lookX = (p < 1300) ? -1.0f : 0.7f; Ft.lookY = 0.6f;
      Ft.openL = Ft.openR = 0.8f; Ft.smile = 0.5f; Ft.mouthW = 18; Ft.blush = 1;
      Ft.browA = 1; Ft.browY = -1; Ft.browTilt = -3;
      Ft.shake = sinf(p * 0.012f) * 1.5f; Ft.jump = 2;
      break;
    case EV_ANGRY:
      Ft.openL = Ft.openR = 0.7f; Ft.browA = 1; Ft.browTilt = 8; Ft.browY = 5;
      Ft.smile = -0.8f; Ft.mouthW = 22; Ft.blush = 0; Ft.lookX = 0; Ft.lookY = 0.1f;
      Ft.shake = sinf(p * 0.05f) * 2.0f;
      break;
    case EV_SPARKLE:
      Ft.openL = Ft.openR = 1.15f; Ft.lookX = 0; Ft.lookY = -0.3f; Ft.smile = 0.9f;
      Ft.mouthW = 26; Ft.blush = 0.8f; Ft.browA = 1; Ft.browY = -4;
      Ft.jump = -fabsf(sinf(p * 0.006f)) * 3.0f;
      break;
    case EV_KISS:
      Ft.smile = 0; Ft.mouthOpen = 0.2f; Ft.mouthW = 9; Ft.blush = 1; Ft.lookX = 0; Ft.lookY = 0;
      Ft.happy = 0.3f;
      if (p > 350 && p < 900) Ft.openR = 0.04f;
      break;
    case EV_THINK:
      Ft.lookX = 0.9f; Ft.lookY = -0.9f; Ft.smile = 0.05f; Ft.mouthW = 14; Ft.blush = 0.1f;
      Ft.browA = 1; Ft.browR = -6; Ft.browY = -3;
      break;
    case EV_DANCE:
      Ft.happy = 1; Ft.smile = 1; Ft.blush = 0.9f; Ft.lookY = 0;
      Ft.shake = sinf(p * 0.009f) * 12.0f; Ft.lookX = sinf(p * 0.009f) * 0.6f;
      Ft.jump = -fabsf(sinf(p * 0.009f)) * 6.0f;
      break;
    case EV_LAUGH:
      Ft.happy = 1; Ft.smile = 1; Ft.blush = 1; Ft.lookX = 0; Ft.lookY = 0;
      Ft.mouthOpen = 0.55f + 0.4f * fabsf(sinf(p * 0.012f)); Ft.mouthW = 34;
      Ft.shake = sinf(p * 0.03f) * 3.0f; Ft.jump = -fabsf(sinf(p * 0.015f)) * 5.0f;
      break;
    case EV_CRY:
      Ft.openL = Ft.openR = 0.7f; Ft.browA = 1; Ft.browTilt = -6; Ft.browY = 3;
      Ft.smile = -1; Ft.mouthOpen = 0.25f + 0.1f * sinf(p * 0.02f); Ft.mouthW = 18;
      Ft.blush = 0; Ft.lookX = 0; Ft.lookY = 0.3f; Ft.shake = sinf(p * 0.025f) * 1.2f;
      break;
    case EV_STAREYES:
      Ft.smile = 1; Ft.mouthOpen = 0.35f; Ft.mouthW = 24; Ft.blush = 1; Ft.lookX = 0; Ft.lookY = 0;
      Ft.jump = -fabsf(sinf(p * 0.007f)) * 3.0f;
      break;
    case EV_HELMET:
      Ft.smile = 0.6f; Ft.mouthW = 26; Ft.lookX = 0; Ft.lookY = 0.1f; Ft.blush = 0.4f;
      if (p > 380 && p < 600) Ft.jump = 3;
      break;
    case EV_BIKE:
      Ft.lookX = -1.0f + 2.0f * q; Ft.lookY = -0.8f; Ft.smile = 0.4f; Ft.mouthW = 22;
      Ft.browA = 1; Ft.browY = -3;
      break;
    case EV_COFFEE:
      Ft.happy = 0.9f; Ft.smile = 1; Ft.blush = 0.8f; Ft.lookX = 0; Ft.lookY = 0;
      Ft.jump = sinf(p * 0.005f) * 1.5f;
      break;
    case EV_HELLO:
      Ft.happy = 0.6f; Ft.smile = 1; Ft.blush = 0.8f; Ft.browA = 1; Ft.browY = -3;
      Ft.lookX = 0.6f; Ft.lookY = 0.2f; Ft.jump = -fabsf(sinf(p * 0.01f)) * 2.0f;
      break;
    case EV_RADAR:
      Ft.openL = Ft.openR = 0.6f; Ft.browA = 1; Ft.browTilt = 3; Ft.browY = 2;
      Ft.smile = 0.1f; Ft.mouthW = 18; Ft.blush = 0; Ft.lookX = sinf(p * 0.006f); Ft.lookY = -0.1f;
      break;
    case EV_SIGNAL:
      Ft.lookX = 0; Ft.lookY = -1; Ft.browA = 1; Ft.browY = -3; Ft.smile = 0.1f; Ft.mouthW = 20; Ft.blush = 0;
      break;
    case EV_BATT:
      Ft.openL = Ft.openR = 0.5f; Ft.smile = -0.6f; Ft.mouthW = 20; Ft.blush = 0; Ft.lookX = sinf(p * 0.004f) * 0.5f;
      Ft.lookY = 0.3f; Ft.shake = (q > 0.7f) ? sinf(p * 0.05f) * 1.5f : 0;
      break;
    case EV_RAIN:
      Ft.lookX = 0; Ft.lookY = -0.6f; Ft.smile = 0.2f; Ft.mouthW = 22; Ft.blush = 0.2f;
      break;
    case EV_ROCKET:
      Ft.lookX = 0.8f; Ft.lookY = -1.0f + 1.6f * q; Ft.smile = 0.5f; Ft.mouthOpen = 0.3f; Ft.mouthW = 20; Ft.browA = 1; Ft.browY = -3;
      break;
    case EV_PONG: {
      float tri = fabsf(fmodf(p / 800.0f, 2.0f) - 1.0f);
      Ft.lookX = constrain((tri - 0.5f) * 2.0f, -1.0f, 1.0f); Ft.lookY = -0.8f; Ft.smile = 0.3f; Ft.mouthW = 22;
      break;
    }
    case EV_FLAG:
      Ft.happy = 0.8f; Ft.smile = 1; Ft.mouthOpen = 0.4f; Ft.mouthW = 28; Ft.blush = 1; Ft.lookX = 0.7f; Ft.lookY = -0.5f;
      Ft.jump = -fabsf(sinf(p * 0.01f)) * 3.0f;
      break;
    case EV_FUEL:
      Ft.lookX = 0; Ft.lookY = -1; Ft.smile = -0.5f; Ft.mouthW = 18; Ft.browA = 1; Ft.browY = -4; Ft.blush = 0;
      Ft.shake = (q > 0.6f) ? sinf(p * 0.04f) * 1.5f : 0;
      break;
    case EV_BALLOON:
      Ft.happy = 0.5f; Ft.smile = 1; Ft.blush = 0.8f; Ft.lookX = sinf(p * 0.004f); Ft.lookY = -1;
      Ft.jump = sinf(p * 0.006f) * 2.0f;
      break;
    case EV_FIREWORK: {
      static const float LX[3] = {-0.7f, 0.7f, 0.0f};
      int bi = min(2, (int)(p / 1000.0f));
      Ft.lookX = LX[bi]; Ft.lookY = -0.9f; Ft.happy = 0.3f; Ft.smile = 1; Ft.mouthOpen = 0.4f; Ft.mouthW = 22; Ft.blush = 0.6f;
      break;
    }
    case EV_MAGNIFY:
      Ft.lookX = sinf(p * 0.0035f); Ft.lookY = 0; Ft.smile = 0.2f; Ft.browA = 1; Ft.browY = -3; Ft.mouthW = 20;
      break;
    case EV_GAME:
      Ft.openL = Ft.openR = 0.75f; Ft.browA = 1; Ft.browTilt = 4; Ft.lookX = -0.5f; Ft.lookY = 0.7f;
      Ft.smile = 0.1f; Ft.mouthW = 16; Ft.shake = sinf(p * 0.03f) * 0.8f;
      break;
    case EV_GHOST: {
      float gx = 210.0f - q * 230.0f;
      Ft.lookX = constrain((gx - 120.0f) / 70.0f, -1.0f, 1.0f); Ft.lookY = -0.7f;
      Ft.browA = 1; Ft.browTilt = -5; Ft.browY = -4; Ft.smile = -0.5f; Ft.mouthOpen = 0.7f; Ft.mouthW = 16;
      Ft.shake = sinf(p * 0.06f) * 2.0f; Ft.blush = 0;
      break;
    }
    case EV_CAT:
      Ft.happy = 0.4f; Ft.smile = 1; Ft.blush = 1; Ft.mouthW = 24; Ft.lookX = 0; Ft.lookY = 0;
      Ft.jump = sinf(p * 0.008f) * 2.0f;
      break;
    case EV_THUNDER: {
      bool fl = p > 700 && fmodf(p, 1100.0f) < 280.0f;
      Ft.openL = Ft.openR = 1; Ft.smile = -0.3f; Ft.mouthOpen = 0.6f; Ft.mouthW = 16; Ft.lookX = 0; Ft.lookY = -0.6f;
      Ft.shake = fl ? sinf(p * 0.25f) * 3.0f : 0; Ft.blush = 0;
      break;
    }
    case EV_TRAFFIC:
      Ft.lookX = 0.8f; Ft.lookY = -0.2f;
      if (p < 1800) { Ft.smile = -0.1f; Ft.browA = 1; Ft.browTilt = 3; Ft.mouthW = 18; }
      else { Ft.smile = 1; Ft.mouthOpen = 0.4f; Ft.mouthW = 28; Ft.blush = 0.8f; Ft.happy = p > 2700 ? 0.5f : 0.0f;
             Ft.jump = -fabsf(sinf(p * 0.012f)) * 3.0f; }
      break;
    case EV_EAT:
      if (p < 1000) { Ft.lookX = 0.6f; Ft.lookY = 0.5f; Ft.smile = 0.3f; Ft.mouthW = 22; Ft.mouthOpen = p > 600 ? 0.5f : 0.0f; }
      else { Ft.happy = 0.8f; Ft.smile = 1; Ft.blush = 1; Ft.mouthW = 24; Ft.lookX = 0; Ft.lookY = 0;
             Ft.mouthOpen = fabsf(sinf(p * 0.014f)) * 0.6f; Ft.jump = -fabsf(sinf(p * 0.014f)) * 2.0f; }
      break;
    case EV_YAWN: {
      float s = sinf(3.14159f * q);
      Ft.openL = Ft.openR = 0.12f + 0.4f * (1.0f - q);
      Ft.happy = 0.7f * s; Ft.mouthOpen = s; Ft.mouthW = 20; Ft.smile = 0;
      Ft.browA = 1; Ft.browY = -6.0f * s; Ft.lookY = -0.3f; Ft.jump = -5.0f * s; Ft.blush = 0.2f;
      break;
    }
    default: break;
  }
}

// ---------- Info lagu (pemutar di HP lewat Chronos) ----------
char musicTxt[100] = "", musicTitle[64] = "", musicArtist[48] = "";
bool musicOn = false;
uint32_t musicElapsed = 0, musicTickAt = 0;                       // detik lagu berjalan (perkiraan, dihitung sejak lagu terdeteksi)

void pollMusic(bool connected) {                                  // dipanggil tiap loop; data dibaca tiap 500 ms
  static uint32_t last = 0;
  uint32_t now = millis();
  if (musicOn && musicTickAt) musicElapsed += now - musicTickAt;  // hitung waktu hanya saat diputar
  musicTickAt = now;
  if (now - last < 500) return;
  last = now;
  if (!connected) { musicOn = false; return; }
  MusicInfo &m = watch.getMusicInfo();
  char nt[64], na[48];
  asciiClean(m.title, nt, sizeof(nt));
  asciiClean(m.artist, na, sizeof(na));
  if (strcmp(nt, musicTitle) || strcmp(na, musicArtist)) {        // lagu berganti: waktu mulai dari 0
    strcpy(musicTitle, nt); strcpy(musicArtist, na);
    musicElapsed = 0;
  }
  snprintf(musicTxt, sizeof(musicTxt), "%s%s%s", nt, (nt[0] && na[0]) ? " - " : "", na);
  musicOn = (m.state != 0 && musicTxt[0] != 0);                   // state 0 = jeda/berhenti
}

// Jendela teks 21 huruf; teks yang lebih panjang bergeser satu huruf tiap 280 ms
int textWindow(const char *src, char *win, uint32_t now) {
  const int W = 21;
  int len = strlen(src);
  if (len <= W) { strcpy(win, src); return len; }
  int cyc = len + 3, st = (int)((now / 280) % (uint32_t)cyc);
  for (int i = 0; i < W; i++) { int k = (st + i) % cyc; win[i] = (k < len) ? src[k] : ' '; }
  win[W] = 0;
  return W;
}
int musicWindow(char *win, uint32_t now) { return textWindow(musicTxt, win, now); }

void drawMusicLine(uint32_t now) {                                // standby wajah: ikon nada hijau + teks kecil
  char win[24];
  int n = musicWindow(win, now);
  int x0 = 120 - (12 + n * 6) / 2;
  cCircle(x0 + 2, 189, 2, COL_GREEN);
  cLine(x0 + 4, 185, x0 + 4, 189, COL_GREEN);
  cLine(x0 + 4, 185, x0 + 7, 187, COL_GREEN);
  cText(win, x0 + 12, 185, 1, COL_DIM);
}

void renderFace(uint32_t now, int mood, bool connected, float blink) {
  useClip(1);                                                     // animasi tidak boleh keluar dari area wajah
  cvClear();
  drawGlow(now, mood, frameDt);
  drawBg(now, mood, frameDt);
  float p = (fxEv != EV_NONE) ? (float)(now - evStartMs) : 0.0f;
  float bob = (mood == MOOD_SLEEP) ? sinf(now / 1500.0f) * 3.0f : sinf(now / 1000.0f) * 2.0f;   // bernapas
  int dx = (int)lroundf(Fc.shake);
  int dy = (int)lroundf(Fc.jump + bob);
  uint16_t ec = (mood == MOOD_OFF) ? COL_RED : COL_GREEN;
  int ey = 112 + dy + (int)lroundf(Fc.lookY * 5.0f);
  int exo = (int)lroundf(Fc.lookX * 9.0f);
  int cxL = 80 + dx + exo, cxR = 160 + dx + exo;

  drawFxBack(p);
  bool starEyes = (curEv == EV_STAREYES && mood == MOOD_HAPPY);

  if (fxEv == EV_VROOM) {                                         // garis kecepatan
    for (int i = 0; i < 5; i++) {
      int x = 204 - (int)fmodf(p * 0.55f + i * 53.0f, 190.0f);
      if (x < CVX + 2) continue;
      cLine(x, 78 + i * 22, x + 14 + (i % 3) * 8, 78 + i * 22, fx(COL_DIM));
    }
  }

  float oL = Fc.openL * (1.0f - blink * 0.96f);
  float oR = Fc.openR * (1.0f - blink * 0.96f);
  if (mood == MOOD_SLEEP) {
    cArc(cxL, ey - 4, 17, 12, 20, 160, ec);
    cArc(cxR, ey - 4, 17, 12, 20, 160, ec);
  } else {
    if (Fc.love < 0.6f && Fc.spiral < 0.6f && !starEyes) {
      drawEye(cxL, ey, oL, Fc.happy, ec, Fc.lookX, Fc.lookY);
      drawEye(cxR, ey, oR, Fc.happy, ec, Fc.lookX, Fc.lookY);
    }
    if (starEyes) {                                                // mata bintang berdenyut
      float sc = constrain(p / 260.0f, 0.0f, 1.0f);
      int R = (int)(27.0f * sc * (1.0f + 0.1f * sinf(p * 0.012f)));
      if (R > 2) {
        cStar(cxL, ey, R, R * 45 / 100, p * 0.0015f, COL_ORANGE);
        cStar(cxR, ey, R, R * 45 / 100, p * 0.0015f, COL_ORANGE);
        cStar(cxL, ey, R / 2, R / 5, p * 0.0015f, COL_TEXT);
        cStar(cxR, ey, R / 2, R / 5, p * 0.0015f, COL_TEXT);
      }
    }
    if (Fc.love > 0.05f) {                                         // mata hati berdetak
      int s = (int)(14.0f * Fc.love * (1.0f + 0.15f * sinf(p * 0.018f)));
      if (s > 1) { cHeart(cxL, ey, s, COL_RED); cHeart(cxR, ey, s, COL_RED); }
    }
    if (Fc.spiral > 0.05f) {                                       // mata pusing berputar
      drawSpiral(cxL, ey, p * 0.012f, Fc.spiral, ec);
      drawSpiral(cxR, ey, -p * 0.012f, Fc.spiral, ec);
    }
  }

  if (fxEv == EV_COOL && mood == MOOD_HAPPY) {                    // kacamata jatuh lalu naik
    float dur = (float)EV_DUR[EV_COOL];
    float off = (p < dur - 450.0f) ? -50.0f * expf(-p / 110.0f) * cosf(p / 55.0f)
                                    : -((p - (dur - 450.0f)) / 450.0f) * 60.0f;
    if (off > -58.0f) {
      int gy = 112 + dy + (int)off;
      cRRect(cxL - 26, gy - 16, 52, 32, 9, COL_DIM);
      cRRect(cxR - 26, gy - 16, 52, 32, 9, COL_DIM);
      cRRect(cxL - 24, gy - 14, 48, 28, 8, COL_TIRE);
      cRRect(cxR - 24, gy - 14, 48, 28, 8, COL_TIRE);
      cRect(cxL + 26, gy - 6, (cxR - 26) - (cxL + 26), 5, COL_DIM);
      cLine(cxL - 16, gy - 8, cxL - 8, gy - 8, COL_TEXT);
      cLine(cxL - 16, gy - 7, cxL - 8, gy - 7, COL_TEXT);
      cLine(cxR - 16, gy - 8, cxR - 8, gy - 8, COL_TEXT);
      cLine(cxR - 16, gy - 7, cxR - 8, gy - 7, COL_TEXT);
    }
  }

  if (Fc.browA > 0.04f && mood != MOOD_SLEEP && Fc.love < 0.5f && fxEv != EV_COOL && fxEv != EV_HELMET && fxEv != EV_BATT && fxEv != EV_SIGNAL && fxEv != EV_FUEL && fxEv != EV_RAIN && fxEv != EV_THUNDER && fxEv != EV_CAT) {
    uint16_t bc = mix565(COL_BG, COL_TEXT, Fc.browA);
    int byL = ey - (int)(26.0f * constrain(Fc.openL, 0.5f, 1.3f)) - 13 + (int)Fc.browY;
    int byR = ey - (int)(26.0f * constrain(Fc.openR, 0.5f, 1.3f)) - 13 + (int)(Fc.browY + Fc.browR);
    drawBrow(cxL, byL, Fc.browTilt, true, bc);
    drawBrow(cxR, byR, Fc.browTilt, false, bc);
  }

  uint16_t cc = mix565(COL_BG, COL_CHEEK, Fc.blush);
  cCircle(52 + dx / 2, 142 + dy, 8, cc);
  cCircle(188 + dx / 2, 142 + dy, 8, cc);

  if (mood == MOOD_SLEEP) {
    int r = 3 + (int)(1.5f * (0.5f + 0.5f * sinf(now / 1500.0f)));
    cv->drawCircle(120 + dx - OX, 148 + dy - OY, r, COL_DIM);
    int br = 3 + (int)(8.0f * (0.5f + 0.5f * sinf(now / 700.0f)));   // gelembung ingus
    cv->drawCircle(131 - OX, 146 + dy - OY, br, COL_DIM);
    for (int i = 0; i < 3; i++) {                                    // Z melayang naik
      float q = ((now + i * 900) % 2700) / 2700.0f;
      uint16_t zc = (q > 0.72f) ? mix565(COL_ORANGE, COL_BG, (q - 0.72f) * 3.5f) : COL_ORANGE;
      cText("Z", 164 + (int)(q * 20.0f), 104 - (int)(q * 44.0f), 1 + (uint8_t)(q * 2.4f), zc);
    }
  } else {
    drawMouth(120 + dx, 142 + dy, Fc.smile, Fc.mouthOpen, Fc.mouthW, Fc.tongue);
  }

  if (fxEv == EV_LOVE) {                                          // hati melayang naik
    static const int hxs[5] = {46, 194, 120, 58, 182};
    for (int i = 0; i < 5; i++) {
      float q = fmodf(p + i * 520.0f, 2600.0f) / 2600.0f;
      int hx = hxs[i] + (int)(sinf(q * 6.28f + i) * 5.0f);
      uint16_t hc = fx((q > 0.8f) ? mix565(COL_RED, COL_BG, (q - 0.8f) * 5.0f) : COL_RED);
      cHeart(hx, 140 + dy - (int)(q * 76.0f), 3 + (i % 3), hc);
    }
  }
  if (fxEv == EV_MUSIC) {                                         // not balok melayang
    for (int i = 0; i < 3; i++) {
      float q = fmodf(p + i * 1100.0f, 3300.0f) / 3300.0f;
      int nx = ((i % 2) ? 188 : 48) + (int)(sinf(q * 9.4f) * 6.0f);
      uint16_t base = fx((i % 2) ? COL_ORANGE : COL_TEXT);
      drawNote(nx, 150 - (int)(q * 88.0f), (q > 0.75f) ? mix565(base, COL_BG, (q - 0.75f) * 4.0f) : base);
    }
  }
  if (fxEv == EV_SNEEZE && p > 1000 && p < 1750) {
    cTextCenter("Hatchii!", 62 + max(0, 10 - (int)((p - 1000.0f) / 20.0f)), 2, fx(COL_ORANGE));
  }
  if (fxEv == EV_VROOM && p > 200 && p < EV_DUR[EV_VROOM] - 300) {
    cTextCenter("VROOM", 64 + (int)(sinf(p * 0.05f) * 2.0f), 2, fx(COL_ORANGE));
  }

  drawFxNew(p, dx, dy, cxL, cxR, ey);
  drawFxNew2(p, dx, dy, cxL, cxR, ey);
  drawFxNew3(p, dx, dy, cxL, cxR, ey);
  drawFxNew4(p, dx, dy, cxL, cxR, ey);

  int cm = clockMinutes(connected);                               // jam kecil di bawah wajah
  if (cm >= 0) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", cm / 60, cm % 60);
    uint16_t tc = (fxEv == EV_PEEK) ? mix565(COL_DIM, COL_ORANGE, 0.5f + 0.5f * sinf(p * 0.012f)) : COL_DIM;
    cTextCenter(buf, 168, 2, tc);
  } else {
    cTextCenter("Menunggu HP", 168, 2, COL_DIM);
  }
  pollMusic(connected);
  if (musicOn) {
    drawMusicLine(now);                                           // lagu diputar: menggantikan baris voltase aki
  } else if (BATT_ENABLED) {                                             // voltase aki: hanya jika pin aki tersambung
    updateBatt();
    if (battInit && battV >= 5.0f && battV < 20.0f) {
      char bb[14];
      snprintf(bb, sizeof(bb), "AKI %.1fV", battV);
      uint16_t bc = battV >= 12.4f ? COL_GREEN : (battV >= 12.0f ? COL_ORANGE : COL_RED);
      cTextCenter(bb, 185, 1, bc);
    }
  }
  useClip(0);
}

int randomEv() {
  int ev = random(EV_HAPPY, EV_YAWN);
  if (ev == lastEv) ev = (ev == EV_EAT) ? EV_HAPPY : ev + 1;
  return ev;
}

// ---------- Standby mode 1: foto + jam transparan + cincin pelangi ----------
int clockMinutes(bool connected);

// ---------- Video latar (JPEG per frame di partisi spiffs, mulai offset VID_OFF) ----------
// Satu slot foto/video = SELURUH partisi data (lihat partitions.csv, 0x250000 = 2,3 MB).
#define VID_HDR1  4096             // format lama "MNVIDEO1": header 4 KB, maks 240 frame
#define VID_HDR2  8192             // format baru "MNVIDEO2": header 8 KB, maks VID_MAXF frame
#define VID_MAXF  900
#define JD_POOL 5120               // memori kerja dekoder JPEG ROM (min. ~3100 byte)
uint8_t *jdPool = nullptr;                 // dialokasi di awal boot (wallpaperLoad), saat RAM masih lega
const uint8_t *vidBase = nullptr;        // frame dipetakan langsung dari flash
uint16_t vidN = 0, vidFps = 15;
const uint32_t *vidTab = nullptr;        // tabel (offset, panjang) tiap frame, dibaca langsung dari header di flash
bool     vidOK = false;

#if TILT_UI
uint16_t rbwCol[256];
float    rbwC[1024], rbwS[1024];
float    rbwRot = 0;
bool     rbwInit = false;

void rbwSetup() {
  for (int i = 0; i < 256; i++) {
    float h = i / 42.6667f; int sx = (int)h; float f = h - sx;
    uint8_t q = (uint8_t)(255 * (1 - f)), t = (uint8_t)(255 * f), r = 0, g = 0, b = 0;
    switch (sx % 6) {
      case 0: r = 255; g = t; b = 0; break;
      case 1: r = q; g = 255; b = 0; break;
      case 2: r = 0; g = 255; b = t; break;
      case 3: r = 0; g = q; b = 255; break;
      case 4: r = t; g = 0; b = 255; break;
      default: r = 255; g = 0; b = q; break;
    }
    rbwCol[i] = fb->color565(r, g, b);
  }
  for (int i = 0; i < 1024; i++) { float a = i * 6.2831853f / 1024.0f; rbwC[i] = cosf(a); rbwS[i] = sinf(a); }
  rbwInit = true;
}

#if HAVE_ROM_JPEG
static const uint8_t *jdSrc = nullptr;
static uint32_t jdLen = 0, jdPos = 0;

static UINT jdIn(JDEC *jd, BYTE *buf, UINT n) {                    // baca data frame dari flash (buf null = lewati)
  (void)jd;
  if (jdPos + n > jdLen) n = jdLen - jdPos;
  if (buf) memcpy(buf, jdSrc + jdPos, n);
  jdPos += n;
  return n;
}

static UINT jdOut(JDEC *jd, void *bmp, JRECT *r) {                 // blok RGB888 hasil decode -> kanvas RGB565
  (void)jd;
  const uint8_t *p = (const uint8_t *)bmp;
  for (int y = r->top; y <= r->bottom; y++) {
    if (y >= 240) break;
    uint16_t *row = fb->row(y);
    for (int x = r->left; x <= r->right; x++, p += 3)
      if (x < 240) row[x] = ((p[0] & 0xF8) << 8) | ((p[1] & 0xFC) << 3) | (p[2] >> 3);
  }
  return 1;
}

bool vidDecode(int i) {
  if (!jdPool || !vidTab) return false;
  jdSrc = vidBase + vidTab[i * 2]; jdLen = vidTab[i * 2 + 1]; jdPos = 0;
  JDEC jd;
  JRESULT rc = jd_prepare(&jd, jdIn, jdPool, JD_POOL, nullptr);
  if (rc == JDR_OK && (jd.width != 240 || jd.height != 240)) rc = JDR_PAR;
  if (rc == JDR_OK) rc = jd_decomp(&jd, jdOut, 0);
  static bool warned = false;
  if (rc != JDR_OK && !warned) { warned = true; Serial.printf("Video: decode frame %d gagal (kode %d)\n", i, (int)rc); }
  return rc == JDR_OK;
}

bool mapDecode(const uint8_t *p, uint32_t len) {                   // JPEG 240x240 dari RAM -> kanvas
  if (!jdPool || !fb || !p || len < 100) return false;
  jdSrc = p; jdLen = len; jdPos = 0;
  JDEC jd;
  JRESULT rc = jd_prepare(&jd, jdIn, jdPool, JD_POOL, nullptr);
  if (rc == JDR_OK && (jd.width != 240 || jd.height != 240)) rc = JDR_PAR;
  if (rc == JDR_OK) rc = jd_decomp(&jd, jdOut, 0);
  return rc == JDR_OK;
}
#else
bool vidDecode(int) { return false; }
bool mapDecode(const uint8_t *, uint32_t) { return false; }
#endif

static inline uint16_t blend565(uint16_t bg, uint16_t fg, int a) {   // a: 0..256
  int rb = bg >> 11, gb = (bg >> 5) & 63, bb = bg & 31;
  int rf = fg >> 11, gf = (fg >> 5) & 63, bf = fg & 31;
  rb += ((rf - rb) * a) >> 8; gb += ((gf - gb) * a) >> 8; bb += ((bf - bb) * a) >> 8;
  return (uint16_t)((rb << 11) | (gb << 5) | bb);
}

// Visualizer simulasi (bukan dari suara asli): 64 batang pelangi menjorok ke dalam dari cincin, tembus pandang
// agar foto tetap terlihat. Simetris kiri-kanan, bass di bawah, denyut tiap ~0,5 dtk, memudar saat lagu jeda.
float vizLv[33];
float vizAmp = 0;

void drawVisualizer(uint32_t now, float dt) {
  vizAmp = ap(vizAmp, musicOn ? 1.0f : 0.0f, 350.0f, dt);
  if (vizAmp < 0.03f) { for (int f = 0; f < 33; f++) vizLv[f] = 0; return; }
  float t = now * 0.001f;
  float kick = expf(-fmodf(t * 1.9f, 1.0f) * 6.0f);
  int rot = (int)rbwRot;
  for (int f = 0; f < 33; f++) {
    float n = (0.5f + 0.5f * sinf(t * (2.3f + 0.37f * f) + f * 1.7f)) * (0.5f + 0.5f * sinf(t * (1.1f + 0.21f * f) + f * 0.6f));
    float tgt = 0.12f + 0.6f * n * (1.0f - f / 55.0f) + kick * (f < 10 ? 0.5f : 0.12f);
    if (tgt > 1.0f) tgt = 1.0f;
    if (tgt > vizLv[f]) vizLv[f] = ap(vizLv[f], tgt, 45.0f, dt);
    else vizLv[f] = max(0.0f, vizLv[f] - dt * 0.0017f);
  }
  for (int i = 0; i < 64; i++) {
    int j = (i <= 32) ? i : 64 - i;
    int len = (int)((3.0f + vizLv[32 - j] * 20.0f) * vizAmp);
    if (len < 2) continue;
    float a = i * 0.0981748f - 1.5708f;
    float ux = cosf(a), uy = sinf(a);
    uint16_t col = rbwCol[((i << 2) + rot) & 255];
    for (int rr = 107; rr > 107 - len; rr--) {
      float fade = 1.0f - (float)(107 - rr) / (len + 1);          // pangkal pekat, ujung memudar
      int aCore = (int)(125 * fade) + 25, aHalo = (int)(55 * fade) + 8;
      for (int tt = -3; tt <= 3; tt++) {
        int x = (int)(119.5f + ux * rr - uy * tt + 0.5f), y = (int)(119.5f + uy * rr + ux * tt + 0.5f);
        if ((unsigned)x >= 240 || (unsigned)y >= 240) continue;
        uint16_t *px = fb->row(y) + x;
        *px = blend565(*px, col, (abs(tt) <= 1) ? aCore : aHalo);
      }
    }
  }
}

// Kartu pemutar kecil di bawah jam: judul, artis, detik berjalan + garis gelombang mengalir (bayangan hitam agar terbaca)
void drawPhotoPlayer(uint32_t now) {
  if (!musicOn) return;
  char win[24];
  fb->setTextSize(1);
  int n = textWindow(musicTitle, win, now), tx = 120 - n * 3;
  fb->setTextColor(0x0000); fb->setCursor(tx + 1, 152); fb->print(win);
  fb->setTextColor(0xFFFF); fb->setCursor(tx, 151); fb->print(win);
  if (musicArtist[0]) {
    n = textWindow(musicArtist, win, now); tx = 120 - n * 3;
    fb->setTextColor(0x0000); fb->setCursor(tx + 1, 163); fb->print(win);
    fb->setTextColor(0xCE79); fb->setCursor(tx, 162); fb->print(win);
  }
  uint32_t sec = musicElapsed / 1000;
  char tb[8];
  snprintf(tb, sizeof(tb), "%u:%02u", (unsigned)(sec / 60), (unsigned)(sec % 60));
  int tw = strlen(tb) * 6, x0 = 120 - (tw + 6 + 76) / 2, wx = x0 + tw + 6;
  fb->setTextColor(0x0000); fb->setCursor(x0 + 1, 175); fb->print(tb);
  fb->setTextColor(0xFFFF); fb->setCursor(x0, 174); fb->print(tb);
  float ph = now * 0.006f;
  int px = wx, py = 0;
  for (int k = 0; k <= 76; k += 2) {
    int x = wx + k, y = 178 + (int)lroundf(sinf(k * 0.28f - ph) * 2.6f);
    if (k) {
      fb->drawLine(px, py + 1, x, y + 1, 0x0000);
      fb->drawLine(px, py, x, y, 0xFFFF);
      fb->drawLine(px, py - 1, x, y - 1, 0xFFFF);
    }
    px = x; py = y;
  }
}

void updatePhotoStandby(bool connected) {
  uint32_t now = millis();
  static uint32_t last = 0;
  static int lastVidIdx = -1;
  int vi = -1;
  if (vidOK) {                                                     // video: ganti frame sesuai fps, di antaranya tidak menggambar ulang
    vi = (int)(((uint64_t)now * vidFps / 1000) % vidN);
    if (vi == lastVidIdx) return;
  }
  if (now - last < FRAME_MS) return;
  float dt = (float)(now - last); if (dt > 100.0f) dt = 100.0f;
  last = now;
  lastVidIdx = vi;
  if (!rbwInit) rbwSetup();

  const uint16_t *csrc = nullptr;                            // piksel asli di area jam (untuk jam transparan)
  int cstride = 240;
  bool vidShown = (vi >= 0) && vidDecode(vi);                // video sebagai latar; gagal decode -> foto
  if (vidShown) {
    // video: latar di bawah huruf jam diambil dari piksel video terdekat di kirinya (tanpa penyangga RAM tambahan)
  } else {
    for (int i = 0; i < SplitCanvas::SN; i++)                // foto sebagai latar
      memcpy_P(fb->s[i], photoPtr + i * 240 * SplitCanvas::SH, 240 * SplitCanvas::SH * 2);
    csrc = photoPtr + 96 * 240 + 26;
  }

  // Jam: digambar dulu dengan warna kunci, lalu piksel kunci diganti putih transparan (campur dengan foto)
  int cm = clockMinutes(connected);
  char buf[8];
  if (cm >= 0) snprintf(buf, sizeof(buf), "%02d:%02d", cm / 60, cm % 60); else strcpy(buf, "--:--");
  const uint16_t KEY = 0x0001;
  fb->setTextSize(6);
  fb->setTextColor(KEY);
  fb->setCursor(30, 99); fb->print(buf);
  fb->setCursor(31, 99); fb->print(buf);                           // tebalkan huruf
  const int ALPHA = 200;                                           // 0-255: makin besar makin putih
  for (int y = 96; y < 146; y++) {
    uint16_t *row = fb->row(y);
    uint16_t nb = row[25];                                         // piksel asli di kiri area jam
    for (int x = 26; x < 214; x++) {
      if (row[x] != KEY) { nb = row[x]; continue; }
      uint16_t pc = csrc ? csrc[(y - 96) * cstride + (x - 26)] : nb;
      int r = pc >> 11, g = (pc >> 5) & 63, b = pc & 31;
      r += ((31 - r) * ALPHA) >> 8; g += ((63 - g) * ALPHA) >> 8; b += ((31 - b) * ALPHA) >> 8;
      row[x] = (r << 11) | (g << 5) | b;
    }
  }

  pollMusic(connected);
  drawVisualizer(now, dt);                                         // visualizer pelangi tembus pandang
  drawPhotoPlayer(now);                                            // judul, artis, detik, garis gelombang

  // Cincin pelangi di tepi, warna bergeser sehingga tampak berputar
  rbwRot += dt * 0.07f;
  if (rbwRot >= 256.0f) rbwRot -= 256.0f;
  int rot = (int)rbwRot;
  for (int i = 0; i < 1024; i++) {
    uint16_t c = rbwCol[((i >> 2) + rot) & 255];
    for (int r = 110; r <= 119; r++) {
      int x = (int)(119.5f + rbwC[i] * r + 0.5f), y = (int)(119.5f + rbwS[i] * r + 0.5f);
      fb->row(y)[x] = c;
    }
  }
  present();
}
#else
void updatePhotoStandby(bool connected) { (void)connected; }
bool mapDecode(const uint8_t *, uint32_t) { return false; }
#endif

void updateFace(bool connected) {
  uint32_t now = millis();
  if (now - lastFrame < FRAME_MS) return;
  float dt = (float)(now - lastFrame);
  if (dt > 100.0f) dt = 100.0f;
  lastFrame = now;
  frameDt = dt;

  if (connected != faceConnected) {
    faceConnected = connected;
    drawRing(1.0f, connected ? COL_GREEN : COL_RED);
    resetStandbyFx();
    curEv = EV_NONE;
  }

  uint32_t st = now - standbyStart;
  int mood = MOOD_HAPPY;
  if (!connected) mood = MOOD_OFF;
  else if (st > SLEEP_MS) mood = MOOD_SLEEP;

  updateOrbit(connected);
  updateStars(mood);
  if (!canvasOK) return;

  // Arah pandang acak (kadang kembali ke tengah)
  if (now >= nextGaze) {
    gzTx = random(-100, 101) / 100.0f;
    gzTy = random(-60, 61) / 100.0f;
    if (random(0, 3) == 0) { gzTx = 0; gzTy = 0; }
    nextGaze = now + random(1200, 3500);
  }

  // Penjadwal aksi acak
  if (mood != MOOD_HAPPY) {
    curEv = EV_NONE;
  } else {
    if (curEv != EV_NONE && now >= evEndMs) {
      if (!yawned && st < SLEEP_MS - 4500 && random(0, 100) < 30) startEv(randomEv(), now);   // langsung sambung lewat kedipan
      else { curEv = EV_NONE; forceBlink(now); }
    }
    if (!yawned && st > SLEEP_MS - 3200) {
      yawned = true;
      startEv(EV_YAWN, now);
    } else if (curEv == EV_NONE && !yawned && now >= nextEvMs) {
      startEv(randomEv(), now);
    }
  }

  float fT = (curEv != EV_NONE) ? 1.0f : 0.0f;   // fade efek aksi masuk/keluar
  fxFade = ap(fxFade, fT, fT > fxFade ? 180.0f : 280.0f, dt);
  if (curEv == EV_NONE && fxFade < 0.03f) fxEv = EV_NONE;

  float blink = (mood == MOOD_SLEEP) ? 0.0f : blinkAmount(now);
  faceTargets(now, mood, st);

  Fc.openL = ap(Fc.openL, Ft.openL, 55, dt);       Fc.openR = ap(Fc.openR, Ft.openR, 55, dt);
  Fc.happy = ap(Fc.happy, Ft.happy, 110, dt);      Fc.lookX = ap(Fc.lookX, Ft.lookX, 70, dt);
  Fc.lookY = ap(Fc.lookY, Ft.lookY, 70, dt);       Fc.browY = ap(Fc.browY, Ft.browY, 80, dt);
  Fc.browR = ap(Fc.browR, Ft.browR, 80, dt);       Fc.browTilt = ap(Fc.browTilt, Ft.browTilt, 90, dt);
  Fc.browA = ap(Fc.browA, Ft.browA, 120, dt);      Fc.smile = ap(Fc.smile, Ft.smile, 100, dt);
  Fc.mouthOpen = ap(Fc.mouthOpen, Ft.mouthOpen, 70, dt);
  Fc.mouthW = ap(Fc.mouthW, Ft.mouthW, 90, dt);    Fc.tongue = ap(Fc.tongue, Ft.tongue, 90, dt);
  Fc.blush = ap(Fc.blush, Ft.blush, 250, dt);      Fc.love = ap(Fc.love, Ft.love, 90, dt);
  Fc.spiral = ap(Fc.spiral, Ft.spiral, 110, dt);   Fc.jump = ap(Fc.jump, Ft.jump, 55, dt);
  Fc.shake = ap(Fc.shake, Ft.shake, 35, dt);

  renderFace(now, mood, connected, blink);
  present();
  if (SHOW_FPS) {
    static uint32_t fc = 0, ft = 0;
    fc++;
    if (now - ft >= 2000) { Serial.printf("FPS standby: %.1f\n", fc * 1000.0f / (now - ft)); fc = 0; ft = now; }
  }
}

// Palet warna dibangun lewat fungsi agar bisa diredupkan (fade-in halaman)
void buildPalette(float k) {
#define PC(r, g, b) (lightTheme ? gfx->color565((uint8_t)((r) * k + 255 * (1 - k)), (uint8_t)((g) * k + 255 * (1 - k)), (uint8_t)((b) * k + 255 * (1 - k))) \
                                : gfx->color565((uint8_t)((r) * k), (uint8_t)((g) * k), (uint8_t)((b) * k)))
  COL_BG          = PC(0, 0, 0);
  COL_MAPBG       = PC(6, 6, 8);
  COL_ROAD        = PC(44, 44, 48);
  COL_EDGE        = PC(255, 255, 255);
  COL_HORIZON     = PC(110, 110, 118);
  COL_RING_BG     = PC(62, 62, 68);
  COL_BLD         = PC(36, 36, 40);
  COL_WIN1        = PC(92, 92, 100);
  COL_WIN2        = PC(145, 145, 155);
  COL_CHIP        = PC(58, 58, 64);
  COL_TEXT        = PC(255, 255, 255);
  COL_DIM         = PC(205, 205, 212);
  COL_GREEN       = PC(40, 240, 100);
  COL_ORANGE      = PC(255, 170, 40);
  COL_RED         = PC(255, 60, 52);
  COL_CHEEK       = PC(170, 92, 26);
  COL_GLOW_GREEN  = PC(14, 98, 46);
  COL_GLOW_ORANGE = PC(118, 70, 14);
  COL_GLOW_RED    = PC(112, 26, 22);
  COL_TIRE        = PC(14, 14, 16);
  COL_JACKET      = PC(105, 108, 118);
  COL_HELMET      = PC(255, 255, 255);
  COL_VISOR       = PC(12, 12, 14);
  COL_TAIL        = PC(255, 60, 50);
  COL_MOUTH       = PC(100, 24, 30);
  COL_TONGUE      = PC(230, 100, 120);
  COL_TEAR        = PC(80, 170, 255);
  if (lightTheme) {                                                // tema terang: latar putih keabuan, teks gelap
    COL_BG = PC(244, 244, 247);   COL_MAPBG = PC(226, 228, 232);   COL_ROAD = PC(188, 190, 197);
    COL_EDGE = PC(255, 255, 255); COL_HORIZON = PC(150, 152, 160); COL_RING_BG = PC(208, 210, 216);
    COL_BLD = PC(204, 206, 212);  COL_WIN1 = PC(168, 170, 178);    COL_WIN2 = PC(128, 130, 138);
    COL_CHIP = PC(214, 216, 222); COL_TEXT = PC(20, 22, 28);       COL_DIM = PC(92, 96, 106);
    COL_GREEN = PC(18, 150, 64);  COL_ORANGE = PC(225, 110, 0);    COL_RED = PC(205, 30, 30);
    COL_CHEEK = PC(255, 170, 150);
    COL_GLOW_GREEN = PC(140, 210, 160); COL_GLOW_ORANGE = PC(245, 190, 130); COL_GLOW_RED = PC(240, 150, 150);
    COL_TIRE = PC(40, 40, 46);    COL_JACKET = PC(90, 95, 110);    COL_HELMET = PC(35, 35, 42);
    COL_VISOR = PC(220, 225, 235); COL_TAIL = PC(235, 50, 40);
    COL_MOUTH = PC(150, 40, 50);  COL_TONGUE = PC(235, 110, 130);  COL_TEAR = PC(60, 140, 235);
  }
#undef PC
}

bool pendingFade = false;

// ---------- Buzzer ----------
uint8_t  buzzLeft = 0;
uint16_t buzzOnMs = 80, buzzOffMs = 80;
uint32_t buzzNext = 0;
bool     buzzState = false;
long     lastCallBeep = -1;

void buzz(uint8_t n, uint16_t on = 80, uint16_t off = 80) {
  if (!BUZZER_ON) return;
  buzzLeft = n; buzzOnMs = on; buzzOffMs = off; buzzState = false; buzzNext = millis();
}

void buzzStop() {
  buzzLeft = 0; buzzState = false;
  digitalWrite(PIN_BUZZ, LOW);
}

void updateBuzzer() {
  if (!BUZZER_ON) return;
  uint32_t now = millis();
  if (buzzState) {
    if (now >= buzzNext) { digitalWrite(PIN_BUZZ, LOW); buzzState = false; buzzNext = now + buzzOffMs; }
  } else if (buzzLeft && now >= buzzNext) {
    digitalWrite(PIN_BUZZ, HIGH); buzzState = true; buzzLeft--; buzzNext = now + buzzOnMs;
  }
}

// ---------- Sudut miring (MPU6050) ----------
bool     mpuOK = false, tiltInit = false;
float    tiltRoll = 0, tiltOffset = 0, tiltMaxL = 0, tiltMaxR = 0;
uint32_t lastTiltUs = 0, lastTiltFrame = 0;
Preferences prefs;

void mpuInit() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0x00);                              // bangunkan sensor
  mpuOK = (Wire.endTransmission() == 0);
  if (mpuOK) {
    Wire.beginTransmission(MPU_ADDR); Wire.write(0x1B); Wire.write(0x08); Wire.endTransmission();  // gyro +/-500 dps
    Wire.beginTransmission(MPU_ADDR); Wire.write(0x1C); Wire.write(0x08); Wire.endTransmission();  // accel +/-4 g
    Wire.beginTransmission(MPU_ADDR); Wire.write(0x1A); Wire.write(0x03); Wire.endTransmission();  // filter 44 Hz
  }
  prefs.begin("nav", true);
  tiltOffset = prefs.getFloat("toff", 0.0f);
  prefs.end();
}

bool mpuRead(float &ax, float &ay, float &az, float &gx, float &gy) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)14) != 14) return false;
  int16_t r[7];
  for (int i = 0; i < 7; i++) { r[i] = (int16_t)(Wire.read() << 8); r[i] |= Wire.read(); }
  ax = r[0] / 8192.0f; ay = r[1] / 8192.0f; az = r[2] / 8192.0f;
  gx = r[4] / 65.5f;   gy = r[5] / 65.5f;
  return true;
}

void updateTilt() {
  float ax, ay, az, gx, gy;
  if (!mpuOK || !mpuRead(ax, ay, az, gx, gy)) return;
  uint32_t nowUs = micros();
  float dt = (nowUs - lastTiltUs) / 1e6f;
  bool fresh = !tiltInit || dt > 0.5f;
  lastTiltUs = nowUs;
#if TILT_AXIS == 0
  float acc = atan2f(ay, az) * 57.29578f;
  float rate = gx;
#else
  float acc = atan2f(-ax, sqrtf(ay * ay + az * az)) * 57.29578f;
  float rate = gy;
#endif
  if (fresh) { tiltRoll = acc; tiltInit = true; return; }
  tiltRoll = TILT_ALPHA * (tiltRoll + rate * dt) + (1.0f - TILT_ALPHA) * acc;
  float a = (tiltRoll - tiltOffset) * TILT_SIGN;
  if (a > tiltMaxR) tiltMaxR = a;
  if (-a > tiltMaxL) tiltMaxL = -a;
}

float tiltAngle() { return (tiltRoll - tiltOffset) * TILT_SIGN; }

void tiltZero() {
  tiltOffset = tiltRoll;
  tiltMaxL = tiltMaxR = 0;
  prefs.begin("nav", false);
  prefs.putFloat("toff", tiltOffset);
  prefs.end();
}

// ---------- Tampilan miring (seluruh layar berputar mengikuti sudut motor) ----------
float    uiAngle = 0;                 // sudut putar tampilan (derajat, searah jarum jam positif), dihaluskan
float    presentedAngle = 999;
bool     fbDirty = true;

#if TILT_UI
uint16_t stripBuf[240 * 8];

void present() {
  if (!canvasOK) return;
  presentedAngle = uiAngle;
  fbDirty = false;
  if (fabsf(uiAngle) < 0.4f) {                                      // tegak: salin langsung
    for (int i = 0; i < SplitCanvas::SN; i++)
      panel->draw16bitRGBBitmap(0, i * SplitCanvas::SH, fb->s[i], 240, SplitCanvas::SH);
    return;
  }
  float a = uiAngle * 0.0174533f;
  float cf = cosf(a) * 65536.0f, sf = sinf(a) * 65536.0f;
  int32_t cI = (int32_t)cf, sI = (int32_t)sf;
  for (int y0 = 0; y0 < 240; y0 += 8) {                              // putar per 8 baris lalu kirim ke layar
    for (int r = 0; r < 8; r++) {
      float dy = (y0 + r) - 119.5f;
      int32_t sx = (int32_t)(7864320.0f + (-119.5f * cf + dy * sf));  // 7864320 = 120 * 65536
      int32_t sy = (int32_t)(7864320.0f + (119.5f * sf + dy * cf));
      uint16_t *o = stripBuf + r * 240;
      for (int xd = 0; xd < 240; xd++) {
        uint32_t xi = (uint32_t)(sx >> 16), yi = (uint32_t)(sy >> 16);
        o[xd] = (xi < 240 && yi < 240) ? fb->row(yi)[xi] : 0;
        sx += cI; sy -= sI;
      }
    }
    panel->draw16bitRGBBitmap(0, y0, stripBuf, 240, 8);
  }
}

void syncFB() { present(); }

void tiltRefresh() {                                                // halaman statis: gambar ulang jika sudut berubah
  if (canvasOK && (fbDirty || fabsf(uiAngle - presentedAngle) > 0.35f)) present();
}

void tiltTick() {                                                   // baca MPU6050 dan haluskan sudut tampilan
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last < 10) return;
  float dt = min(100.0f, (float)(now - last));
  last = now;
  updateTilt();
  float target = mpuOK ? constrain(tiltAngle() * TILT_UI_SIGN, -TILT_MAX_DEG, TILT_MAX_DEG) : 0.0f;
  uiAngle = ap(uiAngle, target, 70.0f, dt);
}
#else
void present() { if (canvasOK) cv->flush(); }
void syncFB() {}
void tiltRefresh() {}
void tiltTick() {}
#endif

// ---------- Game (cukup 1 sensor sentuh): 3 game, masing-masing satu halaman ----------
// 6 MOTOR RUN (lompati kerucut) | 7 TERBANG (lewati pipa) | 8 PAS! (hentikan penanda di zona hijau)
// Sentuh = aksi permainan. Di layar judul / game over / jeda: sentuh = mulai, TAHAN 1 detik = keluar ke halaman berikutnya.
#define GAME_SOUND true         // buzzer: aksi, tabrak, skor
#define G_N        3
#define G_GY       160          // tanah MOTOR RUN
#define G_PX       78           // posisi x motor / burung
#define G_MAXOB    4
#define G_CEIL     72           // TERBANG: langit-langit
#define G_FLOOR    172          // TERBANG: lantai
#define G_GAP      52           // TERBANG: lebar celah pipa
#define G_TX       56           // PAS!: x awal lintasan
#define G_TRACK    128          // PAS!: panjang lintasan

enum GameState { GS_READY, GS_PLAY, GS_OVER, GS_PAUSE };
struct Obst { float x; uint8_t w, h; bool on, pass; };

// gH/gV/gSpeed/gDist/gNextSpawn dipakai bergantian menurut game:
//  MOTOR RUN: gH tinggi lompat, gV kecepatan naik, gDist jarak tempuh, gNextSpawn jarak ke kerucut berikut
//  TERBANG  : gH y burung, gV kecepatan (positif = turun), gNextSpawn jarak ke pipa berikut
//  PAS!     : gH posisi penanda, gV arah (+1/-1), gDist pusat zona, gNextSpawn setengah lebar zona
uint8_t  gState = GS_READY;
Obst     gOb[G_MAXOB];
float    gH = 0, gV = 0, gSpeed = 0.15f, gScore = 0, gNextSpawn = 0, gDist = 0, gPlayMs = 0;
bool     gAir = false, gNewBest = false, gBestLoaded = false;
uint16_t gBest[G_N] = {0, 0, 0};
uint32_t gLastUs = 0, gLastFrame = 0, gOverAt = 0;
int      gRingFor = -1, gLastHundred = 0, gActive = -1;

inline int gid() { return page - PAGE_GAME; }
const char *G_TITLE[G_N] = {"MOTOR RUN", "TERBANG", "PAS!"};
const char *G_TIP[G_N]   = {"SENTUH = LOMPAT", "SENTUH = KEPAK", "SENTUH DI ZONA HIJAU"};
const char *G_OVER[G_N]  = {"TABRAK!", "JATUH!", "MELESET!"};

void gameLoadBest() {
  prefs.begin("nav", true);
  for (int i = 0; i < G_N; i++) { char k[8]; snprintf(k, sizeof(k), "gbest%d", i); gBest[i] = prefs.getUShort(k, 0); }
  prefs.end();
}

void gameSaveBest() {
  char k[8]; snprintf(k, sizeof(k), "gbest%d", gid());
  prefs.begin("nav", false);
  prefs.putUShort(k, gBest[gid()]);
  prefs.end();
}

void gameNewZone() { gDist = random((long)gNextSpawn + 4, G_TRACK - (long)gNextSpawn - 3); }

void gameReset() {
  for (int i = 0; i < G_MAXOB; i++) gOb[i].on = false;
  gH = 0; gV = 0; gAir = false;
  gScore = 0; gDist = 0; gPlayMs = 0; gNewBest = false; gLastHundred = 0;
  int id = gid();
  if (id == 0)      { gSpeed = 0.15f; gNextSpawn = 130; }
  else if (id == 1) { gSpeed = 0.12f; gH = 122; gV = -0.15f; gNextSpawn = 60; }
  else              { gSpeed = 0.12f; gH = 0; gV = 1; gNextSpawn = 22; gameNewZone(); }
}

void gameCrash() {
  gState = GS_OVER;
  gOverAt = millis();
  int sc = (int)gScore, id = gid();
  gNewBest = (sc > (int)gBest[id]);
  if (gNewBest) { gBest[id] = sc; gameSaveBest(); }
  if (GAME_SOUND) buzz(3, 90, 60);
}

void gamePress() {                                                  // sentuhan mulai saat bermain
  if (gState != GS_PLAY) return;
  int id = gid();
  if (id == 0) {
    if (!gAir) { gV = 0.345f; gAir = true; if (GAME_SOUND) buzz(1, 20, 0); }
  } else if (id == 1) {
    gV = -0.30f;
    if (GAME_SOUND) buzz(1, 15, 0);
  } else {
    float d = fabsf(gH - gDist);
    if (d > gNextSpawn) { gameCrash(); return; }
    gScore += (d < 3.0f) ? 2 : 1;                                   // tepat di tengah = 2 poin
    if (GAME_SOUND) buzz(d < 3.0f ? 2 : 1, 30, 30);
    gNextSpawn = max(6.0f, gNextSpawn - 2.0f);
    gSpeed = min(0.40f, gSpeed + 0.012f);
    gameNewZone();
  }
}

void gameRelease() {                                                // MOTOR RUN: lepas cepat = lompat lebih pendek
  if (gid() == 0 && gAir && gV > 0.14f) gV = 0.14f;
}

void gameTapStart() {                                               // sentuhan singkat di layar judul / game over / jeda
  if (gState == GS_OVER && millis() - gOverAt < 600) return;        // cegah restart tak sengaja
  if (gState == GS_PAUSE) { gState = GS_PLAY; return; }
  gameReset();
  gState = GS_PLAY;
}

void gameSpawnRun() {
  for (int i = 0; i < G_MAXOB; i++) {
    if (gOb[i].on) continue;
    int r = random(100);
    gOb[i].on = true;
    gOb[i].x = CVX + CVW + 4;
    if (r < 35)            { gOb[i].w = 12; gOb[i].h = 14; }        // kerucut kecil
    else if (r < 65)       { gOb[i].w = 14; gOb[i].h = 22; }        // sedang
    else if (r < 85)       { gOb[i].w = 14; gOb[i].h = 30; }        // tinggi (perlu tahan)
    else if (gScore >= 30) { gOb[i].w = 28; gOb[i].h = 18; }        // dua kerucut berdempetan
    else                   { gOb[i].w = 12; gOb[i].h = 14; }
    return;
  }
}

void gameUpdateRun(float dt) {
  gPlayMs += dt;
  gSpeed = 0.15f + min(0.17f, gPlayMs * 0.000008f);                 // makin lama makin cepat
  float dx = gSpeed * dt;
  gDist += dx;
  gScore += dx / 12.0f;
  int hundred = (int)gScore / 100;
  if (hundred > gLastHundred) { gLastHundred = hundred; if (GAME_SOUND) buzz(2, 35, 35); }
  if (gAir) {
    gH += gV * dt;
    gV -= 0.0013f * dt;
    if (gH <= 0) { gH = 0; gV = 0; gAir = false; }
  }
  for (int i = 0; i < G_MAXOB; i++) {
    if (!gOb[i].on) continue;
    gOb[i].x -= dx;
    if (gOb[i].x + gOb[i].w < CVX - 2) gOb[i].on = false;
  }
  gNextSpawn -= dx;
  if (gNextSpawn <= 0) {
    gameSpawnRun();
    gNextSpawn = gSpeed * 600.0f + 50.0f + random(0, 120);
  }
  float px0 = G_PX - 9, px1 = G_PX + 9;                             // tabrakan: kotak motor vs kerucut
  for (int i = 0; i < G_MAXOB; i++) {
    if (!gOb[i].on) continue;
    float ox0 = gOb[i].x + 3, ox1 = gOb[i].x + gOb[i].w - 3;
    if (px1 > ox0 && px0 < ox1 && gH < gOb[i].h - 3) { gameCrash(); return; }
  }
}

void gameUpdateFlap(float dt) {
  gPlayMs += dt;
  gSpeed = 0.12f + min(0.08f, gPlayMs * 0.000004f);
  float dx = gSpeed * dt;
  gV = min(0.5f, gV + 0.0011f * dt);
  gH += gV * dt;
  gNextSpawn -= dx;
  if (gNextSpawn <= 0) {
    for (int i = 0; i < G_MAXOB; i++) {
      if (gOb[i].on) continue;
      gOb[i].on = true; gOb[i].pass = false; gOb[i].w = 22;
      gOb[i].x = CVX + CVW + 4;
      gOb[i].h = random(G_CEIL + 10, G_FLOOR - G_GAP - 6);         // y tepi atas celah
      break;
    }
    gNextSpawn = 118 + random(0, 30);
  }
  if (gH - 6 < G_CEIL || gH + 6 > G_FLOOR) { gameCrash(); return; }
  for (int i = 0; i < G_MAXOB; i++) {
    if (!gOb[i].on) continue;
    gOb[i].x -= dx;
    if (gOb[i].x + gOb[i].w < CVX - 4) { gOb[i].on = false; continue; }
    if (!gOb[i].pass && gOb[i].x + gOb[i].w < G_PX - 6) {
      gOb[i].pass = true; gScore += 1;
      if (GAME_SOUND) buzz(1, 25, 0);
    }
    if (G_PX + 5 > gOb[i].x + 1 && G_PX - 5 < gOb[i].x + gOb[i].w - 1 &&
        (gH - 5 < gOb[i].h || gH + 5 > gOb[i].h + G_GAP)) { gameCrash(); return; }
  }
}

void gameUpdateTiming(float dt) {
  gH += gV * gSpeed * dt;
  if (gH >= G_TRACK) { gH = G_TRACK; gV = -1; }
  if (gH <= 0)       { gH = 0;       gV = 1; }
}

void gHLine(int x0, int x1, int y, uint16_t c) {                    // garis datar yang dipotong ke area kanvas
  if (y < CVY || y >= CVY + CVH) return;
  if (x0 < CVX) x0 = CVX;
  if (x1 > CVX + CVW - 1) x1 = CVX + CVW - 1;
  if (x1 < x0) return;
  cv->drawFastHLine(x0 - OX, y - OY, x1 - x0 + 1, c);
}

void gRect(int x, int y, int w, int h, uint16_t c) {                // kotak yang dipotong ke area kanvas
  int x1 = x + w - 1, y1 = y + h - 1;
  if (x < CVX) x = CVX;
  if (y < CVY) y = CVY;
  if (x1 > CVX + CVW - 1) x1 = CVX + CVW - 1;
  if (y1 > CVY + CVH - 1) y1 = CVY + CVH - 1;
  if (x1 < x || y1 < y) return;
  cv->fillRect(x - OX, y - OY, x1 - x + 1, y1 - y + 1, c);
}

void gCone(float fx, int w, int h) {
  int x = (int)fx;
  for (int r = 0; r < h; r++) {
    int span = max(2, w * (r + 1) / h);
    int xs = x + (w - span) / 2;
    gHLine(xs, xs + span - 1, G_GY - h + r, ((r / 5) % 2) ? COL_TEXT : COL_ORANGE);
  }
}

void drawRunScene() {
  static const uint8_t SY[5] = {96, 108, 100, 118, 92};             // bintang kecil bergeser pelan
  for (int i = 0; i < 5; i++) {
    int sx = CVX + CVW - (int)fmodf(gDist * 0.15f + i * 47.0f, (float)CVW);
    cRect(sx, SY[i], 2, 2, COL_DIM);
  }
  gHLine(CVX, CVX + CVW - 1, G_GY, COL_TEXT);                       // tanah
  for (float x = -fmodf(gDist, 24.0f); x < CVW; x += 24.0f) {
    int x0 = CVX + (int)x;
    gHLine(x0, x0 + 9, G_GY + 7, COL_DIM);
  }
  for (int i = 0; i < G_MAXOB; i++) {
    if (!gOb[i].on) continue;
    if (gOb[i].w > 20) { gCone(gOb[i].x, 14, gOb[i].h); gCone(gOb[i].x + 14, 14, gOb[i].h); }
    else               gCone(gOb[i].x, gOb[i].w, gOb[i].h);
  }
  int wy = G_GY - (int)gH - 7;                                      // pusat roda
  int xr = G_PX - 12, xf = G_PX + 12;
  float a = gDist * 0.12f;
  cv->drawCircle(xr - OX, wy - OY, 7, COL_TEXT);
  cv->drawCircle(xf - OX, wy - OY, 7, COL_TEXT);
  cLine(xr, wy, xr + (int)(cosf(a) * 6), wy + (int)(sinf(a) * 6), COL_DIM);
  cLine(xf, wy, xf + (int)(cosf(a) * 6), wy + (int)(sinf(a) * 6), COL_DIM);
  cRRect(G_PX - 12, wy - 8, 26, 5, 2, COL_TEXT);                    // rangka
  cRRect(G_PX - 8, wy - 14, 12, 6, 2, COL_RED);                     // tangki
  cLine(G_PX + 10, wy - 6, G_PX + 12, wy - 17, COL_TEXT);           // stang
  cRRect(G_PX - 7, wy - 24, 10, 14, 3, COL_ORANGE);                 // badan pengendara
  cCircle(G_PX - 1, wy - 30, 6, COL_HELMET);                        // helm
  cRect(G_PX + 1, wy - 32, 6, 3, COL_VISOR);                        // kaca helm
}

void drawFlapScene() {
  uint16_t cap = mix565(COL_GREEN, COL_TEXT, 0.4f);
  gHLine(CVX, CVX + CVW - 1, G_FLOOR, COL_TEXT);
  if (gState == GS_PLAY) gHLine(CVX, CVX + CVW - 1, G_CEIL, COL_RING_BG);
  for (int i = 0; i < G_MAXOB; i++) {
    if (!gOb[i].on) continue;
    int x = (int)gOb[i].x, w = gOb[i].w, top = gOb[i].h, bot = top + G_GAP;
    gRect(x, G_CEIL + 1, w, top - G_CEIL - 1, COL_GREEN);
    gRect(x - 2, top - 6, w + 4, 6, cap);
    gRect(x, bot, w, G_FLOOR - bot, COL_GREEN);
    gRect(x - 2, bot, w + 4, 6, cap);
  }
  int by = (int)constrain(gH, (float)G_CEIL, (float)G_FLOOR);
  cCircle(G_PX, by, 7, COL_ORANGE);
  cRRect(G_PX - 7, by + (gV < 0 ? -2 : 2), 8, 4, 2, COL_RED);       // sayap naik/turun
  cCircle(G_PX + 3, by - 2, 3, COL_TEXT);
  cCircle(G_PX + 4, by - 2, 1, COL_VISOR);
  cTri(G_PX + 6, by, G_PX + 13, by + 2, G_PX + 6, by + 4, COL_RED); // paruh
}

void drawTimingScene() {
  cRRect(G_TX, 120, G_TRACK, 14, 4, COL_RING_BG);
  cRect(G_TX + (int)(gDist - gNextSpawn), 120, (int)(gNextSpawn * 2), 14, COL_GREEN);
  cRect(G_TX + (int)gDist - 1, 122, 2, 10, COL_BG);                 // tanda tengah zona (tepat = 2 poin)
  if (gState != GS_READY) {
    int mx = G_TX + (int)gH;
    cRect(mx - 1, 112, 3, 30, COL_TEXT);                            // penanda
    cTri(mx - 4, 106, mx + 4, 106, mx, 111, COL_ORANGE);
  }
}

void drawGame() {
  useClip(1);
  cvClear();
  int id = gid();
  if (id == 0) drawRunScene(); else if (id == 1) drawFlapScene(); else drawTimingScene();

  char buf[28];
  bool blink = (millis() / 400) & 1;
  if (gState == GS_READY) {
    cTextCenter(G_TITLE[id], 64, 2, COL_ORANGE);
    cTextCenter(G_TIP[id], 84, 1, COL_TEXT);
    cTextCenter("TAHAN = MULAI MAIN", 96, 1, COL_TEXT);
    cTextCenter("KLIK SEKALI = MENU LAIN", 106, 1, COL_DIM);
    snprintf(buf, sizeof(buf), "TERBAIK %d", (int)gBest[id]);
    cTextCenter(buf, 180, 1, COL_DIM);
  } else if (gState == GS_PLAY) {
    snprintf(buf, sizeof(buf), "%d", (int)gScore);
    if (id == 2) cTextCenter(buf, 66, 4, COL_TEXT); else cTextCenter(buf, 58, 2, COL_TEXT);
    snprintf(buf, sizeof(buf), "TERBAIK %d", (int)gBest[id]);
    cTextCenter(buf, 180, 1, COL_DIM);
  } else if (gState == GS_OVER) {
    cTextCenter(G_OVER[id], 62, 2, COL_RED);
    snprintf(buf, sizeof(buf), "SKOR %d", (int)gScore);
    cTextCenter(buf, 84, 2, COL_TEXT);
    if (gNewBest) { if (blink) cTextCenter("REKOR BARU!", 102, 1, COL_ORANGE); }
    else { snprintf(buf, sizeof(buf), "TERBAIK %d", (int)gBest[id]); cTextCenter(buf, 102, 1, COL_DIM); }
    cTextCenter("SENTUH:ULANG TAHAN:KELUAR", 180, 1, COL_DIM);
  } else {                                                          // GS_PAUSE
    cTextCenter("JEDA", 70, 3, COL_ORANGE);
    cTextCenter("SENTUH UNTUK LANJUT", 98, 1, COL_TEXT);
    cTextCenter("TAHAN 1 DTK = KELUAR", 110, 1, COL_DIM);
  }
  useClip(0);
  present();
}

void updateGamePage(bool connected) {
  (void)connected;
  uint32_t nowUs = micros();
  String gs = String("game") + page;
  if (lastSig != gs) {                                              // masuk halaman game / gambar ulang setelah notifikasi
    lastSig = gs;
    if (!gBestLoaded) { gBestLoaded = true; gameLoadBest(); }
    bool resume = (gState == GS_PLAY || gState == GS_PAUSE) && gActive == gid();
    gState = resume ? GS_PAUSE : GS_READY;
    gActive = gid();
    if (!resume) gameReset();
    gfx->fillScreen(COL_BG);
    gRingFor = -1;
    gLastUs = nowUs;
  }
  if (millis() - gLastFrame < FRAME_MS || !canvasOK) return;
  gLastFrame = millis();

  float dt = (nowUs - gLastUs) / 1000.0f;
  gLastUs = nowUs;
  if (dt > 400.0f) { if (gState == GS_PLAY) gState = GS_PAUSE; dt = 0; }   // celah panjang (mis. notifikasi): jeda
  dt = min(dt, 40.0f);
  if (gState == GS_PLAY) {
    int id = gid();
    if (id == 0) gameUpdateRun(dt); else if (id == 1) gameUpdateFlap(dt); else gameUpdateTiming(dt);
  }

  int want = (gState == GS_OVER) ? 2 : (gState == GS_PLAY ? 0 : 1);
  if (want != gRingFor) {                                           // cincin: hijau main, oranye siap/jeda, merah game over
    gRingFor = want;
    drawRing(1.0f, want == 2 ? COL_RED : (want == 0 ? COL_GREEN : COL_ORANGE));
    drawPageDots();
  }
  drawGame();
}

// ---------- Animasi notifikasi ----------
// Dipanggil dari library Chronos saat ada notifikasi baru di HP
// Salin teks sebagai ASCII murni: emoji/karakter non-ASCII dibuang, baris baru jadi spasi, spasi ganda dirapatkan
void asciiClean(const String &in, char *out, size_t cap) {
  size_t n = 0;
  bool sp = true;
  for (size_t i = 0; i < in.length() && n + 1 < cap; i++) {
    uint8_t c = (uint8_t)in[i];
    if (c >= 0x80) continue;
    if (c < 32) c = ' ';
    if (c == ' ') { if (sp) continue; sp = true; } else sp = false;
    out[n++] = (char)c;
  }
  while (n && out[n - 1] == ' ') n--;
  out[n] = 0;
}

// Pecah teks jadi maks 3 baris (21 huruf per baris) pada spasi; sisa yang tidak muat diakhiri "~"
int wrapMsg(const char *src, char lines[3][24]) {
  const int W = 21;
  int len = strlen(src), pos = 0, nl = 0;
  while (pos < len && nl < 3) {
    while (src[pos] == ' ') pos++;
    int rem = len - pos;
    if (rem <= 0) break;
    int take = rem;
    if (rem > W) {
      int k = W;
      while (k >= 8 && src[pos + k] != ' ') k--;
      take = (k >= 8) ? k : W;
    }
    bool last = (nl == 2), more = (pos + take < len);
    int cp = (last && more) ? ((take < W - 1) ? take : W - 1) : take;
    memcpy(lines[nl], src + pos, cp);
    lines[nl][cp] = 0;
    if (last && more) { lines[nl][cp] = '~'; lines[nl][cp + 1] = 0; }
    pos += take;
    nl++;
  }
  return nl;
}

void onNotify(Notification n) {
  asciiClean(n.app, notifApp, sizeof(notifApp));
  asciiClean(n.title, notifTitle, sizeof(notifTitle));
  asciiClean(n.message, notifMsg, sizeof(notifMsg));
  if (!strcmp(notifTitle, notifMsg)) notifMsg[0] = 0;              // judul sama dengan isi: jangan ditampilkan dua kali
  notifPending = true;
}

// Lonceng digambar di kanvas (tanpa kedip), titik pivot (120,70), bellScale untuk animasi muncul
int bsc(int n) { return max(1, (int)lroundf(n * bellScale)); }

void bellPt(float a, float x, float y, int &ox, int &oy) {
  x *= bellScale; y *= bellScale;
  ox = 120 + (int)lroundf(x * cosf(a) - y * sinf(a));
  oy = 70  + (int)lroundf(x * sinf(a) + y * cosf(a));
}

void cQuad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c) {
  cv->fillTriangle(x0 - OX, y0 - OY, x1 - OX, y1 - OY, x2 - OX, y2 - OY, c);
  cv->fillTriangle(x0 - OX, y0 - OY, x2 - OX, y2 - OY, x3 - OX, y3 - OY, c);
}

void drawBell(float a, uint16_t col, uint16_t clap) {
  int x0, y0, x1, y1, x2, y2, x3, y3;
  bellPt(a, 0, 0, x0, y0);   cCircle(x0, y0, bsc(4), col);                  // gagang
  bellPt(a, 0, 24, x0, y0);  cCircle(x0, y0, bsc(18), col);                 // kubah
  bellPt(a, -18, 24, x0, y0); bellPt(a, 18, 24, x1, y1);
  bellPt(a, 30, 50, x2, y2);  bellPt(a, -30, 50, x3, y3);
  cQuad(x0, y0, x1, y1, x2, y2, x3, y3, col);                               // badan melebar
  bellPt(a, -33, 50, x0, y0); bellPt(a, 33, 50, x1, y1);
  bellPt(a, 33, 56, x2, y2);  bellPt(a, -33, 56, x3, y3);
  cQuad(x0, y0, x1, y1, x2, y2, x3, y3, col);                               // bibir
  bellPt(a, -a * 14.0f, 63, x0, y0);
  cCircle(x0, y0, bsc(6), clap);                                            // pemukul
}

void cChipText(const String &s, int y, uint8_t size, uint16_t color, uint16_t bg) {
  int16_t x1, y1;
  uint16_t w, h;
  cv->setTextSize(size);
  cv->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  cRRect(120 - (int)w / 2 - 8, y - 3, w + 16, h + 6, 9, bg);
  cv->setTextColor(color);
  cv->setCursor(120 - OX - (int)w / 2 - x1, y - OY);
  cv->print(s);
}

uint8_t pickNotifStyle() {
  String l = String(notifApp);
  l.toLowerCase();
  if (l.indexOf("mail") >= 0 || l.indexOf("outlook") >= 0 || l.indexOf("yahoo") >= 0) return 2;
  if (l.indexOf("whats") >= 0 || l.indexOf("telegram") >= 0 || l.indexOf("messeng") >= 0 || l.indexOf("line") >= 0 ||
      l.indexOf("sms") >= 0 || l.indexOf("message") >= 0 || l.indexOf("pesan") >= 0 || l.indexOf("signal") >= 0 ||
      l.indexOf("discord") >= 0 || l.indexOf("chat") >= 0 || l.indexOf("instagram") >= 0) return 1;
  return (notifCount++) % 3;                                       // aplikasi lain: bergantian
}

void drawChat(uint32_t t, float in, float out) {
  float sc = bellScale;
  int w = (int)(88 * sc), h = (int)(52 * sc);
  uint16_t bc = mix565(COL_BG, COL_GREEN, out);
  cRRect(120 - w / 2, 92 - h / 2, w, h, 14, bc);
  cTri(120 - w / 2 + 10, 92 + h / 2 - 4, 120 - w / 2 + 28, 92 + h / 2 - 4, 120 - w / 2 + 6, 92 + h / 2 + 14, bc);
  if (in > 0.8f)
    for (int i = 0; i < 3; i++)                                    // titik mengetik memantul
      cCircle(104 + i * 16, 92 - (int)(fmaxf(0, sinf(t * 0.012f - i * 0.8f)) * 6.0f), 4, mix565(COL_BG, COL_TEXT, out));
}

void drawEnvelope(uint32_t t, float in, float out) {
  float sc = bellScale;
  int cx = 120, cy = 96, w = (int)(80 * sc), h = (int)(54 * sc);
  float open = constrain((t - 450.0f) / 350.0f, 0.0f, 1.0f);
  uint16_t body = mix565(COL_BG, COL_TEXT, out), dim = mix565(COL_BG, COL_DIM, out), oc = mix565(COL_BG, COL_ORANGE, out);
  int x0 = cx - w / 2, y0 = cy - h / 2;
  if (open > 0.0f) {                                               // tutup terbuka + surat naik
    cTri(x0 + 2, y0 + 2, x0 + w - 2, y0 + 2, cx, y0 - (int)(h * 0.45f * open), dim);
    int rise = (int)(open * 26.0f);
    cRRect(x0 + 8, y0 - rise + 4, w - 16, h - 8, 3, oc);
    cRect(x0 + 16, y0 - rise + 14, w - 32, 3, mix565(COL_BG, COL_BG, 1));
    cRect(x0 + 16, y0 - rise + 22, w - 44, 3, mix565(COL_BG, COL_BG, 1));
  }
  cRRect(x0, y0, w, h, 6, body);
  cLine(x0 + 2, y0 + 2, cx, cy + 4, dim);
  cLine(x0 + w - 2, y0 + 2, cx, cy + 4, dim);
  if (open <= 0.0f) cTri(x0 + 2, y0 + 2, x0 + w - 2, y0 + 2, cx, cy + 6, dim);
}

void startNotif() {
  notifStyle = pickNotifStyle();
  buzz(2, 70, 80);
  irisClose();
  notifActive = true;
  notifStart = millis();
  lastNotifRing = -1;
  lastNotifFrame = 0;
  gfx->fillScreen(COL_BG);
}

void endNotif() {
  setDirty = true;
  notifActive = false;
  pendingFade = true;
  lastSig = "";              // paksa gambar ulang halaman
  lastFaceKey = -1;
  gfx->fillScreen(COL_BG);
  if (standby) { drawRing(1.0f, faceConnected ? COL_GREEN : COL_RED); resetStandbyFx(); }
}

void updateNotif() {
  uint32_t now = millis();
  uint32_t t = now - notifStart;
  if (t >= NOTIF_MS) { endNotif(); return; }
  if (now - lastNotifFrame < FRAME_MS) return;
  lastNotifFrame = now;

  // Cincin oranye berputar (diperbarui ~10x/detik agar ringan)
  long rk = t / 100;
  if (rk != lastNotifRing) {
    lastNotifRing = rk;
    drawRing((t % 1000) / 1000.0f, COL_ORANGE);
  }
  if (!canvasOK) return;

  float in = constrain(t / 380.0f, 0.0f, 1.0f);
  float out = constrain((NOTIF_MS - t) / 350.0f, 0.0f, 1.0f);
  float x = in - 1.0f;
  bellScale = 1.0f + 2.70158f * x * x * x + 1.70158f * x * x;       // muncul membal (easeOutBack)
  float decay = 1.0f - 0.6f * (float)t / NOTIF_MS;
  float a = 0.5f * sinf(t * 0.018f) * decay * in;                    // ayunan mengecil
  uint16_t oc = mix565(COL_BG, COL_ORANGE, out);
  float tf = min(constrain((t - 150.0f) / 300.0f, 0.0f, 1.0f), out); // teks fade masuk/keluar

  useClip(2);                                                        // seluruh bagian dalam cincin, cincin tidak tertimpa
  cvClearAll();
  for (int k = 0; k < 3; k++) {                                      // gelombang melebar dari lonceng
    float ph = fmodf(t / 1500.0f + k / 3.0f, 1.0f);
    cv->drawCircle(120 - OX, 108 - OY, 14 + (int)(ph * 46.0f),
                   mix565(COL_BG, COL_ORANGE, (1.0f - ph) * 0.55f * out * in));
  }
  if (notifStyle == 1) drawChat(t, in, out);
  else if (notifStyle == 2) drawEnvelope(t, in, out);
  else {
    drawBell(a, oc, mix565(COL_BG, COL_TEXT, out));
    if ((t / 120) % 2 == 0) {                                        // garis getar kiri-kanan
      cLine(76, 88, 64, 80, oc); cLine(74, 102, 60, 102, oc);
      cLine(164, 88, 176, 80, oc); cLine(166, 102, 180, 102, oc);
    }
  }

  String app = clip(String(notifApp[0] ? notifApp : "Notifikasi"), 20);      // aplikasi
  cChipText(app, 144, 1, mix565(COL_BG, COL_ORANGE, tf), mix565(COL_BG, COL_CHIP, tf));
  if (notifTitle[0]) cTextCenter(clip(String(notifTitle), 14).c_str(), 160, 2, mix565(COL_BG, COL_TEXT, tf));   // pengirim / judul
  if (notifMsg[0]) {                                                         // isi pesan (maks 3 baris)
    char ln[3][24];
    int nl = wrapMsg(notifMsg, ln);
    for (int i = 0; i < nl; i++) cTextCenter(ln[i], 182 + i * 11, 1, mix565(COL_BG, COL_DIM, tf));
  }
  useClip(0);
  present();
}

// ---------- Panggilan masuk ----------
// Dipanggil dari library Chronos: incoming = true saat berdering, false saat selesai/ditolak/diangkat
void onRinger(String caller, bool incoming) {
  if (incoming) {
    strlcpy(callName, caller.c_str(), sizeof(callName));
    callStartPending = true;
  } else {
    callEndPending = true;
  }
}

void startCall() {
  irisClose();
  notifActive = false;
  callActive = true;
  lastCallBeep = -1;
  standby = false;
  lastActivity = millis();
  callStart = millis();
  lastCallRing = -1;
  lastCallFrame = 0;
  gfx->fillScreen(COL_BG);
}

void endCall() {
  setDirty = true;
  callActive = false;
  pendingFade = true;
  buzzStop();
  lastSig = "";
  lastFaceKey = -1;
  gfx->fillScreen(COL_BG);
}

void updateCall() {
  uint32_t now = millis();
  uint32_t t = now - callStart;
  if (now - lastCallFrame < FRAME_MS) return;
  lastCallFrame = now;

  long cb = t / 1500;
  if (cb != lastCallBeep) { lastCallBeep = cb; buzz(3, 120, 100); }   // dering berulang
  long rk = t / 100;
  if (rk != lastCallRing) {
    lastCallRing = rk;
    drawRing((t % 1200) / 1200.0f, COL_GREEN);
  }
  if (!canvasOK) return;

  float in = constrain(t / 350.0f, 0.0f, 1.0f);
  float x = in - 1.0f;
  float sc = 1.0f + 2.70158f * x * x * x + 1.70158f * x * x;
  useClip(2);
  cvClearAll();
  for (int k = 0; k < 3; k++) {                                    // gelombang hijau melebar
    float ph = fmodf(t / 1500.0f + k / 3.0f, 1.0f);
    cv->drawCircle(120 - OX, 96 - OY, 40 + (int)(ph * 30.0f), mix565(COL_BG, COL_GREEN, (1.0f - ph) * 0.6f));
  }
  cCircle(120, 96, (int)(38 * sc), mix565(COL_BG, COL_GREEN, 0.35f));
  float sh = ((t % 1500) < 800) ? sinf(t * 0.05f) * 14.0f : 0.0f;  // gagang bergetar lalu jeda
  float a0 = 25.0f + sh, a1 = 155.0f + sh;
  cArc(120, 86, 31, 23, a0, a1, COL_TEXT);                         // gagang telepon
  float r = 27.0f * 3.1416f / 180.0f;
  cCircle(120 + (int)(cosf(a0 * 0.017453f) * 27.0f), 86 + (int)(sinf(a0 * 0.017453f) * 27.0f) - 2, (int)(9 * sc), COL_TEXT);
  cCircle(120 + (int)(cosf(a1 * 0.017453f) * 27.0f), 86 + (int)(sinf(a1 * 0.017453f) * 27.0f) - 2, (int)(9 * sc), COL_TEXT);
  (void)r;

  float tf = constrain((t - 200.0f) / 300.0f, 0.0f, 1.0f);
  cTextCenter("Panggilan masuk", 148, 1, mix565(COL_BG, COL_DIM, tf));
  String nm = clip(String(callName[0] ? callName : "Tak dikenal"), 12);
  cChipText(nm, 164, 2, mix565(COL_BG, COL_TEXT, tf), mix565(COL_BG, COL_CHIP, tf));
  useClip(0);
  present();
}

// ---------- Foto standby: simpan/baca di partisi flash + mode upload lewat WiFi ----------
#define PHOTO_BYTES   (16 + 240 * 240 * 2)
#define PHOTO_ERASE   (((PHOTO_BYTES + 4095) / 4096) * 4096)
static const char PHOTO_MAGIC[8] = {'M', 'N', 'P', 'H', 'O', 'T', 'O', '1'};
#if ESP_IDF_VERSION_MAJOR >= 5
typedef esp_partition_mmap_handle_t PhotoMapH;
#else
typedef spi_flash_mmap_handle_t PhotoMapH;
#endif

const esp_partition_t *photoPart() {
  const esp_partition_t *p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
  return (p && p->size >= PHOTO_ERASE) ? p : nullptr;
}

#if ESP_IDF_VERSION_MAJOR >= 5
  #define WP_UNMAP(h) esp_partition_munmap(h)
#else
  #define WP_UNMAP(h) spi_flash_munmap(h)
#endif
static PhotoMapH wpH;
static bool wpMapped = false;

uint32_t wpCapKB() {                                               // kapasitas slot (seluruh partisi data)
  const esp_partition_t *p = photoPart();
  return p ? p->size / 1024 : 0;
}

// Petakan isi slot dari flash. Slot kosong / rusak -> foto bawaan (standby_photo.h).
// Isi slot: "MNPHOTO1" (foto mentah RGB565) atau "MNVIDEO1/2" (header + frame JPEG).
void wallpaperLoad() {
  if (wpMapped) { WP_UNMAP(wpH); wpMapped = false; }
  photoPtr = STANDBY_PHOTO;
  vidOK = false;
  const esp_partition_t *p = photoPart();
  if (!p) return;
  uint8_t h[16];
  if (esp_partition_read(p, 0, h, 16) != ESP_OK) return;
  const void *ptr = nullptr;
  if (memcmp(h, PHOTO_MAGIC, 8) == 0) {
    if (esp_partition_mmap(p, 0, PHOTO_BYTES, ESP_PARTITION_MMAP_DATA, &ptr, &wpH) != ESP_OK) return;
    wpMapped = true;
    photoPtr = (const uint16_t *)((const uint8_t *)ptr + 16);
    Serial.println("Wallpaper: foto");
    return;
  }
  bool v2 = memcmp(h, "MNVIDEO2", 8) == 0;
  if (!v2 && memcmp(h, "MNVIDEO1", 8) != 0) return;
  uint32_t hdr = v2 ? VID_HDR2 : VID_HDR1;
  uint16_t n = h[8] | (h[9] << 8), fps = h[10] | (h[11] << 8);
  uint32_t total = h[12] | (h[13] << 8) | (h[14] << 16) | ((uint32_t)h[15] << 24);
  if (n < 1 || n > (v2 ? VID_MAXF : 240) || fps < 1 || fps > 30 || total == 0 || hdr + total > p->size) return;
#if HAVE_ROM_JPEG
  if (!jdPool) jdPool = (uint8_t *)malloc(JD_POOL);               // dialokasi sekarang, selagi RAM lega
  if (!jdPool) { Serial.println("Video: RAM kerja dekoder gagal"); return; }
  if (esp_partition_mmap(p, 0, hdr + total, ESP_PARTITION_MMAP_DATA, &ptr, &wpH) != ESP_OK) { Serial.println("Video: mmap gagal"); return; }
  wpMapped = true;
  const uint32_t *tab = (const uint32_t *)((const uint8_t *)ptr + 16);   // header & tabel frame dibaca langsung dari flash
  for (int i = 0; i < n; i++) {
    uint32_t off = tab[i * 2], len = tab[i * 2 + 1];
    if (len == 0 || len > 65535 || off + len > total) { WP_UNMAP(wpH); wpMapped = false; Serial.println("Video: tabel frame rusak"); return; }
  }
  vidTab = tab; vidBase = (const uint8_t *)ptr + hdr; vidN = n; vidFps = fps; vidOK = true;
  Serial.printf("Wallpaper: video %u frame @ %u fps, %u KB\n", n, fps, (unsigned)(total / 1024));
#else
  Serial.println("Video tidak didukung: rom/tjpgd.h tidak ada di core ini");
#endif
}

void wpInfo(char *out, size_t cap) {                               // keterangan isi slot untuk menu
  const esp_partition_t *p = photoPart();
  uint8_t h[16];
  if (!p || esp_partition_read(p, 0, h, 16) != ESP_OK) { strlcpy(out, "TIDAK ADA PARTISI", cap); return; }
  if (memcmp(h, PHOTO_MAGIC, 8) == 0) strlcpy(out, "ISI: FOTO", cap);
  else if (memcmp(h, "MNVIDEO1", 8) == 0 || memcmp(h, "MNVIDEO2", 8) == 0) {
    unsigned n = h[8] | (h[9] << 8), fps = h[10] | (h[11] << 8);
    snprintf(out, cap, "ISI: VIDEO %u DTK", fps ? (n + fps / 2) / fps : 0);
  }
  else strlcpy(out, "ISI: KOSONG (FOTO BAWAAN)", cap);
}

static const char UPLOAD_PAGE[] PROGMEM = R"rawliteral(<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1"><title>Foto/Video Standby</title>
<style>body{font-family:sans-serif;background:#111;color:#eee;text-align:center;padding:16px}canvas{width:240px;height:240px;border-radius:50%;background:#000;margin:12px auto;display:block}button,input{font-size:16px;margin:6px}button{padding:10px 18px;border:0;border-radius:8px;background:#2a8;color:#fff}#r{background:#a33}button:disabled{opacity:.4}</style></head><body>
<h2>Foto / Video Standby</h2>
<input type=file id=f accept="image/*,video/*"><br>
<canvas id=c width=240 height=240></canvas>
Gelapkan tengah: <input type=range id=d min=0 max=60 value=35 oninput=draw()><br>
FPS video: <select id=fp><option value=0>Otomatis (maks 15)</option><option value=10>10 fps (paling panjang)</option><option value=15>15 fps</option><option value=20>20 fps</option><option value=30>30 fps (paling halus, paling pendek)</option></select><br>
<button id=go onclick=go()>Kirim ke layar</button><button id=r onclick=rs()>Kosongkan</button>
<p id=sl></p>
<p style="font-size:13px;color:#aaa">Satu slot untuk foto ATAU video. Foto besar otomatis diperkecil. Video panjang diatur otomatis (fps &amp; kualitas) agar muat, atau pilih fps sendiri (makin tinggi, makin pendek durasi yang muat); proses di HP bisa beberapa menit, jangan tutup halaman.</p>
<p id=m></p>
<script>
var CAP=0,img=null,vel=document.createElement('video'),isV=false,vready=false,c=document.getElementById('c'),x=c.getContext('2d');
x.imageSmoothingQuality='high';vel.muted=true;vel.playsInline=true;vel.preload='auto';
function m(t){document.getElementById('m').textContent=t}
function shade(){var k=document.getElementById('d').value/100,g=x.createRadialGradient(120,120,0,120,120,110);g.addColorStop(0,'rgba(0,0,0,'+k+')');g.addColorStop(1,'rgba(0,0,0,0)');x.fillStyle=g;x.fillRect(0,0,240,240)}
function sq(src,w,h){var s=Math.min(w,h);x.drawImage(src,(w-s)/2,(h-s)/2,s,s,0,0,240,240)}
function draw(){if(isV){if(!vready)return;sq(vel,vel.videoWidth,vel.videoHeight)}else{if(!img)return;sq(img,img.naturalWidth,img.naturalHeight)}shade()}
function seek(t){return new Promise(function(r){var d=false,f=function(){if(!d){d=true;r()}};vel.onseeked=f;setTimeout(f,4000);if(Math.abs(vel.currentTime-t)<0.0005)setTimeout(f,60);vel.currentTime=t})}
document.getElementById('f').onchange=function(e){
var f=e.target.files[0];if(!f)return;img=null;vready=false;isV=f.type.indexOf('video')==0;m('');
if(isV){vel.onloadeddata=function(){seek(Math.min(0.1,vel.duration/2)).then(function(){vready=true;draw();m('Video '+vel.duration.toFixed(1)+' dtk, '+Math.round(f.size/1048576)+' MB')})};vel.onerror=function(){m('Video tidak bisa dibaca di browser ini')};vel.src=URL.createObjectURL(f);vel.load()}
else{var i=new Image();i.onload=function(){img=i;draw();m('Foto '+i.naturalWidth+'x'+i.naturalHeight+' (akan diperkecil)')};i.onerror=function(){m('Foto tidak bisa dibaca')};i.src=URL.createObjectURL(f)}};
function go(){if(isV){if(!vready){m('Pilih video dulu');return}upv()}else{if(!img){m('Pilih foto/video dulu');return}up()}}
function up(){draw();var d=x.getImageData(0,0,240,240).data,b=new Uint8Array(115200);
for(var i=0,j=0;i<57600;i++,j+=2){var px=i%240,py=(i/240)|0,r=d[i*4],g=d[i*4+1],bl=d[i*4+2];if(Math.hypot(px-119.5,py-119.5)>121){r=g=bl=0}
var v=((r&248)<<8)|((g&252)<<3)|(bl>>3);b[j]=v&255;b[j+1]=v>>8}
var fd=new FormData();fd.append('p',new Blob([b]),'p.bin');m('Mengirim...');
fetch('/up',{method:'POST',body:fd}).then(function(r){return r.text()}).then(m).catch(function(e){m('Gagal: '+e)})}
function rs(){fetch('/reset').then(function(r){return r.text()}).then(m)}
function enc(q){return new Promise(function(r){c.toBlob(r,'image/jpeg',q)})}
function pad(n){return (n+3)&~3}
async function upv(){
var FPSMAX=15,FPSMIN=6,MAXF=900,HDR=8192,BUDGET=Math.floor(((CAP||2424832)-HDR-8192)*0.97),dur=vel.duration,btn=document.getElementById('go');
if(!isFinite(dur)||dur<=0){m('Durasi video tidak terbaca');return}
btn.disabled=true;
async function grab(t){await seek(Math.min(t,dur-0.02));draw()}
m('Menghitung ukuran...');
var S=0,ns=6;
for(var j=0;j<ns;j++){await grab(dur*(j+0.5)/ns);var sb=await enc(0.5);S+=pad(sb.size)}
S/=ns;
var nAff=Math.max(1,Math.floor(BUDGET/(S*1.05)));
var fx=parseInt(document.getElementById('fp').value)||0;
var fps=fx?fx:Math.max(FPSMIN,Math.min(FPSMAX,Math.floor(nAff/dur)));
var n=Math.max(1,Math.min(Math.floor(dur*fps),nAff,MAXF));
var q=0.5,blobs=[],total=0,cut=n<Math.floor(dur*fps);
for(var i=0;i<n;i++){
m('Frame '+(i+1)+'/'+n+' ('+fps+' fps, '+Math.round(total/1024)+' KB dari '+Math.round(BUDGET/1024)+' KB)'+(cut?' - video dipotong dari awal':''));
await grab((i+0.5)/fps);
var b=await enc(q),sz=pad(b.size),per=(BUDGET-total)/(n-i);
blobs.push(b);total+=sz;
if(sz>per)q=Math.max(0.2,q-(sz>per*1.25?0.08:0.04));else if(sz<per*0.8)q=Math.min(0.85,q+0.02);}
while(total>BUDGET&&blobs.length>1){total-=pad(blobs.pop().size)}
n=blobs.length;
var hd=new Uint8Array(HDR),dv=new DataView(hd.buffer);
for(var i=0;i<8;i++)hd[i]='MNVIDEO2'.charCodeAt(i);
dv.setUint16(8,n,true);dv.setUint16(10,fps,true);dv.setUint32(12,total,true);
var off=0,parts=[hd];
for(var i=0;i<n;i++){
var sz=blobs[i].size;
dv.setUint32(16+i*8,off,true);dv.setUint32(20+i*8,sz,true);
parts.push(blobs[i]);
var p=pad(sz)-sz;
if(p)parts.push(new Uint8Array(p).fill(255));
off+=sz+p;}
var fd=new FormData();fd.append('v',new Blob(parts),'v.bin');
m('Mengirim '+n+' frame ('+fps+' fps, '+(n/fps).toFixed(1)+' dtk, '+Math.round(total/1024)+' KB), jangan tutup halaman...');
fetch('/upv',{method:'POST',body:fd}).then(function(r){return r.text()}).then(function(t){m(t);btn.disabled=false}).catch(function(e){m('Gagal: '+e);btn.disabled=false})}
fetch('/info').then(function(r){return r.text()}).then(function(t){CAP=parseInt(t)||0;document.getElementById('sl').textContent='Kapasitas: '+(CAP/1048576).toFixed(1)+' MB (1 slot foto/video)'});
</script></body></html>)rawliteral";

WebServer *web = nullptr;
#define UPBUF_SZ 4096
static uint8_t *upBuf = nullptr;                                   // dialokasi hanya di mode upload (hemat RAM)
static uint32_t upFill = 0, upOff = 16;
static bool     upBad = false, upOk = false;
static uint32_t upDoneAt = 0;

static void upFlush(bool last) {                                   // tulis per blok 4 KB (selalu rata 4 byte)
  if (!upFill) return;
  uint32_t n = upFill;
  if (last) { while (n & 3) upBuf[n++] = 0xFF; }
  if (!upBad && esp_partition_write(photoPart(), upOff, upBuf, n) != ESP_OK) upBad = true;
  upOff += upFill; upFill = 0;
}

static void upHandle() {
  HTTPUpload &u = web->upload();
  if (u.status == UPLOAD_FILE_START) {
    upFill = 0; upOff = 16; upBad = false; upOk = false;
    if (PHOTO_ERASE > photoPart()->size || esp_partition_erase_range(photoPart(), 0, PHOTO_ERASE) != ESP_OK) upBad = true;
  } else if (u.status == UPLOAD_FILE_WRITE) {
    for (size_t i = 0; i < u.currentSize; i++) {
      upBuf[upFill++] = u.buf[i];
      if (upFill == UPBUF_SZ) upFlush(false);
    }
  } else if (u.status == UPLOAD_FILE_END) {
    upFlush(true);
    if (!upBad && upOff - 16 >= 240 * 240 * 2 && upOff - 16 < 240 * 240 * 2 + 4) {
      uint8_t hd[16]; memcpy(hd, PHOTO_MAGIC, 8); memset(hd + 8, 0xFF, 8);
      upOk = esp_partition_write(photoPart(), 0, hd, 16) == ESP_OK;
    }
  }
}

static uint32_t upOffV = 0;
static uint8_t  upMagic[8];

static void upFlushV(bool last) {                                  // tulis per blok 4 KB; tiap sektor dihapus tepat sebelum ditulis
  if (!upFill) return;
  uint32_t n = upFill;
  if (last) { while (n & 3) upBuf[n++] = 0xFF; }
  const esp_partition_t *p = photoPart();
  if (!upBad) {
    if (upOffV == 0) { memcpy(upMagic, upBuf, 8); memset(upBuf, 0xFF, 8); }   // tanda sah ditulis paling akhir
    if (upOffV + n > p->size) upBad = true;
    else {
      if ((upOffV & 4095) == 0 && esp_partition_erase_range(p, upOffV, 4096) != ESP_OK) upBad = true;
      if (!upBad && esp_partition_write(p, upOffV, upBuf, n) != ESP_OK) upBad = true;
    }
  }
  upOffV += upFill; upFill = 0;
}

static void upHandleV() {
  HTTPUpload &u = web->upload();
  if (u.status == UPLOAD_FILE_START) {
    upFill = 0; upOffV = 0; upBad = false; upOk = false;
    const esp_partition_t *p = photoPart();
    if (esp_partition_erase_range(p, 0, 4096) != ESP_OK) upBad = true;
  } else if (u.status == UPLOAD_FILE_WRITE) {
    for (size_t i = 0; i < u.currentSize; i++) {
      upBuf[upFill++] = u.buf[i];
      if (upFill == UPBUF_SZ) upFlushV(false);
    }
  } else if (u.status == UPLOAD_FILE_END) {
    upFlushV(true);
    if (!upBad && upOffV >= VID_HDR2 + 4) upOk = esp_partition_write(photoPart(), 0, upMagic, 8) == ESP_OK;
  }
}

static void uplText(int y, const char *t, uint8_t sz, uint16_t col) {
  panel->setTextSize(sz);
  panel->setTextColor(col);
  int16_t x1, y1; uint16_t w, h;
  panel->getTextBounds(t, 0, 0, &x1, &y1, &w, &h);
  panel->setCursor(120 - (int)w / 2 - x1, y);
  panel->print(t);
}

bool bootPressed();

[[noreturn]] void runUploadMode() {
  panel->begin(SPI_HZ);
  panel->fillScreen(0x0000);
  upBuf = (uint8_t *)malloc(UPBUF_SZ);
  if (!upBuf) ESP.restart();
  uplText(52, "UPLOAD MEDIA", 2, 0xFFFF);
  if (!photoPart()) {
    uplText(100, "Partisi spiffs", 2, 0xF800);
    uplText(122, "tidak ada!", 2, 0xF800);
    uplText(160, "Pilih Partition Scheme", 1, 0xFFFF);
    uplText(172, "Default 4MB with spiffs", 1, 0xFFFF);
    delay(6000);
    ESP.restart();
  }
  uplText(70, "Foto atau video (1 slot)", 1, 0xFFE0);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("MotorNav-Foto", "12345678");
  uplText(88, "WiFi:", 1, 0x07E0);
  uplText(100, "MotorNav-Foto", 2, 0xFFFF);
  uplText(124, "Sandi: 12345678", 1, 0x07E0);
  uplText(146, "Buka browser:", 1, 0x07E0);
  uplText(158, "192.168.4.1", 2, 0xFFE0);
  uplText(186, "Sentuh = batal", 1, 0x7BEF);

  web = new WebServer(80);
  web->on("/", HTTP_GET, []() { web->send_P(200, "text/html", UPLOAD_PAGE); });
  web->on("/up", HTTP_POST, []() {
    web->send(200, "text/plain", upOk ? "Berhasil! Layar restart sebentar..." : "Gagal menyimpan, coba lagi");
    if (upOk) upDoneAt = millis();
  }, upHandle);
  web->on("/reset", HTTP_GET, []() {
    esp_partition_erase_range(photoPart(), 0, 4096);
    web->send(200, "text/plain", "Slot dikosongkan (kembali ke foto bawaan). Restart...");
    upDoneAt = millis();
  });
  web->on("/upv", HTTP_POST, []() {
    web->send(200, "text/plain", upOk ? "Video tersimpan! Layar restart sebentar..." : "Gagal menyimpan video (sudah flash dengan partitions.csv?)");
    if (upOk) upDoneAt = millis();
  }, upHandleV);
  web->on("/info", HTTP_GET, []() {                                // kapasitas slot (byte) untuk browser
    char b[16]; snprintf(b, sizeof(b), "%u", (unsigned)photoPart()->size);
    web->send(200, "text/plain", b);
  });
  web->begin();

  while (digitalRead(PIN_TOUCH) || bootPressed()) delay(10);       // tunggu jari dilepas dulu
  uint32_t t0 = millis();
  while (true) {
    web->handleClient();
    if (upDoneAt && millis() - upDoneAt > 1500) { panel->fillScreen(0x0000); uplText(110, "SELESAI", 2, 0x07E0); delay(800); ESP.restart(); }
    if (digitalRead(PIN_TOUCH) || bootPressed() || millis() - t0 > 900000UL) ESP.restart();   // sentuh = batal, 5 menit tanpa aksi = keluar
    delay(2);
  }
}

// tahan sensor sentuh ~2 detik saat dinyalakan -> mode upload foto
bool uploadRequested() {
  if (!digitalRead(PIN_TOUCH)) return false;
  uint32_t t0 = millis();
  while (millis() - t0 < 2000) {
    if (!digitalRead(PIN_TOUCH)) return false;
    delay(10);
  }
  return true;
}

// ---------- Utama ----------
bool bootPressed() {                                               // tombol BOOT (ESP32 biasa) = sentuhan tambahan
#if PIN_BOOT >= 0
  return digitalRead(PIN_BOOT) == LOW;
#else
  return false;
#endif
}

// ---------- Menu pengaturan (tahan sensor 4 dtk di standby) ----------
#define SET_N 5
const char *SET_NAME[SET_N] = {"MULAI UPLOAD", "TAMPILAN NAV", "TEMA", "STANDBY", "KELUAR"};

void saveU8(const char *k, uint8_t v) {
  prefs.begin("nav", false);
  prefs.putUChar(k, v);
  prefs.end();
}

void drawSettings() {
  gfx->fillScreen(COL_BG);
  drawRing(1.0f, COL_ORANGE);
  drawCentered("PENGATURAN", 40, 1, COL_DIM);
  drawCentered(SET_NAME[setIdx], 76, 2, COL_ORANGE);
  char v[24] = "", info[32] = "";
  switch (setIdx) {
    case 0: strlcpy(v, "TAHAN", sizeof(v)); wpInfo(info, sizeof(info)); break;
    case 1:
      strlcpy(v, navStyle == 1 ? "3D" : (navStyle == 2 ? "LIVE" : "PETA"), sizeof(v));
      strlcpy(info, navStyle == 2 ? "PETA ASLI DARI HP (BLE)" : "TAHAN: UBAH TAMPILAN", sizeof(info));
      break;
    case 2: strlcpy(v, lightTheme ? "TERANG" : "GELAP", sizeof(v)); strlcpy(info, "TAHAN: UBAH TEMA", sizeof(info)); break;
    case 3: strlcpy(v, standbyMode ? "FOTO/VIDEO" : "WAJAH", sizeof(v)); strlcpy(info, "TAHAN: UBAH MODE", sizeof(info)); break;
    default: strlcpy(v, "TAHAN", sizeof(v)); strlcpy(info, "KEMBALI KE STANDBY", sizeof(info)); break;
  }
  drawCentered(v, 106, 3, COL_TEXT);
  drawCentered(info, 148, 1, COL_DIM);
  for (int i = 0; i < SET_N; i++) {
    int x = 120 + (int)((i - (SET_N - 1) / 2.0f) * 14);
    if (i == setIdx) gfx->fillCircle(x, 170, 3, COL_ORANGE);
    else             gfx->drawCircle(x, 170, 3, COL_DIM);
  }
  drawCentered("KLIK=LANJUT  TAHAN=PILIH", 190, 1, COL_DIM);
}

void settingsEnter() {
  irisClose();
  settingsActive = true;
  standby = false;
  setIdx = 0;
  setDirty = true;
  setLastMs = millis();
}

void settingsLeave() {
  settingsActive = false;
  lastActivity = millis();
  enterStandby(isLinked());
}

void settingsAction() {                                            // dipanggil saat item menu ditahan
  switch (setIdx) {
    case 0:
      saveU8("upl", 1);
      delay(300);
      ESP.restart();                                               // masuk mode upload foto/video
      break;
    case 1:
      navStyle = (navStyle + 1) % (MAP_BLE ? 3 : 2);
      saveU8("navst", navStyle);
      break;
    case 2:
      lightTheme = !lightTheme;
      saveU8("light", lightTheme ? 1 : 0);
      buildPalette(1.0f);
      break;
    case 3:
      standbyMode = standbyMode ? 0 : 1;
      saveU8("sbmode", standbyMode);
      break;
    default:
      settingsLeave();
      return;
  }
  setDirty = true;
}

#if MAP_BLE
// ---------- v22.13: upload wallpaper (foto 240x240 RGB565) lewat BLE ----------
// Status (dibaca aplikasi): 0 diam, 1 menghapus, 2 siap, 3 menerima, 4 sukses, 5 gagal.
static volatile uint8_t wpUp = 0;
static volatile bool wpUpReq = false, wpUpErasing = false, wpUpReload = false;
static uint32_t wpUpExp = 0, wpUpFlushed = 0, wpUpEr = 0, wpUpCrc = 0;
static uint16_t wpUpFill = 0;
static volatile uint32_t wpUpLast = 0;                              // aktivitas terakhir (batas waktu 10 dtk)
#define WPU_BYTES (240 * 240 * 2)

static void wpUpSet(uint8_t st) {
  wpUp = st;
  if (mapChr) mapChr->setValue(&st, 1);
}

static uint32_t wpCrc(uint32_t c, const uint8_t *p, size_t n) {    // CRC-32 standar (sama dengan java.util.zip.CRC32)
  while (n--) { c ^= *p++; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
  return c;
}

bool bleWpBusy() { return wpUp >= 1 && wpUp <= 3; }

void bleWpBegin(uint32_t size) {                                   // dipanggil dari tugas BLE
  if (size != WPU_BYTES || !photoPart() || !mapBuf[0]) { wpUpSet(5); return; }
  wpUpExp = 0; wpUpFlushed = 0; wpUpFill = 0; wpUpCrc = 0xFFFFFFFFu;
  mapFid = 255; mapGot = 0; mapTot = 0;
  wpUpLast = millis();
  wpUpSet(1);
  wpUpReq = true;                                                  // penghapusan dikerjakan di loop() agar tugas BLE tidak tertahan
}

static bool wpUpFlush() {                                          // tulis penyangga (mapBuf[0]) ke flash, mulai offset 16
  if (!wpUpFill) return true;
  if (esp_partition_write(photoPart(), 16 + wpUpFlushed, mapBuf[0], wpUpFill) != ESP_OK) return false;
  wpUpFlushed += wpUpFill; wpUpFill = 0;
  return true;
}

void bleWpData(uint32_t off, const uint8_t *d, size_t n) {
  if (wpUp != 2 && wpUp != 3) return;
  wpUpLast = millis();
  if (off < wpUpExp) return;                                       // paket ganda (kirim ulang): abaikan
  if (off != wpUpExp || off + n > WPU_BYTES) { wpUpSet(5); return; }
  wpUpCrc = wpCrc(wpUpCrc, d, n);
  size_t i = 0;
  while (i < n) {
    size_t k = n - i; if (k > 4096u - wpUpFill) k = 4096u - wpUpFill;
    memcpy(mapBuf[0] + wpUpFill, d + i, k);
    wpUpFill += k; i += k;
    if (wpUpFill == 4096 && !wpUpFlush()) { wpUpSet(5); return; }
  }
  wpUpExp += n;
  if (wpUp == 2) wpUpSet(3);
}

void bleWpEnd(uint32_t crc) {
  if (wpUp != 3 || wpUpExp != WPU_BYTES || !wpUpFlush() || (~wpUpCrc) != crc) { wpUpSet(5); return; }
  uint8_t hd[16]; memcpy(hd, PHOTO_MAGIC, 8); memset(hd + 8, 0xFF, 8);   // tanda sah ditulis paling akhir
  if (esp_partition_write(photoPart(), 0, hd, 16) != ESP_OK) { wpUpSet(5); return; }
  wpUpReload = true;                                               // petakan ulang di loop()
  wpUpSet(4);
}

void bleWpTick() {                                                 // dipanggil dari loop()
  if (bleWpBusy() && !wpUpErasing && !wpUpReq && millis() - wpUpLast > 10000) wpUpSet(5);   // HP hilang di tengah upload: lepas kunci frame peta
  if (wpUpReq) {
    wpUpReq = false;
    if (wpMapped) { WP_UNMAP(wpH); wpMapped = false; }            // lepas pemetaan flash sebelum dihapus
    photoPtr = STANDBY_PHOTO; vidOK = false;
    wpUpEr = 0; wpUpErasing = true;
  }
  if (wpUpErasing) {
    if (esp_partition_erase_range(photoPart(), wpUpEr, 4096) != ESP_OK) { wpUpErasing = false; wpUpSet(5); return; }
    wpUpEr += 4096;
    if (wpUpEr >= PHOTO_ERASE) { wpUpErasing = false; wpUpLast = millis(); wpUpSet(2); }
  }
  if (wpUpReload) { wpUpReload = false; wallpaperLoad(); Serial.println("Wallpaper dari BLE tersimpan"); }
}
#endif

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOUCH, INPUT);
#if PIN_BOOT >= 0
  pinMode(PIN_BOOT, INPUT_PULLUP);
#endif
  prefs.begin("nav", false);                                       // permintaan mode upload dari standby (tahan 4 dtk)
  bool uplFlag = prefs.getUChar("upl", 0) != 0;
  if (uplFlag) prefs.putUChar("upl", 0);
  prefs.end();
  if (uplFlag || uploadRequested()) runUploadMode();
  prefs.begin("nav", true);
  lightTheme = prefs.getUChar("light", 0) != 0;
  navStyle = prefs.getUChar("navst", 0);
  if (navStyle >= (MAP_BLE ? 3 : 2)) navStyle = 0;
  prefs.end();
  wallpaperLoad();
  pinMode(PIN_BUZZ, OUTPUT);
  digitalWrite(PIN_BUZZ, LOW);
  analogSetPinAttenuation(PIN_BATT, ADC_11db);
  mpuInit();

  panel->begin(SPI_HZ);
#if TILT_UI
  fb = new SplitCanvas();       // kanvas penuh agar seluruh tampilan bisa dimiringkan
  canvasOK = fb->begin(CANVAS_BEGIN_ARG);
  if (canvasOK) { cv = fb; gfx = fb; }
  else Serial.println("Kanvas penuh gagal dialokasi (memori kurang): tampilan miring nonaktif. Set TILT_UI false.");
#ifdef ARDUINO_ARCH_ESP32
  Serial.printf("Kanvas %s | heap bebas %u | blok terbesar %u\n", canvasOK ? "OK" : "GAGAL", (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
#endif
  {                                                                // layar diagnostik 2 detik saat boot (untuk foto)
    panel->fillScreen(0x0000);
    panel->setTextSize(2);
    panel->setTextColor(canvasOK ? 0x07E0 : 0xF800);
    panel->setCursor(50, 70);  panel->print(canvasOK ? "KANVAS OK" : "KANVAS GAGAL");
    panel->setTextSize(1);
    panel->setTextColor(0xFFFF);
    panel->setCursor(50, 105); panel->printf("heap bebas : %u", (unsigned)ESP.getFreeHeap());
    panel->setCursor(50, 120); panel->printf("blok besar : %u", (unsigned)ESP.getMaxAllocHeap());
    panel->setCursor(50, 135); panel->printf("video %s", vidOK ? "ya" : "tidak");
    panel->setCursor(50, 150); panel->printf("tema %s  slot %u KB", lightTheme ? "terang" : "gelap", (unsigned)wpCapKB());
    delay(2000);
  }
#else
  cv = new Arduino_Canvas(CVW, CVH, (Arduino_G *)panel, CVX, CVY);
  canvasOK = cv->begin(CANVAS_BEGIN_ARG);
  if (!canvasOK) Serial.println("Kanvas wajah gagal dialokasi");
#endif
  buildPalette(1.0f);
  gfx->fillScreen(COL_BG);
  syncFB();

  lastActivity = millis();
  prefs.begin("nav", true);
  standbyMode = prefs.getUChar("sbmode", 0) ? 1 : 0;               // mode standby terakhir
  prefs.end();
#if MAP_BLE
  mapBuf[0] = (uint8_t *)malloc(MAP_BUF_SZ);
  mapBuf[1] = (uint8_t *)malloc(MAP_BUF_SZ);
  if (!jdPool) jdPool = (uint8_t *)malloc(JD_POOL);
  NimBLEDevice::init("Motor Nav");                                 // layanan peta ditambah sebelum Chronos memulai server
  NimBLEServer *msrv = NimBLEDevice::createServer();
  NimBLEService *msvc = msrv->createService(MAP_SVC_UUID);
  NimBLECharacteristic *mch = msvc->createCharacteristic(MAP_CHR_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::READ);
  mch->setCallbacks(&mapCharCb);
  mapChr = mch;
  { uint8_t z = 0; mch->setValue(&z, 1); }                         // status upload wallpaper: 0 = diam
  msvc->start();
#endif
  watch.setNotificationCallback(onNotify);
  watch.setRingerCallback(onRinger);
  watch.begin();
#if MAP_BLE
  NimBLEDevice::getServer()->setCallbacks(&mapSrvCb, false);       // ganti callback agar HP-peta tidak dianggap Chronos
#endif
  watch.set24Hour(true);
}

void handleTouch() {
  bool t = digitalRead(PIN_TOUCH) || bootPressed();
  if (t && !lastTouch && callActive && millis() - lastTap > 300) {
    lastTap = millis();       // sentuhan menutup layar panggilan
    endCall();
    lastTouch = t;
    return;
  }
  if (t && !lastTouch && notifActive && millis() - lastTap > 300) {
    lastTap = millis();       // sentuhan menutup animasi notifikasi
    endNotif();
    lastTouch = t;
    return;
  }
  if (settingsActive) {                                            // menu pengaturan: klik = berikutnya, tahan 0,7 dtk = pilih
    static bool stHold = false, stDone = false;
    static uint32_t stPress = 0;
    if (t && !lastTouch) { stHold = true; stDone = false; stPress = millis(); }
    if (stHold && t && !stDone && millis() - stPress > 700) { stDone = true; buzz(1, 60, 0); setLastMs = millis(); settingsAction(); }
    if (stHold && !t) {
      stHold = false;
      if (!stDone) { setIdx = (setIdx + 1) % SET_N; setDirty = true; setLastMs = millis(); }
    }
    lastTouch = t;
    return;
  }
  static bool gHold = false, gExit = false;
  static uint8_t gPressState = GS_READY;
  static uint32_t gPressMs = 0;
  if (!standby && page >= PAGE_GAME) {                             // di halaman game: sentuh = main/lompat, tahan = keluar
    if (t && !lastTouch) { lastActivity = millis(); gHold = true; gExit = false; gPressMs = millis(); gPressState = gState; if (gState == GS_PLAY) gamePress(); }
    // Layar judul: sentuh singkat = menu berikutnya, TAHAN 0,7 dtk = mulai main.
    // Game over / jeda: sentuh = ulang/lanjut, TAHAN 1 dtk = keluar ke menu berikutnya.
    if (gHold && t && !gExit && gPressState != GS_PLAY && millis() - gPressMs > (gPressState == GS_READY ? 700UL : 1000UL)) {
      gExit = true; gHold = false;
      lastTap = millis(); lastActivity = lastTap;
      buzz(1, 60, 0);
      if (gPressState == GS_READY) gameTapStart();
      else { irisClose(); page = (page + 1) % PAGE_COUNT; lastSig = ""; pendingFade = true; }
    }
    if (gHold && !t) {
      gHold = false;
      if (gPressState == GS_PLAY) gameRelease();
      else if (gPressState == GS_READY) { lastTap = millis(); lastActivity = lastTap; irisClose(); page = (page + 1) % PAGE_COUNT; lastSig = ""; pendingFade = true; }
      else if (!gExit) gameTapStart();
    }
    lastTouch = t;
    return;
  }
  static bool navHold = false, navZeroed = false;
  static uint32_t navPress = 0;
  if (!standby && page == PAGE_NAV) {                              // di Navigasi: tap = halaman berikut, TAHAN 1,5 dtk = nol-kan sudut (motor tegak)
    if (t && !lastTouch && millis() - lastTap > 300) { navHold = true; navZeroed = false; navPress = millis(); }
    if (navHold && t && !navZeroed && millis() - navPress > 1500) { navZeroed = true; tiltZero(); buzz(2, 90, 80); }
    if (navHold && !t) {
      navHold = false;
      if (!navZeroed) { lastTap = millis(); lastActivity = lastTap; irisClose(); page = (page + 1) % PAGE_COUNT; lastSig = ""; pendingFade = true; }
    }
    lastTouch = t;
    return;
  }
  if (standby) {      // standby: tap = bangun | tahan 1,2 dtk lalu lepas = ganti mode | tahan 4 dtk lalu lepas = mode upload foto
    static bool sbHold = false;
    static uint8_t sbStage = 0;
    static uint32_t sbPress = 0;
    if (t && !lastTouch && millis() - lastTap > 300) { sbHold = true; sbStage = 0; sbPress = millis(); }
    if (sbHold && t) {
      uint32_t d = millis() - sbPress;
      if (sbStage == 0 && d > 1200) { sbStage = 1; buzz(1, 60, 0); }       // bunyi 1x: lepas sekarang = ganti mode
      if (sbStage == 1 && d > 4000) { sbStage = 2; buzz(3, 60, 60); }      // bunyi 3x: lepas sekarang = mode upload foto
    }
    if (sbHold && !t) {
      sbHold = false;
      lastTap = millis(); lastActivity = lastTap;
      if (sbStage == 2) {
        settingsEnter();                                            // tahan 4 dtk = menu pengaturan
      } else if (sbStage == 1) {
        standbyMode = standbyMode ? 0 : 1;
        prefs.begin("nav", false); prefs.putUChar("sbmode", standbyMode); prefs.end();
        Serial.printf("Mode standby: %u\n", standbyMode);
        buzz(2, 60, 60);
        enterStandby(isLinked());
      } else {
        irisClose();
        standby = false;
        page = isLinked() ? PAGE_NAV : PAGE_GAME;
        lastSig = "";
        pendingFade = true;
      }
    }
    lastTouch = t;
    return;
  }
  if (t && !lastTouch && millis() - lastTap > 300) {
    lastTap = millis();
    lastActivity = lastTap;
    if (standby) {
      irisClose();
      standby = false;      // sentuhan pertama hanya membangunkan
      page = isLinked() ? PAGE_NAV : PAGE_GAME;   // v20: tanpa Chronos, sentuhan langsung membuka game
    } else {
      irisClose();
      page = (page + 1) % PAGE_COUNT;
    }
    lastSig = "";
    pendingFade = true;
  }
  lastTouch = t;
}

void drawCurrentPage(const Navigation &nav, bool connected) {
  switch (page) {
    case PAGE_NAV:   drawNavPage(nav, connected);   break;
    default: break;
  }
}

// ---------- Jam (disimpan sendiri agar tetap jalan saat Bluetooth putus) ----------
bool     clockSet = false;
int      clockBaseMin = 0;          // menit dalam sehari saat disinkron
uint32_t clockBaseMs = 0;
uint32_t lastClockPoll = 0;

void updateClock(bool connected) {
  if (millis() - lastClockPoll < 1000) return;
  if (appClockMs != 0 && millis() - appClockMs < 90000UL) {        // jam dari aplikasi MotorNav
    lastClockPoll = millis();
    if (appH > 23 || appM > 59 || appS > 59) return;
    uint32_t base = (uint32_t)appH * 3600UL + appM * 60UL + appS + (millis() - appClockMs) / 1000UL;
    clockBaseMin = (int)((base / 60UL) % 1440UL);
    clockBaseMs = millis() - (base % 60UL) * 1000UL;
    clockSet = true;
    return;
  }
  if (!connected) return;
  lastClockPoll = millis();
  int h = watch.getHourC();          // jam 24 jam (set24Hour(true))
  int m = watch.getMinute();
  if (h < 0 || h > 23 || m < 0 || m > 59) return;
  clockBaseMin = h * 60 + m;
  clockBaseMs = millis();
  clockSet = true;
}

// menit dalam sehari, atau -1 jika jam belum pernah tersinkron
int clockMinutes(bool connected) {
  (void)connected;
  if (!clockSet) return -1;
  return (clockBaseMin + (int)((millis() - clockBaseMs) / 60000UL)) % 1440;
}

void loop() {
  watch.loop();
  updateBuzzer();
  handleTouch();
  tiltTick();                                                      // sudut miring dibaca terus di semua mode

#if MAP_BLE
  bleWpTick();
  {
    static uint32_t advT = 0;                                      // iklan tetap menyala selama koneksi < 2 (HP Chronos + HP-peta)
    if (millis() - advT > 2000) {
      advT = millis();
      NimBLEServer *sv = NimBLEDevice::getServer();
      if (sv && sv->getConnectedCount() < 2 && !NimBLEDevice::getAdvertising()->isAdvertising()) NimBLEDevice::startAdvertising();
    }
  }
#endif
  bool connected = isLinked();
  updateClock(connected);
  pollMusic(connected);                                            // detik lagu terus dihitung di semua halaman
  static Navigation nav;                                           // disalin tiap 40 ms saja (hemat heap, FPS naik)
  static uint32_t lastNavPoll = 0;
  if (millis() - lastNavPoll >= 40) { lastNavPoll = millis(); nav = watch.getNavigation(); if (appNavOn()) nav.active = true; }

  if (callEndPending) { callEndPending = false; if (callActive) endCall(); }
  if (callStartPending) { callStartPending = false; startCall(); }
  if (callActive) {
    notifPending = false;                 // panggilan lebih penting dari notifikasi
    updateCall();
    delay(1);
    return;
  }
  if (notifPending) {
    notifPending = false;
    startNotif();
  }
  if (notifActive) {
    updateNotif();
    delay(1);
    return;
  }

  if (nav.active) {
    lastActivity = millis();
    if (standby) { irisClose(); standby = false; lastSig = ""; pendingFade = true; }
  }
  if (nav.active && settingsActive) { settingsActive = false; lastSig = ""; pendingFade = true; }
  if (nav.active && !lastActive) {
    page = PAGE_NAV;
    lastSig = "";
    pendingFade = true;
  }
  lastActive = nav.active;

  if (settingsActive) {                                            // menu pengaturan menggantikan semua halaman
    if (millis() - setLastMs > 60000UL) { settingsLeave(); return; }
    if (setDirty) { setDirty = false; drawSettings(); fbDirty = true; }
    tiltRefresh();
    delay(10);
    return;
  }

  // v20: Bluetooth putus di halaman Navigasi langsung masuk standby (game tetap bisa dimainkan lewat sentuhan)
  if (!standby && !nav.active && (millis() - lastActivity > STANDBY_MS || (!connected && page == PAGE_NAV))) {
    enterStandby(connected);
  }
  if (standby) {
    if (standbyMode == 1 && canvasOK) updatePhotoStandby(connected);
    else updateFace(connected);
    delay(1);
    return;
  }

  if (page >= PAGE_GAME) {                                          // halaman game: standby jika tidak bermain
    if (gState == GS_PLAY) lastActivity = millis();
    updateGamePage(connected);
    delay(1);
    return;
  }

  String sig = String(page);
  sig += "|"; sig += (int)connected;
  sig += "|"; sig += (int)nav.active;
  sig += "|"; sig += nav.directions;
  sig += "|"; sig += nav.title;
  sig += "|"; sig += nav.distance;
  sig += "|"; sig += nav.speed;
  sig += "|"; sig += nav.eta;
  sig += "|"; sig += nav.duration;
  sig += "|"; sig += nav.iconCRC;
#if MAP_BLE
  if (navStyle == 2 || appNavOn()) { sig += "|m"; sig += (unsigned)mapStamp; sig += mapFresh() ? "1" : "0"; }
  sig += appNavOn() ? "|A" : "|a";
#endif

  if (sig != lastSig) {
    lastSig = sig;

    if (nav.active) {
      Serial.printf("NAV dir='%s' title='%s' dist='%s' eta='%s' dur='%s' speed='%s' arah=%d\n",
                    nav.directions.c_str(), nav.title.c_str(), nav.distance.c_str(),
                    nav.eta.c_str(), nav.duration.c_str(), nav.speed.c_str(), turnDirection(nav));
    }

    if (pendingFade) {                                              // halaman timbul perlahan (palet diredupkan)
      pendingFade = false;
      static const float FS[4] = {0.16f, 0.38f, 0.64f, 0.86f};
      for (int k = 0; k < 4; k++) { buildPalette(FS[k]); drawCurrentPage(nav, connected); syncFB(); }
      buildPalette(1.0f);
    }
    drawCurrentPage(nav, connected);
    fbDirty = true;
  }
  tiltRefresh();
  delay(10);
}
