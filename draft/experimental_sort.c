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

// Few lines to define a classical linked list of orderable data
typedef struct sList { void *data ; struct sList *next ; } *List ;
#define ListNew() ((List)malloc(sizeof(struct sList)))
#define ListNext(l) ((l)->next)
#define ListSetNext(l,n) ((l)->next=(n))
#define ListSetData(l,d) ((l)->data=(d))
#define ListCompare(a,b) (*(int *)((a)->data) >= *(int *)((b)->data))


// merge function which keeps sort direction thanks to mutable links
List ListMerge(List previous, List after) {
   struct sList fake_first = {NULL, NULL} ;
   List p = &fake_first ;
   List tmp ;
   ListSetNext(p, previous) ;
   while (previous && after)
     {
       if (!ListCompare(after, previous))
         {
           // correct broken link
           ListSetNext(p, after) ;
           // swap after/previous lists // xor swap?
           tmp = previous ;
           previous = after ;
           after = tmp ;
         }
       p = previous ;
       previous = ListNext(previous) ;
     }

   ListSetNext(p, (after ? after : previous)) ;

   return ListNext(fake_first) ;
}

// THE algorithm *WOOT!*

// cut list at end of the first rise, and returns the next rise
List ListGetNextRise(List l) {
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
List down(List list, int level, List *remaining) {
  List sub_remaining, l1, l2 ;
  if (!list)
     {
     *remaining = NULL ;
     return NULL ;
     }

  if (!hauteur)
     {
     *remaining = ListGetNextRise(list) ;
     return list ;
     }

  l1 = down(list, hauteur-1, &sub_remaining) ;
  l2 = down(sub_remaining, hauteur-1, remaining) ;
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

void main()
{
   List demo = NULL, orderedList ;

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
}
