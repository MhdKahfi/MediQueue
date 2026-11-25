#include "patient.h"
#include <stdio.h>
#include <time.h>

void patient_print_header(void) {
    printf("%-12s %-24s %-5s %-9s %-21s %-12s\n",
           "ID", "NAMA", "USIA", "PRIORITAS", "WAKTU DAFTAR", "STATUS");
    printf("-----------------------------------------------------------------------------------------------\n");
}

void patient_print(const Patient *p) {
    if (!p) return;
    printf("%-12s %-24.24s %-5d %-9d %-21.21s %-12s\n",
           p->id, p->name, p->age, p->priority, p->registered_at,
           p->status == STATUS_WAITING ? "Menunggu" : "Dipanggil");
}

void patient_set_timestamp(char *buffer, unsigned long size) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    if (local) strftime(buffer, size, "%Y-%m-%d %H:%M:%S", local);
    else snprintf(buffer, size, "waktu-tidak-tersedia");
}
