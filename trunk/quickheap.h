/* ========================================================================== */
// Quick Heaps
/* ========================================================================== */
typedef struct sQuickHeap {
   int sizeOfItem ;
   void *heap ;
   int maxSize ;
   int size ;
   void *hole ;
} QuickHeapS, QuickHeap ;

#define QuickHeapInitializerFor(T) { sizeof(T) ; (void *)0 ; 0 ; 0 ; (void *)0 }

QuickHeap QuickHeapNew(int sizeOfItem) ;

#define QuickHeapNewFor(T) QuickHeapNew(sizeof(T))

void *QuickHeapAlloc(QuickHeap q) ;

void QuickHeapFree(QuickHeap q, void *p) ;
