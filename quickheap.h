/* ========================================================================== */
// Quick Heaps
/* ========================================================================== */
typedef struct sQuickHeap {
   int sizeOfItem ;
   char *heap ;
   int maxSize ;
   int size ;
   char *hole ;
   struct sQuickHeap *next ;
} QuickHeapS, *QuickHeap ;

#define QuickHeapInitializer(S) { (S), (char *)0, 0, 0, (char *)0, NULL }
#define QuickHeapInitializerFor(T) QuickHeapInitializer(sizeof(T))

QuickHeap QuickHeapNew(int sizeOfItem) ;

#define QuickHeapNewFor(T) QuickHeapNew(sizeof(T))

void *QuickHeapAlloc(QuickHeap q) ;

void QuickHeapFree(QuickHeap q, void  *p) ;
