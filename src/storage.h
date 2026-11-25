#ifndef STORAGE_H
#define STORAGE_H
#include "patient.h"
#include "heap.h"
#include "bst.h"
#include "history.h"

int storage_load(const char *filename, Patient **all, MinHeap *heap, BSTNode **index, HistoryList *history, unsigned long *sequence);
int storage_save(const char *filename, const Patient *all);

#endif
