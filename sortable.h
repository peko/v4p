/* ========================================================================== */
/*   Sortable Lists +                                                         */
/*   Experimental inline version of a Divide & Conquer type sort algorihm     */
/*   (c) 2008 Garden Sylvain sylvain.garden@gmail.com                         */
/* ========================================================================== */

// A settable function to compare lists. Please set it before sorting!
extern int (*ListDataCompare)(void *, void *) ;
#define ListSetDataCompare(p) ListDataCompare = (p)

// A classical linked list of orderable data
typedef struct sList { void *data ; struct sList *next ; } *List ;

// create list
List ListNew() ;

// free list and return next
List ListFree(List p) ;

#define ListData(l) ((l)->data)
#define ListNext(l) ((l)->next)
#define ListSetNext(l,n) ((l)->next=(n))
#define ListSetData(l,d) ((l)->data=(d))
#define ListCompare(A,B) ListDataCompare((A)->data,(B)->data)
#define ListAdd(l,n) (ListSetNext(n, l), l = n)
#define ListAddData(l,d) { List _n = ListNew() ; ListSetData(_n, d) ; ListAdd(l, _n) ; }

// merge 2 lists by keeping sort direction thanks to mutable links
List ListMerge(List previous, List after) ;

// cut list at end of the first rise, and returns the next rise
List ListGetNextRise(List l) ;

// repeat until exhaustion of the unordered list :
//  - merge-sort a sub-list which weights as much as the already ordered list
//  - then merge both as a weight+1 ordered list
List inlineDivideAndConquerSort(List list) ;
#define ListSort(l) inlineDivideAndConquerSort(l)

