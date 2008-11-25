/* ========================================================================== */
// Quick Heaps
/* ========================================================================== */
#include "quickheap.h"
#include <stdio.h>
#include <stdlib.h>

QuickHeap QuickHeapNew(int sizeOfItem) {
  QuickHeap p = (QuickHeap)malloc(sizeof(QuickHeapS)) ;
  QuickHeapS newHeap = QuickHeapInitializerFor(void *) ;
  newHeap.sizeOfItem = (sizeOfItem < sizeof(void *) ? sizeof(void *) : sizeOfItem) ;
  *p = newHeap ;
  return p ;
}

QuickHeap QuickHeapReservedNew(int sizeOfItem, int maxSize) {
  QuickHeap p = (QuickHeap)malloc(sizeof(QuickHeapS)) ;
  QuickHeapS newHeap = QuickHeapInitializerFor(void *) ;
  newHeap.sizeOfItem = (sizeOfItem < sizeof(void *) ? sizeof(void *) : sizeOfItem) ;
  newHeap.maxSize = maxSize ;
  if (newHeap.maxSize)
     newHeap.heap = (void *)malloc(newHeap.sizeOfItem * newHeap.maxSize) ;
  *p = newHeap ;
  return p ;
}

void QuickHeapReset(QuickHeap q) {
  QuickHeap p = q->next ;
  while (p) {
    QuickHeap next = p->next ;
    if (p->heap) free(p->heap) ;
    free(p) ;
    p = next ;
  }
  q->hole = NULL ;
  q->size = 0 ;
  q->next = NULL ;
}

void QuickHeapDelete(QuickHeap q) {
  QuickHeapReset(q);
  free(q);
}

#define QuickHeapNewFor(T) QuickHeapNew(sizeof(T))

void *QuickHeapAlloc(QuickHeap q) {
   if (q->maxSize == 0) {
      q->maxSize = 128 ;
      q->heap = (void *)malloc(q->sizeOfItem * 128) ;
      q->size = 1 ;
      return q->heap ;
   } else if (q->hole) {
      void *p = q->hole ;
      q->hole = *(void **)p ;
      return p ;
   } else if (q->next) {
       return QuickHeapAlloc(q->next) ;
   } else if (q->size >= q->maxSize) {
      q->next = QuickHeapReservedNew(q->sizeOfItem, q->maxSize * 2) ;
      return QuickHeapAlloc(q->next) ;
   } else  {
      return q->heap + q->sizeOfItem * q->size++ ;
   }
}

void QuickHeapFree(QuickHeap q, void *p) {
  *( void **)p = q->hole ;
  q->hole = p ;
}
