/* ========================================================================== */
/*                                                                            */
/*   Experimental inline version of a Divide & Conquer type sort algorihm    */
/*   (c) 2008 Garden Sylvain sylvain.garden@gmail.com                         */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
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
List ListMerge(List l1, List l2) {
   struct sList fake_first = {NULL, NULL} ;
   List p = &fake_first ;
   while (l1 && l2)
     {
       if (ListCompare(l1, l2))
         {
           p = ListSetNext(p, l2) ;
           l2 = ListNext(l2) ;
         }
       else
         {
           p = ListSetNext(p, l1) ;
           l1 = ListNext(l1) ;
         }
     }

   ListSetNext(p, (l2 ? l2 : l1)) ;

   return ListNext(fake_first) ;
}

// cut list at end of the first rise, and returns the next rise
List ListGetNextRise(List l) {
   if (!l) return NULL ;

   // sort break search loop
   for (last = l, l = ListNext(l) ;
        l && ListCompare(l, last) ;
        last = l, l = ListNext(l)) ;

   //  cut l at sort break
   if (last) ListSetNext(last, NULL) ;

   return l ;
}

// one direction :
// merge an implicite structure of pairs of n=2^level consecutive rises
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

// THE algorithm *WOOT!*
// repeat until exhaustion of the unordered list :
//  merge the already orderered list with the same-depth list part
//  read and merge-sorted from the head of the unorderer list

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
