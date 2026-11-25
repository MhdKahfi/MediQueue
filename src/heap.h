#ifndef HEAP_H
#define HEAP_H
#include "patient.h"

#define HEAP_CAPACITY 1000

typedef struct {
    Patient *items[HEAP_CAPACITY];
    int size;
} MinHeap;

void heap_init(MinHeap *heap);
int heap_push(MinHeap *heap, Patient *patient);
Patient *heap_pop(MinHeap *heap);
Patient *heap_peek(const MinHeap *heap);
int heap_is_empty(const MinHeap *heap);
void heap_print_levels(const MinHeap *heap);
void heap_print_priority_order(const MinHeap *heap);

#endif
