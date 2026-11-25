#include "patient.h"
#include "heap.h"
#include "bst.h"
#include "history.h"
#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DATA_FILE "data/patients.csv"
#define MAX_INPUT 256

static void clear_line(void) { int c; while ((c = getchar()) != '\n' && c != EOF) {} }
static void read_line(const char *prompt, char *out, size_t size) {
    printf("%s", prompt);
    if (!fgets(out, (int)size, stdin)) { out[0] = '\0'; return; }
    out[strcspn(out, "\r\n")] = '\0';
    if (strchr(out, '\n') == NULL && strlen(out) == size - 1) clear_line();
}
static int read_int(const char *prompt, int min, int max) {
    char buffer[MAX_INPUT], *end; long value;
    for (;;) {
        read_line(prompt, buffer, sizeof buffer);
        value = strtol(buffer, &end, 10);
        while (isspace((unsigned char)*end)) end++;
        if (buffer[0] && *end == '\0' && value >= min && value <= max) return (int)value;
        printf("Input tidak valid. Masukkan angka %d sampai %d.\n", min, max);
    }
}
static int valid_text(const char *s) {
    if (!s[0]) return 0;
    for (; *s; ++s) if (*s == ',' || *s == '\n' || *s == '\r') return 0;
    return 1;
}
static void save_or_warn(Patient *all) {
    if (!storage_save(DATA_FILE, all)) printf("PERINGATAN: data gagal disimpan ke %s.\n", DATA_FILE);
}
static void register_patient(Patient **all, MinHeap *heap, BSTNode **index, unsigned long *sequence) {
    Patient *p = (Patient *)calloc(1, sizeof(Patient));
    if (!p) { puts("Memori tidak cukup."); return; }
    read_line("ID pasien (unik, tanpa koma): ", p->id, sizeof p->id);
    if (!valid_text(p->id) || strlen(p->id) >= ID_LEN || bst_search(*index, p->id)) {
        puts("ID kosong/tidak valid atau sudah terdaftar."); free(p); return;
    }
    read_line("Nama pasien (tanpa koma): ", p->name, sizeof p->name);
    if (!valid_text(p->name)) { puts("Nama tidak valid."); free(p); return; }
    p->age = read_int("Usia (0-120): ", 0, 120);
    puts("\nPrioritas simulasi (bukan kategori triase medis):");
    puts("1 = paling didahulukan dalam simulasi");
    puts("2 = urutan kedua   3 = urutan ketiga");
    puts("4 = urutan keempat 5 = urutan kelima");
    p->priority = read_int("Masukkan angka prioritas (1-5): ", 1, 5);
    p->arrival_order = ++(*sequence); p->status = STATUS_WAITING;
    patient_set_timestamp(p->registered_at, sizeof p->registered_at);
    if (heap->size >= HEAP_CAPACITY) { puts("Antrean mencapai kapasitas maksimum."); free(p); return; }
    if (!bst_insert(index, p)) { puts("Gagal menambahkan indeks BST."); free(p); return; }
    if (!heap_push(heap, p)) { puts("Gagal menambahkan antrean."); free(p); return; }
    p->next_all = *all; *all = p;
    puts("\nPasien berhasil didaftarkan ke antrean simulasi.");
    printf("Posisi terdepan saat ini: %s (prioritas %d)\n", heap_peek(heap)->id, heap_peek(heap)->priority);
    save_or_warn(*all);
}
static void call_next(MinHeap *heap, HistoryList *history, Patient *all) {
    Patient *p = heap_pop(heap);
    if (!p) { puts("Antrean kosong. Tidak ada pasien untuk dipanggil."); return; }
    p->status = STATUS_SERVED;
    if (!history_append(history, p)) {
        /* Tetap tercatat sebagai dipanggil di daftar utama; riwayat bisa dibangun ulang saat program dibuka. */
        puts("Peringatan: node riwayat gagal dialokasikan.");
    }
    puts("\n=== PEMANGGILAN BERIKUTNYA (SIMULASI) ===");
    patient_print_header(); patient_print(p);
    printf("Sisa antrean: %d pasien.\n", heap->size);
    save_or_warn(all);
}
static void search_patient(BSTNode *index) {
    char id[ID_LEN]; read_line("Masukkan ID yang dicari: ", id, sizeof id);
    Patient *p = bst_search(index, id);
    if (!p) { puts("Data pasien tidak ditemukan."); return; }
    puts("\nHasil pencarian BST:"); patient_print_header(); patient_print(p);
}
static void show_stats(const Patient *all, const MinHeap *heap, const HistoryList *history) {
    int total = 0, waiting = 0, served = 0;
    for (const Patient *p = all; p; p = p->next_all) { total++; if (p->status == STATUS_WAITING) waiting++; else served++; }
    puts("\n========== STATISTIK MEDIQUEUE ==========");
    printf("Total pasien terdaftar : %d\n", total);
    printf("Sedang menunggu        : %d\n", waiting);
    printf("Sudah dipanggil        : %d\n", served);
    printf("Ukuran Min-Heap        : %d\n", heap->size);
    printf("Node riwayat           : %d\n", history->size);
    if (heap_peek(heap)) printf("Prioritas terdepan     : %d (ID %s)\n", heap_peek(heap)->priority, heap_peek(heap)->id);
    else puts("Prioritas terdepan     : - (antrean kosong)");
}
static void show_all(BSTNode *index) {
    if (!index) { puts("Belum ada data pasien."); return; }
    puts("Data pasien berdasarkan urutan ID (BST inorder):"); patient_print_header(); bst_inorder(index);
}
static void menu(void) {
    puts("\n============================================================");
    puts("                 MEDIQUEUE | SIMULASI IGD");
    puts("============================================================");
    puts("1. Daftarkan pasien baru");
    puts("2. Panggil pasien berikutnya");
    puts("3. Lihat antrean berdasarkan urutan prioritas");
    puts("4. Cari pasien berdasarkan ID (BST)");
    puts("5. Lihat seluruh data pasien (BST inorder)");
    puts("6. Lihat riwayat pemanggilan (Linked List)");
    puts("7. Visualisasi Min-Heap");
    puts("8. Statistik sistem");
    puts("0. Simpan dan keluar");
    puts("------------------------------------------------------------");
}
static void free_all(Patient *all) { while (all) { Patient *next = all->next_all; free(all); all = next; } }

int main(void) {
    MinHeap heap; BSTNode *index = NULL; HistoryList history;
    Patient *all = NULL; unsigned long sequence = 0;
    heap_init(&heap); history_init(&history);
    if (!storage_load(DATA_FILE, &all, &heap, &index, &history, &sequence))
        puts("Belum ada file data. MediQueue dimulai dengan data kosong.");
    else printf("Data berhasil dimuat. Total antrean menunggu: %d\n", heap.size);

    int choice;
    do {
        menu(); choice = read_int("Pilih menu: ", 0, 8);
        switch (choice) {
            case 1: register_patient(&all, &heap, &index, &sequence); break;
            case 2: call_next(&heap, &history, all); break;
            case 3: puts("\nAntrean dari prioritas tertinggi ke terendah:"); heap_print_priority_order(&heap); break;
            case 4: search_patient(index); break;
            case 5: show_all(index); break;
            case 6: puts("\nRiwayat pasien yang sudah dipanggil:"); history_print(&history); break;
            case 7: heap_print_levels(&heap); break;
            case 8: show_stats(all, &heap, &history); break;
            case 0: save_or_warn(all); puts("Data disimpan. Terima kasih telah menggunakan MediQueue."); break;
        }
    } while (choice != 0);

    history_free_nodes(&history); bst_free(index); free_all(all);
    return 0;
}
