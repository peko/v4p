/* ========================================================================== */
// Quick Heaps
/* ========================================================================== */
typedef struct sQuickHeap {
   int sizeOfItem ;
   char *heap ;
   int maxSize ;
   int size ;
   char *hole ;
} QuickHeapS, *QuickHeap ;

#define QuickHeapInitializerFor(T) { sizeof(T), (char *)0, 0, 0, (char *)0 }

QuickHeap QuickHeapNew(int sizeOfItem) ;

#define QuickHeapNewFor(T) QuickHeapNew(sizeof(T))

void *QuickHeapAlloc(QuickHeap q) ;

void QuickHeapFree(QuickHeap q, void  *p) ;
