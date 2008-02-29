/* ========================================================================== */
/*                                                                            */
/*   Experimental inline version of a Divide & Conquer type sort algorihm     */
/*   (c) 2008 Garden Sylvain sylvain.garden@gmail.com                         */
/*                                                                            */
/*   Description                                                              */
/*   I aim a O(n) complexity when the to-be-ordered list is the               */
/*   concatenation of y (y << n) median-sized sub-lists.                      */
/*   There are a lot of applicative case.                                     */
/* ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include "sortable.h"
#include "quickheap.h"

// A Heap for lists
static QuickHeapS listHeapS = QuickHeapInitializerFor(struct sList) ;
static QuickHeap listHeap = &listHeapS ;

// A settable function to compare lists. Please set it before sorting!
int (*ListDataCompare)(void *, void *) = NULL ;

// create a list item
List ListNew() {
  return (List)QuickHeapAlloc(listHeap) ;
}

// free a list item and return next
List ListFree(List p) {
  List next = p->next ;
  QuickHeapFree(listHeap, (void *)p) ;
  return next ;
}

// merge 2 lists by keeping sort direction thanks to mutable links
List ListMerge(List previous, List after) {
   List head, p, tmp ;

   if (!previous || after && !ListCompare(after, previous))
      { // 1st swapping of after & previous lists
      tmp = previous ;
      previous = after ;
      after = tmp ;
      }
   head = p = previous ;
   if (p) previous = ListNext(p) ;

   while (previous && after)
     {
       if (!ListCompare(after, previous))
         {
           // correct broken link
           ListSetNext(p, after) ;
           // swap after & previous lists // xor swap?
           tmp = previous ;
           previous = after ;
           after = tmp ;
         }
       p = previous ;
       previous = ListNext(p) ;
     }

   ListSetNext(p, (after ? after : previous)) ;

   return head ;
}

// cut list at end of the first rise, and returns the next rise
List ListGetNextRise(List l) {
   List last ;

   if (!l) return NULL ;

   // sorting break search loop
   for (last = l, l = ListNext(l) ;
        l && ListCompare(l, last) ;
        last = l, l = ListNext(l)) ;

   // will cut l at sort break
   if (last) ListSetNext(last, NULL) ;

   return l ;
}

// look for and merge a structure of pairs of rises in a list
// n=2^level consecutive rises from the head of the list are merged
static List down(List list, int level, List *remaining) {
  List sub_remaining, l1, l2 ;
  if (!list)
     {
     *remaining = NULL ;
     return NULL ;
     }

  if (!level)
     {
     *remaining = ListGetNextRise(list) ;
     return list ;
     }

  l1 = down(list, level-1, &sub_remaining) ;
  l2 = down(sub_remaining, level-1, remaining) ;
  return ListMerge(l1, l2) ;
}

// repeat until exhaustion of the unordered list :
//  merge-sort a sub-list which weights as much as the already ordered list
//  then merge both as a weight+1 ordered list

List inlineDivideAndConquerSort(List list) {
  List done = NULL, remaining, ordered ;
  int level = 0 ;
  while (list) {
     ordered = down(list, level, &remaining)  ;
     done = ListMerge(done, ordered) ;
     level++ ;
     list = remaining ;
  }
  return done ;
}

#ifdef TEST

int intDataCompare(void *a, void *b) {
   return ((int)a) >= ((int b)) ;
}

void main()
{
   List demo = NULL, orderedList ;

   ListSetDataCompare(intDataCompare) ;

   // create a random list of integers
   for (int i = 0 ; i < 1000 ; i++) {
      List n = ListNew() ;

      ListSetNext(n, demo) ;

      {
        int* ip = malloc(int) ;
        *ip = (int)random(10000) ;
        ListSetData(n, ip) ;
      }

      demo = n ;
   }

   orderedList = inlineDivideAndConquerSort(demo) ;

   for (List p = orderedList ; p ;  p = ListNext(p) ) {
      printf("%d\t", *((int *)ListData(p)) ;
   }
   printf("\n") ;

   while (orderedList) {
     free(ListData(orderedList)) ;
     orderedList = ListFree(orderedList) ;
   }
}

#endif // TEST
