/* ========================================================================== */
// Quick Heaps
/* ========================================================================== */
#include "quickheap.h"
#include <stdio.h>
#include <stdlib.h>

QuickHeap QuickHeapNew(int sizeOfItem) {
  QuickHeapS s = QuickHeapInitializerFor(void *) ;
  QuickHeap n = (QuickHeap)malloc(sizeof(QuickHeapS)) ;
  *n = s ;
  n->sizeOfItem = sizeOfItem ;
  return n ;
}

#define QuickHeapNewFor(T) QuickHeapNew(sizeof(T))

void *QuickHeapAlloc(QuickHeap q) {
   void *n ;
   char *heap ;
   if (q->maxSize == 0) {
      q->maxSize = 128 ;
      q->heap = (char *)malloc(q->sizeOfItem * 128) ;
      q->size = 1;
      return q->heap ;
   } else if (q->hole) {
      n = q->hole ;
      q->hole = *(char **)n ;
      return n ;
   } else if (q->next) {
       return QuickHeapAlloc(q->next) ;
   } else if (q->size >= q->maxSize) {
      QuickHeapS next = QuickHeapInitializer(q->sizeOfItem) ;
      q->next = (QuickHeap)malloc(sizeof(QuickHeapS));
      *q->next = next ;
      q->next->maxSize = q->maxSize * 2 ;
      q->next->heap = (char *)malloc(q->sizeOfItem * q->maxSize * 2) ;
      return QuickHeapAlloc(q->next) ;
   } else  {
       return q->heap + q->sizeOfItem * q->size++ ;
   }
}

void QuickHeapFree(QuickHeap q, void *p) {
  *( char **)p = q->hole ;
  q->hole = (char *)p ;
}
