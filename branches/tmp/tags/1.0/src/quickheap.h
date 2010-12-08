/* ========================================================================== */
// Quick Heaps
/* ========================================================================== */
typedef struct sQuickHeap {
   int sizeOfItem ;
   void *heap ;
   int maxSize ;
   int size ;
   void *hole ;
   struct sQuickHeap *next ;
} QuickHeapS, *QuickHeap ;

#define QuickHeapInitializer(S) { (S), (void *)0, 0, 0, (void *)0, NULL }
#define QuickHeapInitializerFor(T) QuickHeapInitializer(sizeof(T))

QuickHeap QuickHeapNew(int sizeOfItem) ;

#define QuickHeapNewFor(T) QuickHeapNew(sizeof(T))

void *QuickHeapAlloc(QuickHeap q) ;
void QuickHeapReset(QuickHeap q) ;

void QuickHeapFree(QuickHeap q, void  *p) ;
