# MediQueue — Simulasi Manajemen Antrean IGD

Program console berbahasa C untuk proyek Struktur Data. Ini adalah simulasi akademik, bukan sistem triase/diagnosis atau alat keputusan medis.

## Struktur data
- Min-Heap sebagai Priority Queue: prioritas angka terkecil didahulukan; jika sama, urutan kedatangan lebih awal didahulukan.
- Binary Search Tree (BST): indeks pencarian pasien berdasarkan ID.
- Singly Linked List: riwayat pasien yang sudah dipanggil.
- Linked list tambahan pada data pasien menjadi pemilik record selama program berjalan.

Heap disimpan dalam array dan membentuk complete binary tree. Modul 6 membahas queue dan priority queue; Modul 7 membahas tree/BST. Pemakaian Min-Heap untuk implementasi priority queue merupakan pengembangan/implementasi yang menghubungkan kedua konsep. Modul 7 tidak perlu diklaim sebagai bab implementasi heap yang lengkap.

## Kebutuhan
- GCC/MinGW (C compiler)
- VS Code (opsional, sebagai editor)

## Kompilasi (Windows PowerShell, dari folder MediQueue)
```powershell
gcc -std=c11 -Wall -Wextra -pedantic src/main.c src/patient.c src/heap.c src/bst.c src/history.c src/storage.c -o MediQueue.exe
```

Jalankan:
```powershell
.\MediQueue.exe
```

Pastikan menjalankan program dari folder utama `MediQueue`, bukan dari folder `src`, agar `data/patients.csv` tersimpan pada lokasi yang benar.

## Kompilasi (Linux/macOS)
```bash
gcc -std=c11 -Wall -Wextra -pedantic src/main.c src/patient.c src/heap.c src/bst.c src/history.c src/storage.c -o MediQueue
./MediQueue
```

## Menu
1. Daftarkan pasien baru
2. Panggil pasien berikutnya
3. Lihat antrean sesuai prioritas
4. Cari pasien lewat BST
5. Tampilkan seluruh pasien (inorder BST)
6. Riwayat pemanggilan (linked list)
7. Visualisasi level Min-Heap
8. Statistik
0. Simpan dan keluar

## Aturan simulasi
- Prioritas berupa angka 1–5. Angka 1 didahulukan daripada 2, dan seterusnya. Angka ini hanya data uji.
- Jika dua pasien memiliki prioritas sama, pasien dengan nomor kedatangan lebih kecil didahulukan (FIFO untuk nilai prioritas yang sama).
- ID harus unik. Nama dan ID tidak boleh mengandung koma karena format penyimpanan CSV sederhana.
- Data otomatis disimpan ke `data/patients.csv` setelah pendaftaran/pemanggilan dan saat keluar.
- Kapasitas antrean menunggu: 1000 pasien.

## Kompleksitas utama
- Insert Min-Heap: O(log n)
- Ambil pasien terdepan (extract-min): O(log n)
- Lihat pasien terdepan (peek): O(1)
- Pencarian BST: rata-rata O(log n), terburuk O(n) jika pohon condong
- Tambah riwayat linked list dengan pointer tail: O(1)
- Menampilkan seluruh data: O(n) kunjungan, belum termasuk biaya output.

## Catatan pengembangan dan pengujian
Modul proyek meminta ADT inti dibangun sendiri, dipisahkan ke header dan implementasi, integrasi dilakukan bertahap, dan pengujian memakai dataset besar (modul menyebut minimal 1000 baris). Untuk pengujian skala, buat data dummy sintetis dan jangan gunakan data pribadi/medis nyata. Tema di luar daftar saran perlu memenuhi kriteria modul dan memperoleh persetujuan dosen.
