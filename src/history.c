#include "history.h"
#include <stdlib.h>
#include <stdio.h>

void history_init(HistoryList *list) { list->head = list->tail = NULL; list->size = 0; }
int history_append(HistoryList *list, Patient *patient) {
    HistoryNode *node = (HistoryNode *)malloc(sizeof(HistoryNode));
    if (!node) return 0;
    node->patient = patient; node->next = NULL;
    if (list->tail) list->tail->next = node; else list->head = node;
    list->tail = node; list->size++; return 1;
}
void history_print(const HistoryList *list) {
    if (!list->head) { puts("Belum ada riwayat pemanggilan."); return; }
    patient_print_header();
    for (HistoryNode *n = list->head; n; n = n->next) patient_print(n->patient);
}
void history_free_nodes(HistoryList *list) {
    HistoryNode *n = list->head;
    while (n) { HistoryNode *next = n->next; free(n); n = next; }
    list->head = list->tail = NULL; list->size = 0;
}
