#ifndef HISTORY_H
#define HISTORY_H
#include "patient.h"

typedef struct HistoryNode {
    Patient *patient;
    struct HistoryNode *next;
} HistoryNode;

typedef struct {
    HistoryNode *head;
    HistoryNode *tail;
    int size;
} HistoryList;

void history_init(HistoryList *list);
int history_append(HistoryList *list, Patient *patient);
void history_print(const HistoryList *list);
void history_free_nodes(HistoryList *list);

#endif
