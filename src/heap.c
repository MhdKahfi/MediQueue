#include "heap.h"
#include <stdio.h>

/* Urutan: prioritas terkecil dahulu, lalu urutan kedatangan terkecil. */
static int comes_before(const Patient *a, const Patient *b) {
    if (a->priority != b->priority) return a->priority < b->priority;
    return a->arrival_order < b->arrival_order;
}

static void swap(Patient **a, Patient **b) {
    Patient *tmp = *a; *a = *b; *b = tmp;
}

static void sift_up(MinHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (!comes_before(heap->items[index], heap->items[parent])) break;
        swap(&heap->items[index], &heap->items[parent]);
        index = parent;
    }
}

static void sift_down(MinHeap *heap, int index) {
    for (;;) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int best = index;
        if (left < heap->size && comes_before(heap->items[left], heap->items[best])) best = left;
        if (right < heap->size && comes_before(heap->items[right], heap->items[best])) best = right;
        if (best == index) break;
        swap(&heap->items[index], &heap->items[best]);
        index = best;
    }
}

void heap_init(MinHeap *heap) { heap->size = 0; }
int heap_is_empty(const MinHeap *heap) { return heap->size == 0; }
Patient *heap_peek(const MinHeap *heap) { return heap->size ? heap->items[0] : NULL; }

int heap_push(MinHeap *heap, Patient *patient) {
    if (heap->size >= HEAP_CAPACITY) return 0;
    heap->items[heap->size] = patient;
    sift_up(heap, heap->size);
    heap->size++;
    return 1;
}

Patient *heap_pop(MinHeap *heap) {
    if (heap->size == 0) return NULL;
    Patient *top = heap->items[0];
    heap->size--;
    if (heap->size > 0) {
        heap->items[0] = heap->items[heap->size];
        sift_down(heap, 0);
    }
    heap->items[heap->size] = NULL;
    return top;
}

void heap_print_levels(const MinHeap *heap) {
    if (!heap->size) { puts("Heap masih kosong."); return; }
    puts("Representasi Min-Heap (per level):");
    int index = 0, level = 0, count = 1;
    while (index < heap->size) {
        printf("Level %d: ", level);
        for (int j = 0; j < count && index < heap->size; ++j, ++index)
            printf("[%s | P%d]  ", heap->items[index]->id, heap->items[index]->priority);
        putchar('\n'); level++; count *= 2;
    }
    puts("Catatan: posisi array menunjukkan bentuk complete binary tree; akar selalu prioritas terdepan.");
}

void heap_print_priority_order(const MinHeap *heap) {
    if (!heap->size) { puts("Tidak ada pasien yang sedang menunggu."); return; }
    MinHeap copy = *heap;
    patient_print_header();
    Patient *p;
    while ((p = heap_pop(&copy)) != NULL) patient_print(p);
}
