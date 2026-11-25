#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append_all(Patient **head, Patient *p) {
    p->next_all = *head; *head = p;
}

int storage_load(const char *filename, Patient **all, MinHeap *heap, BSTNode **index, HistoryList *history, unsigned long *sequence) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0; /* File belum ada: kondisi awal normal. */
    char line[256];
    if (fgets(line, sizeof line, fp) == NULL) { fclose(fp); return 0; }
    while (fgets(line, sizeof line, fp)) {
        Patient *p = (Patient *)calloc(1, sizeof(Patient));
        if (!p) break;
        int status, age, priority;
        unsigned long order;
        char id[ID_LEN], name[NAME_LEN], timestamp[TIME_LEN];
        int fields = sscanf(line, "%15[^,],%59[^,],%d,%d,%lu,%31[^,],%d",
                            id, name, &age, &priority, &order, timestamp, &status);
        if (fields != 7 || age < 0 || priority < 1 || priority > 5 || (status != 0 && status != 1)) { free(p); continue; }
        strcpy(p->id, id); strcpy(p->name, name); strcpy(p->registered_at, timestamp);
        p->age = age; p->priority = priority; p->arrival_order = order;
        p->status = (PatientStatus)status;
        if (!bst_insert(index, p)) { free(p); continue; }
        append_all(all, p);
        if (order > *sequence) *sequence = order;
        if (p->status == STATUS_WAITING) heap_push(heap, p);
        else history_append(history, p);
    }
    fclose(fp); return 1;
}

int storage_save(const char *filename, const Patient *all) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;
    fputs("id,nama,usia,prioritas,urutan,waktu,status\n", fp);
    /* Balikkan urutan linked list saat menyimpan tidak diperlukan untuk fungsi; arrival_order tetap tersimpan. */
    for (const Patient *p = all; p; p = p->next_all)
        fprintf(fp, "%s,%s,%d,%d,%lu,%s,%d\n", p->id, p->name, p->age,
                p->priority, p->arrival_order, p->registered_at, (int)p->status);
    int ok = !ferror(fp); fclose(fp); return ok;
}
