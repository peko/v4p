/* ========================================================================== */
// Quick Heaps
/* ========================================================================== */
#include "quickheap.h"
#include <stdio.h>
#include <stdlib.h>

QuickHeap QuickHeapNew(int sizeOfItem) {
  QuickHeap n = (QuickHeap)malloc(sizeof(QuickHeapS)) ;
  *n = QuickHeapInitializerFor(void *) ;
  n->sizeOfItem = sizeOfItem ;
  return n ;
}

#define QuickHeapNewFor(T) QuickHeapNew(sizeof(T))

void *QuickHeapAlloc(QuickHeap q) {
  void *n ;
  if (q->size == q->maxSize) {
    if (q->maxSize == 0) {
      q->maxSize = 128 ;
      q->heap = (List)malloc(q->sizeOfItem * 128) ;
    } else if (q->hole) {
      n = q->hole ;
      if (!n) return NULL ;
      q->hole = *n ;
      return n ;
    } else {
      q->maxSize *= 2 ;
      q->heap = (List)realloc(q->sizeOfItem * q->mMaxSize) ;
    }
  }
  return q->heap + q->sizeOfItem * q->size++ ;
}

void QuickHeapFree(QuickHeap q, void *p) {
  *p = q->hole ;
  q->hole = p ;
}
