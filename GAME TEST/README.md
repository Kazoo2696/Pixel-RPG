# Lumina Quest

Prototype action-adventure 2D pixel art orisinal dalam C++ native Win32/GDI. Tidak memakai HTML, JavaScript, engine, atau aset game pihak lain.

## Kontrol

- `WASD` / panah: bergerak
- `J` / `Z` / Space: serang
- `K` / `X` / Shift: dash
- `E` / Enter: interaksi
- `R`: ulang setelah menang/kalah
- `Esc`: keluar

Tujuan: kalahkan para moss slime, ambil 3 Lumina Shard, lalu aktifkan tower cahaya di utara dengan `E`. Tower akan menteleport pemain ke **Moonlit Ruins**, map kedua bernuansa malam dengan Void Wisp dan penjaga yang lebih kuat. Kalahkan semuanya untuk menamatkan permainan. Peti menyediakan pemulih HP.

## Build (Windows)

Dengan Visual Studio Build Tools / Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\LuminaQuest.exe
```

Atau dengan MinGW-w64:

```powershell
g++ -std=c++17 -O2 -municode -mwindows src/main.cpp -lgdi32 -luser32 -o LuminaQuest.exe
.\LuminaQuest.exe
```

Karakter, slime, terrain rumput, peti, pagar, dinding batu, dan efek dash memakai sprite transparan Mystic Woods Free 2.2 di `assets/mysticwoods/`. Aset ini hanya boleh digunakan untuk proyek nonkomersial; ketentuan asli disimpan di `assets/mysticwoods/LICENSE.txt`. Pohon, batu, dekorasi, kristal quest, dan UI digambar secara orisinal karena atlas objek/dekorasi versi gratis mengandung watermark premium dan tidak aman digunakan langsung.
