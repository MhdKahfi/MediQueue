# MediQueue Web

Aplikasi simulasi antrean berbasis web dengan backend C. Struktur data inti dibuat manual menggunakan C:
- Min-Heap / Priority Queue untuk antrean menunggu.
- Binary Search Tree (BST) untuk pencarian berdasarkan ID.
- Linked List untuk riwayat pemanggilan.

## Menjalankan di Windows

1. Pastikan GCC tersedia (misalnya melalui MinGW-w64/MSYS2) dan perintah `gcc` dapat dijalankan dari PowerShell.
2. Buka PowerShell pada folder utama `MediQueue`.
3. Kompilasi:

```powershell
gcc -std=c11 -Wall -Wextra src/server.c src/patient.c src/heap.c src/bst.c src/history.c src/storage.c -o MediQueue.exe -lws2_32
```

4. Jalankan:

```powershell
.\MediQueue.exe
```

5. Buka browser ke `http://localhost:8091`.
6. Biarkan jendela server tetap berjalan selama aplikasi digunakan. Untuk menghentikan server, tekan `Ctrl+C`.

## Menjalankan di Linux

```bash
gcc -std=c11 -Wall -Wextra src/server.c src/patient.c src/heap.c src/bst.c src/history.c src/storage.c -o MediQueue
./MediQueue
```

Kemudian buka `http://localhost:8091`.

## Penyimpanan

Data disimpan pada `data/patients.csv`. Jalankan program dari folder utama proyek agar lokasi file dan folder `web` ditemukan.

## Endpoint API

- `GET /api/patients` — daftar pasien menunggu, sudah diurutkan.
- `POST /api/patients` — mendaftarkan pasien.
- `POST /api/call-next` — memanggil pasien terdepan.
- `GET /api/search?id=P001` — pencarian menggunakan BST.
- `GET /api/history` — riwayat Linked List.
- `GET /api/stats` — statistik.

Prioritas 1–5 merupakan angka simulasi untuk menguji pengurutan struktur data. Aplikasi ini bukan perangkat triase medis dan tidak boleh digunakan untuk keputusan klinis.

## Tampilan antarmuka

Antarmuka web telah diperbarui dengan dashboard bergaya modern, sidebar navigasi, kartu statistik, formulir pendaftaran, panel pemanggilan, tabel antrean, visualisasi Min-Heap, riwayat, dan pencarian pasien. Tampilan menyesuaikan layar desktop maupun ponsel. Seluruh endpoint dan ID elemen JavaScript yang digunakan untuk menghubungkan frontend dengan backend C tetap dipertahankan.

## Workspace multi-halaman

Antarmuka versi ini memiliki halaman terpisah melalui navigasi sidebar:
- Dashboard
- Pendaftaran pasien
- Antrean pasien
- Visualisasi Min-Heap
- Riwayat panggilan
- Pencarian pasien

Navigasi menggunakan hash URL (contoh `http://localhost:8091/#queue`). Backend C dan endpoint API tetap sama.
