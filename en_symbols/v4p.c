/* v4p = Vectors rendition engine For Pocket

   This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/* Abbrevs
** display size = rectangle lineNb*lineWidth
** view = rectangular area defining what part of the scene to display
** scene = a polygons list
** Polygon = closed path of n points, with color and depth
** Color = any data needed by the external drawing function
** Point = x,y coordinates in scene referential
** ActiveEdge : non horizontal edge inside view
** sub : sub-polygon related to a parent polygon (rotation,moving,optimisation)
** z = depth = layer #
** XHeap : reserved memory for X
** XP : X pointer
** IX : X table indice
** XCallBack : external function
** opened polygon : polygon intersected by the scan-line : min(y(points)) < y < max(y(points))
** to be opened polygon : y(scan-line) < min(y(points))
** closed polygon : y(scan-line) > max(y(points))
*/

#include "v4p.h"
#include <stdlib.h>
#include "lowmath.h"
#include "quickheap.h"
#include "sortable.h"

// external data&func
extern Coord      lineNb, lineWidth ; // display size
extern Color    bgColor ; // Color - background

extern Boolean v4pDisplayCallBack() ;
extern Boolean v4pErrorCallBack(char *s,...) ;
extern Boolean v4pSliceCallBack(Coord y, Coord x0, Coord x1, Color c) ;
extern Boolean v4pCollideCallBack(ICollide i1, ICollide i2, Coord py, Coord x1, Coord x2, PolygonP p1, PolygonP p2) ;
extern Boolean v4pDisplayEndCallBack() ;


// Polygon type
typedef struct poly_s {
 PolygonProps props ; // property flags
 PointP point1 ; // points list
 Color color ; // color (any data needed by the drawing function)
 ILayer z ; // depth
 ICollide i ; // collide layer index (i may be != z)
 PolygonP sub1 ; //  subs list
 PolygonP next ; // subs list link
 Coord minx,maxx,miny,maxy; // minimal surrounding rectangle
 Coord minyv, maxyv ;
 ActiveEdgeP ActiveEdge1 ; // ActiveEdges list
} Polygon ;

// type ActiveEdge
typedef struct ba_s {
 Coord x0, y0, x1, y1 ; // vector
 PolygonP p ; // polygone
 Coord x, o1, o2, s, h, r1, r2 ; // bresenham: offsets, sums, lineNb, remainings
 ActiveEdgeP next ;
} ActiveEdge ;

// contexte V4P
typedef struct v4pContext_s {
 PolygonP *scene ; // scene = a polygon set
 Coord step ; // step
 int debug1 ;
 Coord   xvu0,yvu0,xvu1,yvu1 ; // view coordinates
 QuickHeap pointHeap, polygonHeap, activeEdgeHeap ;
 List    openedPolygonsList, openablePolygonsList ; // polygons lists
 List    activeEdgeXSortedList, activeEdgeYSortedList ; // ActiveEdge lists
 Coord   dxvu,dyvu, // view related data
  divxvu,modxvu,divxvub,modxvub,
  divyvu,modyvu,divyvub,modyvub;
 Boolean scaling; // do polygons need to be scaled?
} V4pContextS, *V4pContext ;

V4pContext v4p = NULL ; // current (selected) v4p Context

// define the current view
int v4pSetView(Coord x0, Coord y0, Coord x1, Coord y1) {
  v4p->xvu0=x0;
  v4p->yvu0=y0;
  v4p->xvu1=x1;
  v4p->yvu1=y1;
  v4p->dxvu=x1-x0;
  v4p->dyvu=y1-y0;
  if (!v4p->dxvu || !v4p->dyvu) return failure; // can't divide by 0
  v4p->divxvu = lineWidth / v4p->dxvu;
  v4p->modxvu = lineWidth % v4p->dxvu;
  v4p->divxvub = v4p->dxvu / lineWidth;
  v4p->modxvub = v4p->dxvu % lineWidth;
  v4p->divyvu = lineNb / v4p->dyvu;
  v4p->modyvu = lineNb % v4p->dyvu;
  v4p->divyvub = v4p->dyvu / lineNb;
  v4p->modyvub = v4p->dyvu % lineNb;
  v4p->scaling = !(v4p->divxvu==1 && v4p->divyvu==1 && v4p->modxvu==0 && v4p->modyvu==0);
  return success;
}


// create and select a v4p context
V4pContext v4pContextNew() {
  v4p = (V4pContext)malloc(sizeof(V4pContextS)) ;
  v4p->scene = NULL;
  v4p->step = 8;
  v4p->pointHeap = QuickHeapNewFor(Point) ;
  v4p->polygonHeap = QuickHeapNewFor(Polygon) ;
  v4p->activeEdgeHeap = QuickHeapNewFor(ActiveEdge) ;
  v4pSetView(0,0, lineWidth, lineNb);
  return v4p ;
}

// select a v4p context
int v4pContextSet(V4pContext p) {
  v4p = p ;
}

int v4pInit() {
  v4pContextNew() ;
  return success ;
}

// create a polygon
PolygonP v4pPolygonNew(PolygonProps t, Color col, ILayer z) {
  PolygonP p = QuickHeapAlloc(v4p->polygonHeap) ;
  p->props = t;
  p->z = z;
  p->i = (ICollide)-1;
  p->color = col;
  p->point1 = NULL;
  p->sub1 = NULL;
  p->next = NULL;
  p->miny = ~0; // miny = too much => limits to be computed
  return p;
}

// delete a poly (including its points and subs)
int v4pPolygonDel(PolygonP p) {
 while (p->point1) v4pPolygonDelPoint(p,p->point1);
 while (p->sub1) v4pPolygonDelSub(p,p->sub1);
 QuickHeapFree(v4p->polygonHeap,p);
 return success ;
}

// Add a polygon to a list linked by the 'next' pointer
PolygonP v4pPolygonIntoList(PolygonP p, PolygonP* list) {
  p->next=*list;
  *list=p;
  return p ;
}

// remove a polygon from a list linked by the next pointer
Boolean v4pPolygonOutOfList(PolygonP p, PolygonP* list) {
  PolygonP ppl,pl;

  if (*list == p)
    *list = p->next;
  else {
    ppl = *list;
    pl = ppl->next;
    while (pl != p && pl) {
      ppl = pl;
      pl = pl->next;
    }
    if (!pl) return (v4pErrorCallBack("poly perdu"), failure) ;
    ppl->next = p->next;
  }
  p->next=NULL;
  return success ;
}

// Add a polygon to an other polygon subs list
PolygonP v4pPolygonAddSub(PolygonP parent, PolygonP p) {
  return v4pPolygonIntoList(p, &parent->sub1);
}

// combo PolygonNew+PolygonIntoList
PolygonP v4pListAddPolygon(PolygonP *list, PolygonProps t, Color col, ILayer z) {
  return v4pPolygonIntoList(v4pPolygonNew(t, col, z), list);
}

// combo ListDelPolygon+PolygonOutOfList
Boolean v4pListDelPolygon(PolygonP *list, PolygonP p) {
  return v4pPolygonOutOfList(p, list) || v4pPolygonDel(p) ;
}

// combo PolygonAPolygon+PolygonNew
PolygonP v4pPolygonAddNewSub(PolygonP parent, PolygonProps t, Color col, ILayer z) {
   return v4pPolygonAddSub(parent, v4pPolygonNew(t, col, z)) ;
}

// remove a poly from an other poly subs list, then delete it
Boolean v4pPolygonDelSub(PolygonP parent, PolygonP p) {
   return v4pPolygonOutOfList(p, &parent->sub1) || v4pPolygonDel(p) ;
}

// set a polygon property flag
PolygonProps v4pPolygonPutProp(PolygonP p, PolygonProps i) {
  return (p->props |= i) ;
}

// remove a polygon property flag
PolygonProps v4pPolygonRemoveProp(PolygonP p, PolygonProps i) {
  return (p->props &= ~i) ;
}

// Add a polygon point
PointP v4pPolygonAddPoint(PolygonP p, Coord x, Coord y) {
   PointP s = QuickHeapAlloc(v4p->pointHeap) ;
   s->x = x;
   s->y = y;
   if (!p->point1) {
      p->minx = p->maxx = x ;
      p->miny = p->maxy = y ;
   } else {
      if (x < p->minx) p->minx = x ;
      else if (x > p->maxx) p->maxx = x ;
      if (y < p->miny) p->miny = y ;
      else if (y > p->maxy) p->maxy = y ;
   }
   s->next = p->point1;
   p->point1 = s;
   // p modif
   return s ;
}

// set polygon color
Color v4pPolygonSetColor(PolygonP p, Color c) {
   return p->color = c;
}

// returns a polygon points list
PointP v4pPolygonGetPoints(PolygonP p) {
   return p->point1 ;
}

// returns a polygon depth (layer index)
ILayer v4pPolygonGetZ(PolygonP p) {
   return p->z ;
}

// returns a polygon color
Color v4pPolygonGetColor(PolygonP p) {
   return p->color ;
}

// move a polygon point
PointP v4pPolygonMovePoint(PolygonP p, PointP s, Coord x, Coord y) {
   if (p->miny == ~0)
      {}
   else if (s->x == p->minx || s->y == p->miny
      || s->x == p->maxx || s->y == p->maxy) {
      p->miny = ~0 ; // calcul a refaire
   } else {
      if (x < p->minx) p->minx = x ;
      else if (x > p->maxx) p->maxx = x ;
      if (y < p->miny) p->miny = y ;
      else if (y > p->maxy) p->maxy = y ;
   }
   s->x = x;
   s->y = y;
   // p modif
   return s ;
}

// remove a point from a polygon
PolygonP v4pPolygonDelPoint(PolygonP p, PointP s) {
   PointP pps, ps;

   if (p->point1 == s)
      p->point1 = s->next;
   else {
     pps = p->point1;
     ps = pps->next;
     while (ps != s && ps) {
        pps = ps;
        ps = ps->next;
     }
     if (!ps) return NULL ;
     pps->next = ps->next;
   }

   if (p->miny != ~0
       && (s->x == p->minx || s->y == p->miny
           || s->x == p->maxx || s->y == p->maxy)) {
      p->miny = ~0 ; // calcul a refaire
   }

   QuickHeapFree(v4p->pointHeap, s) ;

   // p modif
   return p ;
}

// transform hexa char ('0-9,A-F') to int
int v4pXToD(char c) {
   int o,r ;
   o = (int)c ;
   r = o - (int)'0' ;
   if (r >= 0 && r <= 9)
      return r ;
   else {
      r =o - (int)'A';
      if (r >= 0 && r <= 5)
         return 10 + r ;
      else {
         r = o - (int)'a' ;
         return (r >= 0 && r <= 5
                 ? 10 + r : 0) ;
} } }

// add points to a polygon with coordinates decoded from a c-string
PolygonP v4pPolygonDecodePoints(PolygonP p, char *s) {
   int l, j;
   Coord xs, ys, xs1, ys1;
   Boolean sep, psep;
   char c ;

   psep = false;
   for (j = 0 ; s[j] ; j++) {
      c = s[j] ;
      if (c == ' ')
         continue ;

      if (c == '.') {
         sep = true;
         if (psep)
            v4pPolygonAddPoint(p, xs1 * v4p->step, ys1 * v4p->step);
         continue;
      }

      sep = false;

      xs = v4pXToD(c) << 4 + v4pXToD(s[++j]);
      j++ ;
      ys = v4pXToD(c) << 4 + v4pXToD(s[++j]);
      j++;
      v4pPolygonAddPoint(p, xs * v4p->step, ys * v4p->step);

      if (sep) {
         xs1 = xs ;
         ys1 = ys ;
         psep = true;
      }
  }
  if (psep)
     v4pPolygonAddPoint(p, xs1 * v4p->step, ys1 * v4p->step);

  return p;
}

// create a polygon by adding all points encoded in a c-string
PolygonP v4pQuickPolygon(PolygonProps t, Color col, ILayer z, char* s) {
  return v4pPolygonDecodePoints(v4pPolygonNew(t, col, z), s);
}

// encode every point of a polygon into a single c-string
char *v4pPolygonEncodePoints(PolygonP p) {
  static char *t = "0123456789ABCDEF" ;
  char *s;
  PointP s1, m, pm;
  int i, l ;
  Coord v;

  s = (char *)malloc(32 * sizeof(char)) ;
  l = 0;
  s1 = p->point1 ;
  m = s1;
  pm = NULL;
  while (m) {
     if (pm && m == s1) {
        s[l++] = '.';
        pm = NULL;
        s1 = m->next;
        m = s1;
     } else {
        for (i = 0; i <= 1; i++) {
           if (!i) v = m->x;
              else v = m->y;
           v /= v4p->step;
           s[l++] = t[v & 15];
           s[l++] = t[(v >> 4) & 15];
         }
         pm = m;
         m = m->next;
     }
     if (l % 32 >= 28) {
        s = (char *)realloc(s, (64 + l - l % 32) * sizeof(char)) ;
        if (!s) { v4pErrorCallBack("full Heap") ; return NULL ; }
     }
  }
  s[l] = '\0';
  return s;
}

// set the polygon colliding layer
PolygonP v4pPolygonConcrete(PolygonP p, ICollide i) {
   p->i = i ;
   return p ;
}

// create an ActiveEdge linked by 'next' to other polygon ActiveEdges
ActiveEdgeP v4pEnableEdgeNew(PolygonP p) {
   ActiveEdgeP b = QuickHeapAlloc(v4p->activeEdgeHeap) ;
   b->p = p;
   b->next = p->ActiveEdge1;
   return (p->ActiveEdge1 = b);
}

// delete all ActiveEdges of a poly
PolygonP v4pPolygonDelActiveEdges(PolygonP p) {
   ActiveEdgeP b, s;
   for (b = p->ActiveEdge1; b; b = s) {
      s = b->next;
      QuickHeapFree(v4p->activeEdgeHeap, b) ;
   }
   p->ActiveEdge1 = NULL;
   return p;
}

// merge-sort fn
int compareListActiveEdgey0(void *data1, void *data2) {
   ActiveEdgeP b1 = data1, b2 = data2 ;
   return (b1->y0 < b2->y0) ;
}

List v4pSortListActiveEdgey(List list) {
   ListSetDataPrior(compareListActiveEdgey0) ;
   return ListSort(list) ;
}

// called by v4pPolygonTransformClone()
PolygonP v4pRecPolygonTransformClone(Boolean estSub, PolygonP p, PolygonP c, Coord dx, Coord dy, ILayer dz) {
   PointP sp, sc;
   Coord x, y, x2, y2;
   p->z = c->z+dz;
   sp = p->point1;
   sc = c->point1;
   while (sp) {
      x = sp->x;
      y = sp->y;
      straighten(x, y, &x2, &y2);
      x2+= dx;
      y2+= dy;
      v4pPolygonMovePoint(c, sc, x2, y2) ;
      sp = sp->next;
      sc = sc->next;
   }
   if (estSub && p->next)
      v4pRecPolygonTransformClone(true, p->next, c->next, dx, dy, dz);
   if (p->sub1)
      v4pRecPolygonTransformClone(true, p->sub1, c->sub1, dx, dy, dz);
   // c modif
   return c ;
}

// transform a clone c of a polygon p so that points(c) = transfo(points(p),delta-x/y, turn-angle)
PolygonP v4pPolygonTransformClone(PolygonP p, PolygonP c, Coord dx, Coord dy, int angle, ILayer dz) {
  computeCosSin(angle);
  /* a voir: ratiox et ratioy :
       cosa:=(cosa*ratiox) shr 7;
       sina:=(sina*ratioy) shr 7;
   */
  return v4pRecPolygonTransformClone(false, p, c, dx, dy, dz);
}

// transform a polygon
PolygonP v4pPolygonTransform(PolygonP p, Coord dx, Coord dy, int angle, ILayer dz) {
   return v4pPolygonTransformClone(p, p, dx, dy, angle, dz);
}

// called by v4pPolygonClone
PolygonP v4pRecPolygonClone(Boolean estSub, PolygonP p) {
   PointP s;
   PolygonP c = v4pPolygonNew(p->props, p->color, p->z);

   for (s = p->point1 ; s ; s = s->next)
      v4pPolygonAddPoint(c, s->x, s->y);

   if (estSub && p->next)
      c->next = v4pRecPolygonClone(true, p->next);
   if (p->sub1)
      c->sub1 = v4pRecPolygonClone(true, p->sub1);

   return c;
}

// clone a polygon (including its descendants)
PolygonP v4pPolygonClone(PolygonP p) {
   return v4pRecPolygonClone(false, p);
}

// combo PolygonClone+PolygonIntoList
PolygonP v4pListAddClone(PolygonP *list, PolygonP p) {
   return v4pPolygonIntoList(v4pPolygonClone(p), list);
}

#define v4pMinCoord(A,B) ((A) < (B) ? (A) : (B))
#define v4pMaxCoord(A,B) ((A) < (B) ? (B) : (A))

// compute the minimal rectangle surrounding a polygon
PolygonP v4pPolygonComputeLimits(PolygonP p) {
   PointP s = p->point1;
   if (!s) { // pas de point
      p->miny = ~0;
   } else {
      Coord minx, maxx, miny, maxy;
      maxx = minx = s->x;
      maxy = miny = s->y;
      for (s = s->next; s; s = s->next) {
         if (s->x < minx) minx = s->x;
         else if (s->x > maxx) maxx = s->x;
         if (s->y < miny) miny = s->y;
         else if (s->y > maxy) maxy = s->y;
      }
      p->minx = minx;
      p->miny = miny;
      p->maxx = maxx;
      p->maxy = maxy;
   }
   return p;
}

// return false if polygon is located outside from the view area
Boolean v4pIsVisible(PolygonP p) {
   if (!p->point1)
      return false ;
   if (p->miny == ~0) // bornes pas a jour
      v4pPolygonComputeLimits(p) ;

   { Coord minx = p->minx, maxx = p->maxx, miny = p->miny, maxy = p->maxy;

   if (!(p->props & relative)) {
      minx -= v4p->xvu0;
      miny -= v4p->yvu0;
      maxx -= v4p->xvu0;
      maxy -= v4p->yvu0;
      if (v4p->scaling) {
         minx = minx * v4p->divxvu + (minx * v4p->modxvu) / v4p->dxvu;
         maxx = maxx * v4p->divxvu + (maxx * v4p->modxvu) / v4p->dxvu;
         miny = miny * v4p->divyvu + (miny * v4p->modyvu) / v4p->dyvu;
         maxy = maxy * v4p->divyvu + (maxy * v4p->modyvu) / v4p->dyvu;
      }
   }
   p->minyv = miny ;
   p->maxyv = maxy ;
   return (maxx >= 0 && maxy >= 0 && minx < lineWidth && miny < lineNb) ;
   }
}

// build a list of ActiveEdges for a given polygon
PolygonP v4pPolygonBuildActiveEdgeList(PolygonP p) {
   PointP sa, sb, s1;
   Boolean loop, end;
   int dx, dy, q, r;
   Coord sx0, sx1, sy0, sy1;
   ActiveEdgeP b ;


   p->ActiveEdge1 = NULL;
   if (!v4pIsVisible(p) || (p->props & invisible))
      return p;

   s1 = p->point1;
   while (s1 && s1->next) { // lacots
      sa = s1;
      sb = s1->next;
      loop = false;
      end = false;
      while (!end) { // points
         if (sa->y != sb->y) { // actif
            if (sa->y < sb->y) {
               sx0 = sa->x;
               sy0 = sa->y;
               sx1 = sb->x;
               sy1 = sb->y;
            } else {
               sx0 = sb->x;
               sy0 = sb->y;
               sx1 = sa->x;
               sy1 = sa->y;
            }

            if (!(p->props & relative)) {
                sx0 -= v4p->xvu0;
                sy0 -= v4p->yvu0;
                sx1 -= v4p->xvu0;
                sy1 -= v4p->yvu0;
                if (v4p->scaling) {
                   sx0 = sx0 * v4p->divxvu + (sx0 * v4p->modxvu) / v4p->dxvu;
                   sy0 = sy0 * v4p->divyvu + (sy0 * v4p->modyvu) / v4p->dyvu;
                   sx1 = sx1 * v4p->divxvu + (sx1 * v4p->modxvu) / v4p->dxvu;
                   sy1 = sy1 * v4p->divyvu + (sy1 * v4p->modyvu) / v4p->dyvu;
               }
            }
            dx = sx1 - sx0;
            dy = sy1 - sy0;
            q = dx / dy ;
            r = dx > 0 ? dx % dy : (-dx) % dy ;
            // troncage haut
            if (sy0 < 0 && dy > 0) {
               sx0 -= sy0 * q + sy0 * (dx > 0 ? r : -r) / dy;
               sy0 = 0;
            }

            b = v4pEnableEdgeNew(p) ;
            b->x0 = sx0;
            b->y0 = sy0;
            b->x1 = sx1;
            b->y1 = sy1;
            if (dy > 0) {
               b->o1 = q ;
               b->r1 = r ;
               if (dx > 0) {
                  b->o2 = b->o1 + 1;
               } else {
                  b->o2 = b->o1 - 1;
               }
               b->r2 = b->r1 - dy;
            } else {
               b->o1 = b->o2 = b->r1 = b->r2 = 0 ;
            }

            // ActiveEdge dans list y
            ListAddData(v4p->activeEdgeYSortedList, b) ;

         } // actif
         if (loop) {
            s1 = NULL ;
            end = true ;
         } else if (sb->x == s1->x && sb->y == s1->y) {
            // neweau lacot
            s1 = sb->next ;
            end = true ;
         } else {
            sa = sb ;
            sb = sb->next ;
            if (!sb) {
               sb = s1 ;
               loop = true ;
            }
         }
      } //  points
   } // lacots
   return p ;
}

// compare minyv between 2 polygons
int comparePolygonMinyv(void *data1, void *data2) {
  PolygonP p1 = data1, p2 = data2 ;
  return (p1->minyv < p2->minyv) ;
}

// sort a polygon list ordered by 'miny'
List v4pSortPolygon(List list) {
   ListSetDataPrior(comparePolygonMinyv) ;
   return ListSort(list) ;
}

// compare maxyv between 2 polygons
int comparePolygonMaxyv(void *data1, void *data2) {
  PolygonP p1 = data1, p2 = data2 ;
  return (p1->maxyv < p2->maxyv) ;
}

// sort a poly list ordered by 'maxy'
List v4pSortOpenedPolygons(List list) {
   ListSetDataPrior(comparePolygonMaxyv) ;
   return ListSort(list) ;
}

// build a polygon list by following next & sub links
void v4pBuildOpenablePolygonsList(PolygonP l) {
   PolygonP p ;
   for (p = l ; p ; p = p->next) {
      if (p->props & disabled) continue ;

      if (!v4pIsVisible(p)) continue ;

      if (p->sub1) v4pBuildOpenablePolygonsList(p->sub1) ;

      ListAddData(v4p->openablePolygonsList, p) ;
   }
}

// called by v4pSortActiveEdge()
int compareActiveEdgeX(void *data1, void *data2) {
   ActiveEdgeP b1 = data1, b2 = data2 ;
   return (b1->x < b2->x) ;
}

// sort an ActiveEdge list ordered by 'x'
List v4pSortActiveEdge(List list) {
   ListSetDataPrior(compareActiveEdgeX) ;
   return ListSort(list) ;
}

// draw a scan-line slice depending on its polygon
Boolean v4pDrawSlice(int y, int x0, int x1, PolygonP p) {
   return v4pSliceCallBack(y, x0, x1, p ? p->color : bgColor) ;
}

// transform relative coordinates into absolute (scene related) ones
void v4pViewToAbsolute(Coord x, Coord y, Coord *xa, Coord *ya) {
   *xa = v4p->xvu0 + x * v4p->divxvub + x * v4p->modxvub / lineWidth ;
   *ya = v4p->yvu0 + y * v4p->divyvub + y * v4p->modyvub / lineNb ;
}

// open all polygons newly intersected by scan-line
List v4pOpenPolygons(int y) {
   PolygonP p ;
   List l ;
   Boolean new = false ;

   for (l = v4p->openablePolygonsList ; l && (p = (PolygonP)ListData(l))->miny <= y ; l = ListFree(l)) {
      if (p->maxyv < y) continue ;

      v4pPolygonBuildActiveEdgeList(p) ;
      new = true ;

      ListAddData(v4p->openedPolygonsList, p) ;
   }
   v4p->openablePolygonsList = l ;

   if (new) {
     v4p->activeEdgeYSortedList = v4pSortListActiveEdgey(v4p->activeEdgeYSortedList) ;
     v4p->openedPolygonsList = v4pSortOpenedPolygons(v4p->openedPolygonsList) ;
   }


   return v4p->openedPolygonsList ;
}

// open all new scan-line intersected ActiveEdge
Boolean v4pOpenActiveEdge(Coord y) {
   Boolean open = false ;
   List l ;
   ActiveEdgeP b ;
   for ( l = v4p->activeEdgeYSortedList ; l && (b = (ActiveEdgeP)ListData(l))->y0 <= y ; l = ListFree(l)) {
      if (b->y1 <= y) continue ;
      b->s = b->r2 - 1 ;
      b->x = b->x0 ;
      ListAddData(v4p->activeEdgeXSortedList, b) ;
      open = true ;
   }
   v4p->activeEdgeYSortedList = l ;
   return open ;
}
void dprintf(Coord x, Coord y, Char *formatString, ...) ;

// affiche une scene avec une vue
Boolean v4pAffiche() {
   List l, pl ;
   PolygonP p, polyVisible ;
   ActiveEdgeP b, pb ;
   Coord y, px ;
   ILayer z ;
   PolygonP layers[16] ;
   int zMax ;
   UInt16 bz, bi ; // bit-word of layers & collides
   UInt16 mz, mi ; // masques
   ICollide i, colli1, colli2 ;
   int nColli ;
   PolygonP pColli[16] ;

   v4pDisplayCallBack() ;

   // clean ActiveEdges
   v4p->activeEdgeYSortedList = NULL ; // sort ActiveEdge verticaly

   //openablePolygonsList gives the next polygon to open
   //openedPolygonsList gives the next opened polygon to close
   v4p->openablePolygonsList = NULL ;
   v4p->openedPolygonsList= NULL ;
   if (!v4p->scene) return failure;
   v4pBuildOpenablePolygonsList(*(v4p->scene)) ;
   v4p->openablePolygonsList = v4pSortPolygon(v4p->openablePolygonsList) ;
   v4pOpenPolygons(0) ;

   // list of opened ActiveEdges
   v4p->activeEdgeXSortedList = NULL ;
   v4pOpenActiveEdge(0) ;
   v4p->activeEdgeXSortedList = v4pSortActiveEdge(v4p->activeEdgeXSortedList) ;
   // scanline loop
   for (y = 0 ; y < lineNb ; y++) {

      // opened ActiveEdge loop
      l = v4p->activeEdgeXSortedList ;
      pl = NULL ;
      while (l) {
         b = (ActiveEdgeP)ListData(l) ;
         if (y >= b->y1) { // close ActiveEdge
            if (pl)
               ListSetNext(pl, l = ListFree(l)) ;
            else
               v4p->activeEdgeXSortedList = l = ListFree(l) ;
         } else { // shift ActiveEdge

            if (b->s > 0) {
               b->x+= b->o2 ;
               b->s+= b->r2 ;
            } else {
               b->x+= b->o1 ;
               b->s+= b->r1 ;
            }
            pl = l ;
            l = ListNext(l) ;
         }
      } // opened ActiveEdge loop

      //  polygon closing loop
      for (l = v4p->openedPolygonsList ;
           l && y > (p = (PolygonP)ListData(l))->maxyv ;
           v4pPolygonDelActiveEdges(p), l = ListFree(l)) ;
      v4p->openedPolygonsList = l ;

      // open newly intersected polygons
      v4pOpenPolygons(y) ;

      // open newly intersected ActiveEdge
      v4pOpenActiveEdge(y) ;

      // sort ActiveEdge
      v4p->activeEdgeXSortedList = v4pSortActiveEdge(v4p->activeEdgeXSortedList) ;

      // reset layers
      bz = 0 ;
      polyVisible = NULL ;
      zMax = -1 ;

      // reset collides
      bi = 0 ;
      nColli = 0 ;

      // loop among scanline slices
      px = 0 ;
      b = NULL ;
      for (l = v4p->activeEdgeXSortedList ; l ; l = ListNext(l)) { // loop ActiveEdge ouvert / x
          pb = b ;
          b = (ActiveEdgeP)ListData(l) ;
          if (b->x > 1000) dprintf(0,0, "problem %d %d %d", (int)b->x, (int)px, (pb ? pb->x : -1));
          if (b->x > 0) {
              //if (px > b->x) v4pErrorCallBack("pb slice %d %d %d", (int)y, (int)px, (int)b->x);
              if (y >= 0 && y < lineNb && px < lineWidth)
                 v4pDrawSlice(y, v4pMaxCoord(px, 0), v4pMinCoord(b->x, lineWidth), polyVisible);
              if (nColli>1)
                 v4pCollideCallBack(colli1, colli2, y, px, b->x, pColli[colli1], pColli[colli2]);
          }
          px = b->x;
          p = b->p;
          i = p->i;
          z = p->z & 15;
          mz = (UInt16)1 << z ;
          mi = (i == (ICollide)-1 ? (UInt16)0 : (UInt16)1 << (i & 15)) ;
          if ((bz ^= mz) & mz) {
             layers[z] = p ;
             if ((int)z > zMax) {
                zMax = z ;
                polyVisible = p ;
             }
             if (mi) {
                pColli[i] = p ;
                if (!(bi & mi)) {
                   bi |= mi ;
                   nColli++ ;
                   if (nColli == 1)
                      colli1 = i ;
                   else if (nColli == 2)
                      colli2 = i ;
                }
             }
          } else {
             if (z == zMax) {
                   zMax = floorLog2(bz) ;
                   //dprintf(0,120-5*zMax,"v%d",(int)(zMax));
                   polyVisible = (zMax>=0 ? layers[zMax] : NULL) ;
             }
             if (bi & mi) {
                bi ^= mi ;
                nColli-- ;
                if (nColli == 1 && i == colli1)
                   colli1 = colli2 ;
                else if (nColli == 2)
                   if (i == colli1)
                      colli1 = floorLog2(bi ^ (1 << colli2)) ;
                   else if (i == colli2)
                      colli2 = floorLog2(bi ^ (1 << colli1)) ;
             }
          }
       } // x opened ActiveEdge loop

       // last slice
       if (px < lineWidth)
          v4pDrawSlice(y, imax(0, px), lineWidth, NULL) ;

   } // y loop ;

   // clean lists
   l = v4p->openablePolygonsList ;
   while (l) l = ListFree(l) ;
   l = v4p->activeEdgeXSortedList ;
   while (l) l = ListFree(l) ;

   // close still opened polygons
   l = v4p->openedPolygonsList ;
   while (l) {
      v4pPolygonDelActiveEdges((PolygonP)ListData(l)) ;
      l = ListFree(l) ;
   }

   l = v4p->activeEdgeYSortedList ;
   while (l) l = ListFree(l) ;

   QuickHeapReset(v4p->activeEdgeHeap) ;

   v4pDisplayEndCallBack() ;
   return success ;
}

// Add 4 points as a rectangle
PolygonP v4pPolygonRect(PolygonP p, Coord x0, Coord y0, Coord x1, Coord y1) {
   v4pPolygonAddPoint(p, x0, y0) ;
   v4pPolygonAddPoint(p, x0, y1) ;
   v4pPolygonAddPoint(p, x1, y1) ;
   v4pPolygonAddPoint(p, x1, y0) ;
   return p ;
}

PolygonProps v4pPolygonEnable(PolygonP p) {
   return v4pPolygonRemoveProp(p, disabled) ;
}

PolygonProps v4pPolygonDisable(PolygonP p) {
   return v4pPolygonPutProp(p, disabled) ;
}

// Set a list of polygons as a scene
void v4pSetScene(PolygonP *scene) {
   v4p->scene = scene ;
}
