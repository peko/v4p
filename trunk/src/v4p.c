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
#define NEW_DEAL
#define NEW_DEAL_2

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
** v4pDisplayX : lower layer function call
** opened polygon : polygon intersected by the scan-line : min(y(points)) < y < max(y(points))
** to be opened polygon : y(scan-line) < min(y(points))
** closed polygon : y(scan-line) > max(y(points))
*/

#include "v4p.h"
#include <stdlib.h>
#include "lowmath.h"
#include "quickheap.h"
#include "sortable.h"
#include "v4pi.h"

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
 List ActiveEdge1 ; // ActiveEdges list
} Polygon ;

// type ActiveEdge
typedef struct ba_s {
 Coord x0, y0, x1, y1 ; // vector
 PolygonP p ; // polygone
 Coord x, o1, o2, s, h, r1, r2 ; // bresenham: offsets, sums, lineNb, remainings
} ActiveEdge ;

// contexte V4P
typedef struct v4pContext_s {
 PolygonP *scene ; // scene = a polygon set
 Coord   xvu0,yvu0,xvu1,yvu1 ; // view coordinates
 Color   bgColor;
 Coord step ; // step
 int debug1 ;
 QuickHeap pointHeap, polygonHeap, activeEdgeHeap ;
#ifndef NEW_DEAL_2
 List    openedPolygonsList, openablePolygonsList ; // polygons lists
#endif
 List    openedAEList ; // ActiveEdge lists
#ifdef NEW_DEAL
 List    relativeOpenableAEList, absoluteOpenableAEList ; // ActiveEdge lists
#ifdef NEW_DEAL_2
 List    currentRelativeOpenableAE, currentAbsoluteOpenableAE ; // ActiveEdge lists
#endif
#else
 List    openableAEList ;
#endif
 Coord   dxvu,dyvu, // view related data
  divxvu,modxvu,divxvub,modxvub,
  divyvu,modyvu,divyvub,modyvub;
 Boolean scaling; // do polygons need to be scaled?
 UInt32 changes;
} V4pContext ;

#define V4P_CHANGED_ABSOLUTE 1
#define V4P_CHANGED_RELATIVE 2
#define V4P_CHANGED_VIEW 4

static V4pContextP v4p = NULL ; // current (selected) v4p Context
V4pContextP v4pDefaultContext = NULL;

// define the current BG color
Color v4pSetBGColor(Color bg) {
  return v4p->bgColor=bg;
}

// define the current view
Boolean v4pSetView(Coord x0, Coord y0, Coord x1, Coord y1) {
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

// create a v4p context
V4pContextP v4pContextNew() {
  V4pContextP v4p = (V4pContextP)malloc(sizeof(V4pContext)) ;
  v4p->scene = NULL;
  v4p->step = 8;
  v4p->pointHeap = QuickHeapNewFor(Point) ;
  v4p->polygonHeap = QuickHeapNewFor(Polygon) ;
  v4p->activeEdgeHeap = QuickHeapNewFor(ActiveEdge) ;
#ifdef NEW_DEAL_2
   v4p->absoluteOpenableAEList = NULL ; // vertical sort
   v4p->relativeOpenableAEList = NULL ; // vertical sort
#endif
  v4p->xvu0=0;
  v4p->yvu0=0;
  v4p->xvu1=lineWidth;
  v4p->yvu1=lineNb;
  v4p->dxvu=lineWidth;
  v4p->dyvu=lineNb;
  v4p->divxvu = 1;
  v4p->modxvu = 0;
  v4p->divxvub = 1;
  v4p->modxvub = 0;
  v4p->divyvu = 1;
  v4p->modyvu = 0;
  v4p->divyvub = 1;
  v4p->modyvub = 0;
  v4p->scaling = 0;
  v4p->changes = 0;
  return v4p;
}

// select a v4p context
void v4pContextSet(V4pContextP p) {
  v4p = p;
}

// delete a v4p context
void v4pContextFree(V4pContextP p) {
  QuickHeapDelete(v4p->pointHeap) ;
  QuickHeapDelete(v4p->polygonHeap) ;
  QuickHeapDelete(v4p->activeEdgeHeap) ;
  free(p);
}

Boolean v4pInit() {
  if (!v4pDefaultContext) {
     v4pDefaultContext=v4pContextNew();
  }
  v4pContextSet(v4pDefaultContext);
  return success ;
}

void v4pQuit() {
  if (v4pDefaultContext) v4pContextFree(v4pDefaultContext);
  if (v4p == v4pDefaultContext) v4p = NULL;
  v4pDefaultContext = NULL;
}

// create a polygon
PolygonP v4pPolygonNew(PolygonProps t, Color col, ILayer z) {
  PolygonP p = QuickHeapAlloc(v4p->polygonHeap) ;
  p->props = t & ~V4P_CHANGED;
  p->z = z;
  p->i = (ICollide)-1;
  p->color = col;
  p->point1 = NULL;
  p->sub1 = NULL;
  p->next = NULL;
  p->miny = ~0; // miny = too much => limits to be computed
  p->ActiveEdge1 = NULL ;
  return p;
}

#define v4pPolygonChanged(P) ((P)->props|=V4P_CHANGED)

PolygonP v4pPolygonDelActiveEdges(PolygonP p);

// delete a poly (including its points and subs)
int v4pPolygonDel(PolygonP p) {
 v4pPolygonDelActiveEdges(p) ;
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
    if (!pl) return (v4pDisplayError("polygon lost"), failure) ;
    ppl->next = p->next;
  }
  p->next=NULL;
  return success ;
}


static v4pPolygonNotMoreInDisabled(PolygonP p) {
    v4pPolygonRemoveProp(p, V4P_IN_DISABLED) ;
    if (p->next)
      v4pPolygonNotMoreInDisabled(p->next) ;
    if (p->sub1 && !(p->props & V4P_DISABLED))
      v4pPolygonNotMoreInDisabled(p->sub1) ;
}

PolygonProps v4pPolygonEnable(PolygonP p) {
   if (!(p->props & V4P_DISABLED))
      return p->props ;

   if (p->sub1 && !(p->props & V4P_IN_DISABLED))
      v4pPolygonNotMoreInDisabled(p->sub1) ;

   return v4pPolygonRemoveProp(p, V4P_DISABLED) ;
}

static v4pPolygonInDisabled(PolygonP p) {
    v4pPolygonPutProp(p, V4P_IN_DISABLED) ;
    if (p->next)
      v4pPolygonInDisabled(p->next) ;
    if (p->sub1)
      v4pPolygonInDisabled(p->sub1) ;
}

PolygonProps v4pPolygonDisable(PolygonP p) {
   if (p->props & V4P_DISABLED)
      return p->props ;
   if (p->sub1)
      v4pPolygonInDisabled(p->sub1) ;
   return v4pPolygonPutProp(p, V4P_DISABLED) ;
}


// Add a polygon to an other polygon subs list
PolygonP v4pPolygonAddSub(PolygonP parent, PolygonP p) {
  if (parent->props & (V4P_DISABLED | V4P_IN_DISABLED))
      v4pPolygonInDisabled(p) ;
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
  v4pPolygonChanged(p) ;
  return (p->props |= i) ;
}

// remove a polygon property flag
PolygonProps v4pPolygonRemoveProp(PolygonP p, PolygonProps i) {
  v4pPolygonChanged(p) ;
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
   v4pPolygonChanged(p) ;
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
   v4pPolygonChanged(p) ;
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

   v4pPolygonChanged(p) ;
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
        if (!s) { v4pDisplayError("full Heap") ; return NULL ; }
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

// create an ActiveEdge of a polygon
ActiveEdgeP v4pActiveEdgeNew(PolygonP p) {
   ActiveEdgeP b = QuickHeapAlloc(v4p->activeEdgeHeap) ;
   b->p = p;
   ListAddData(p->ActiveEdge1, b);
   return b ;
}

// delete all ActiveEdges of a poly
PolygonP v4pPolygonDelActiveEdges(PolygonP p) {
   List l ;
   ActiveEdgeP b, s;
   l = p->ActiveEdge1 ;
   while (l) {
      b = (ActiveEdgeP)ListData(l) ;
#ifdef NEW_DEAL_2
      // we remove it only while scanning openable AE list
      b->p = NULL ;
#else
      QuickHeapFree(v4p->activeEdgeHeap, b) ;
#endif
      l = ListFree(l) ;
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
   // c changed
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

// compute the minimal rectangle surrounding a polygon
PolygonP v4pPolygonComputeLimits(PolygonP p) {
   PointP s = p->point1;
   if (!s) { // no point
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

// transform relative coordinates into absolute (scene related) ones
void v4pViewToAbsolute(Coord x, Coord y, Coord *xa, Coord *ya) {
   *xa = v4p->xvu0 + x * v4p->divxvub + x * v4p->modxvub / lineWidth ;
   *ya = v4p->yvu0 + y * v4p->divyvub + y * v4p->modyvub / lineNb ;
}

// transform absolute coordinates into relative (scene related) ones
void v4pAbsoluteToView(Coord x, Coord y, Coord *xa, Coord *ya) {
   x -= v4p->xvu0 ;
   y -= v4p->yvu0 ;
   if (v4p->scaling) {
     *xa = x * v4p->divxvu + x * v4p->modxvu / v4p->dxvu ;
     *ya = y * v4p->divyvu + y * v4p->modyvu / v4p->dyvu ;
    } else {
     *xa = x ;
     *ya = y ;
    }
}

// return false if polygon is located out of the view area
Boolean v4pIsVisible(PolygonP p) {
   if (!p->point1)
      return false ;
   if (p->miny == ~0) // unknown limits
      v4pPolygonComputeLimits(p) ;

   { Coord minx = p->minx, maxx = p->maxx, miny = p->miny, maxy = p->maxy;

   if (!(p->props & relative)) {
      v4pAbsoluteToView(minx, miny, &minx, &miny) ;
      v4pAbsoluteToView(maxx, maxy, &maxx, &maxy) ;
   }
   p->minyv = miny ;
   p->maxyv = maxy ;
   return (maxx >= 0 && maxy >= 0 && minx < lineWidth && miny < lineNb) ;
   }
}

// clone a list
List ListClone(List list) {
   List l = list, clone = NULL, n = NULL, p = NULL ;

   // sorting break search loop
   if (l)
      {
      clone = n = ListNew() ;
      ListSetData(n, ListData(l)) ;
      for (l = ListNext(l) ; l ; l = ListNext(l))
        {
        p = n ;
        n = ListNew() ;
        if (p) ListSetNext(p, n) ;
        ListSetData(n, ListData(l)) ;
        }
       ListSetNext(n, NULL) ;
       }
  return clone ;
}

// build a list of ActiveEdges for a given polygon
PolygonP v4pPolygonBuildActiveEdgeList(PolygonP p) {
   PointP sa, sb, s1;
   Boolean loop, end;
   int dx, dy, q, r;
   Coord sx0, sx1, sy0, sy1;
   ActiveEdgeP b ;

#ifdef NEW_DEAL
   if (!(p->props & V4P_CHANGED))
      {
#ifdef NEW_DEAL_2
      // NOTHING TO DO
#else
      // ActiveEdge dans list y
      if (p->props & relative)
         v4p->relativeOpenableAEList = ListMerge(v4p->relativeOpenableAEList, ListClone(p->ActiveEdge1)) ;
      else
         v4p->absoluteOpenableAEList = ListMerge(v4p->absoluteOpenableAEList, ListClone(p->ActiveEdge1)) ;
#endif
      return p ;
      }
   v4p->changes |= (p->props & relative) ? V4P_CHANGED_RELATIVE : V4P_CHANGED_ABSOLUTE ;
   if (p->ActiveEdge1)
      // change => need to recompile AE
      v4pPolygonDelActiveEdges(p) ;
#endif
   p->ActiveEdge1 = NULL ;
   // Mmmm: autrefois, j'ai retir� v4pIsVisible(p) mais pourquoi?
   // !v4pIsVisible(p) || 
   if ((p->props & (V4P_DISABLED | V4P_IN_DISABLED | invisible)))
      return p;

   s1 = p->point1;
   while (s1 && s1->next) { // lacots
      sa = s1;
      sb = s1->next;
      loop = false;
      end = false;
      while (!end) { // points
         if (sa->y != sb->y) { // active


            b = v4pActiveEdgeNew(p) ;

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

#ifndef NEW_DEAL
            if (!(p->props & relative)) {
                v4pAbsoluteToView(sx0, sy0, &sx0, &sy0) ;
                v4pAbsoluteToView(sx1, sy1, &sx1, &sy1) ;
            }
#endif

            b->x0 = sx0;
            b->y0 = sy0;
            b->x1 = sx1;
            b->y1 = sy1;


#ifdef NEW_DEAL
         if (p->props & relative) {
#endif
            dx = sx1 - sx0;
            dy = sy1 - sy0;
            q = dx / dy ;
            r = dx > 0 ? dx % dy : (-dx) % dy ;
            b->o1 = q ;
            b->o2 = b->o1 + (dx > 0 ? 1 : -1) ;
            b->r1 = r ;
            b->r2 = r - dy ;
#ifdef NEW_DEAL
          } // relative polygon
#endif


#ifdef NEW_DEAL
            if (p->props & relative)
               { ListAddData(v4p->relativeOpenableAEList, b) ; }
            else
               { ListAddData(v4p->absoluteOpenableAEList, b) ; }
#else
            // ActiveEdge dans list y
            ListAddData(v4p->openableAEList, b) ;
#endif
         } // active
         if (loop) {
            s1 = NULL ;
            end = true ;
         } else if (sb->x == s1->x && sb->y == s1->y) {
            // new lacot
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
#ifdef NEW_DEAL
   p->props &= ~V4P_CHANGED ; // remove the flag
#endif
   return p ;
}

#ifdef NEW_DEAL_2
#else
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
#endif


#ifdef NEW_DEAL_2
#else
// build a polygon list by following next & sub links
void v4pBuildOpenablePolygonsList(PolygonP l) {
   PolygonP p ;
   for (p = l ; p ; p = p->next) {
      if (p->props & (V4P_DISABLED | V4P_IN_DISABLED)) continue ;

      if (!v4pIsVisible(p)) continue ;

      if (p->sub1) v4pBuildOpenablePolygonsList(p->sub1) ;

      ListAddData(v4p->openablePolygonsList, p) ;
   }
}
#endif


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
  return v4pDisplaySlice(y, x0, x1, p ? p->color : v4p->bgColor) ;
}

#ifdef NEW_DEAL_2
// build AE lists
void v4pBuildOpenableAELists(PolygonP l) {
   PolygonP p ;
   for (p = l ; p ; p = p->next) {
      //if (p->props & (V4P_DISABLED | V4P_IN_DISABLED | invisible) continue ;

      //if (!v4pIsVisible(p)) continue ;

      v4pPolygonBuildActiveEdgeList(p) ;

      if (p->sub1) v4pBuildOpenableAELists(p->sub1) ;
   }
}
#else
// open all polygons newly intersected by scan-line
List v4pOpenPolygons(int y) {
   PolygonP p ;
   List l ;
   Boolean new_relative = false, new_absolute = false ;

   for (l = v4p->openablePolygonsList ; l && (p = (PolygonP)ListData(l))->miny <= y ; l = ListFree(l)) {
      if (p->maxyv < y) continue ;

      v4pPolygonBuildActiveEdgeList(p) ;
      if (p->props & relative)
        new_relative = true ;
      else
        new_absolute = true ;

      ListAddData(v4p->openedPolygonsList, p) ;
   }
   v4p->openablePolygonsList = l ;

   if (new_relative || new_absolute) {
#ifdef NEW_DEAL
     if (new_relative)
        v4p->relativeOpenableAEList = v4pSortListActiveEdgey(v4p->relativeOpenableAEList) ;
     if (new_absolute)
        v4p->absoluteOpenableAEList = v4pSortListActiveEdgey(v4p->absoluteOpenableAEList) ;
#else
     v4p->openableAEList = v4pSortListActiveEdgey(v4p->openableAEList) ;
#endif
     v4p->openedPolygonsList = v4pSortOpenedPolygons(v4p->openedPolygonsList) ;
   }


   return v4p->openedPolygonsList ;
}
#endif

// open all new scan-line intersected ActiveEdge
#ifdef NEW_DEAL
Boolean v4pOpenActiveEdge(Coord yl, Coord yu) {
#else
Boolean v4pOpenActiveEdge(Coord y) {
#endif
   Boolean open = false ;
   List p, l ;
   ActiveEdgeP b ;
#ifdef NEW_DEAL
   Coord xr0, yr0, xr1, yr1, dx, dy, q, r ;

#ifdef NEW_DEAL_2
   for ( p = NULL, l = v4p->currentAbsoluteOpenableAE ; l ; l = (p ? ListNext(p) : v4p->absoluteOpenableAEList)) {
      b = (ActiveEdgeP)ListData(l) ;
      if (!b->p) {
         if (p)
           ListSetNext(p, ListNext(l)) ;
         else
            v4p->absoluteOpenableAEList = ListNext(l) ;
         QuickHeapFree(v4p->activeEdgeHeap, b) ;
         ListFree(l) ;
         continue ;
      } else
         p = l ;
      if (b->y0 > yu) break ;
#else
   for ( l = v4p->absoluteOpenableAEList ; l && (b = (ActiveEdgeP)ListData(l))->y0 <= yu ; l = ListFree(l)) {
#endif
      if (b->y1 <= yu) continue ;
      v4pAbsoluteToView(b->x0, b->y0, &xr0, &yr0) ;
      v4pAbsoluteToView(b->x1, b->y1, &xr1, &yr1) ;
      dx = xr1 - xr0;
      dy = yr1 - yr0;
      q = dx / dy ;
      r = dx > 0 ? dx % dy : (-dx) % dy ;
      b->o1 = q ;
      b->o2 = b->o1 + (dx > 0 ? 1 : -1) ;
      b->r1 = r ;
      b->r2 = r - dy ;
      b->s = -dy + r ;
      if (yr0 < yl) { // top crop needed
        int dy2 = yl - yr0 ;
        xr0 += dy2 * q + dy2 * (dx > 0 ? r : -r) / dy ;
        b->s += (dy2 * r) % dy ;
      }
      b->h = yr1 - yl - 1 ;
      b->x = xr0 ;
      ListAddData(v4p->openedAEList, b) ;
      open = true ;
   }
#ifdef NEW_DEAL_2
   v4p->currentAbsoluteOpenableAE = l ;
#else
   v4p->absoluteOpenableAEList = l ;
#endif

#ifdef NEW_DEAL_2
   for ( p = NULL, l = v4p->currentRelativeOpenableAE ; l  ; l = (p ? ListNext(p) : v4p->relativeOpenableAEList)) {
      b = (ActiveEdgeP)ListData(l) ;
      if (!b->p) {
         if (p)
           ListSetNext(p, ListNext(l)) ;
         else
            v4p->relativeOpenableAEList = ListNext(l) ;
         QuickHeapFree(v4p->activeEdgeHeap, b) ;
         ListFree(l) ;
         continue ;
      } else
         p = l ;
      if (b->y0 > yl) break ;
#else
   for ( l = v4p->relativeOpenableAEList ; l && (b = (ActiveEdgeP)ListData(l))->y0 <= yl ; l = ListFree(l)) {
#endif
      if (b->y1 <= yl) continue ;
      b->s = b->r2 - b->r1 ;
      b->x = b->x0 ;
      b->h = b->y1 - yl ;
      if (b->y0 < yl) { // top crop needed
        int dy2 = yl -  b->y0 ;
        b->x += dy2 * b->o1 + dy2 * (b->o2 >= 0 ? b->r1 : -b->r1) / dy ;
        b->s += (dy2 * b->r1) % dy ;
      }
      ListAddData(v4p->openedAEList, b) ;
      open = true ;
   }
#ifdef NEW_DEAL_2
   v4p->currentRelativeOpenableAE = l ;
#else
   v4p->relativeOpenableAEList = l ;
#endif
#else
   for ( l = v4p->openableAEList ; l && (b = (ActiveEdgeP)ListData(l))->y0 <= y ; l = ListFree(l)) {
      int dy ;
      if (b->y1 <= y) continue ;
      dy = b->y1 - b->y0 ;
      b->s = -dy ;
      b->x = b->x0 ;
      b->h = b->y1 - y ;
      if (b->y0 < y) { // top crop needed
        int dy2 = y - b->y0 ;
        b->x += dy2 * b->o1 + dy2 * (b->o2 > 0 ? b->r1 : -b->r1) / dy ;
        b->s += (dy2 * b->r1) % dy ;
      }
      ListAddData(v4p->openedAEList, b) ;
      open = true ;
   }
   v4p->openableAEList = l ;
#endif
   return open ;
}

// Render a scene
Boolean v4pRender() {
   List l, pl ;
   PolygonP p, polyVisible ;
   ActiveEdgeP b, pb ;
   Coord y, px, px_collide ;
#ifdef NEW_DEAL
   Coord yu, ou1, ou2, su, ru1, ru2 ;
#endif
   ILayer z ;
   PolygonP layers[16] ;
   int zMax ;
   UInt16 bz, bi ; // bit-word of layers & collides
   UInt16 mz, mi ; // masques
   ICollide i, colli1, colli2 ;
   int nColli ;
   PolygonP pColli[16] ;
   Boolean sortNeeded ;

   v4pDisplayStart() ;

   // clean ActiveEdges
#ifdef NEW_DEAL
#ifdef NEW_DEAL_2
#else
   v4p->absoluteOpenableAEList = NULL ; // vertical sort
   v4p->relativeOpenableAEList = NULL ; // vertical sort
#endif
#else
   v4p->openableAEList = NULL ; // sort ActiveEdge verticaly
#endif

#ifndef NEW_DEAL_2
   //openablePolygonsList gives the next polygon to open
   //openedPolygonsList gives the next opened polygon to close
   v4p->openablePolygonsList = NULL ;
   v4p->openedPolygonsList= NULL ;
#endif

   if (!v4p->scene) return failure;
#ifdef NEW_DEAL_2
   v4pBuildOpenableAELists(*(v4p->scene)) ;
   if (v4p->changes & V4P_CHANGED_ABSOLUTE) v4p->absoluteOpenableAEList = v4pSortListActiveEdgey(v4p->absoluteOpenableAEList) ;
   if (v4p->changes & V4P_CHANGED_RELATIVE) v4p->relativeOpenableAEList = v4pSortListActiveEdgey(v4p->relativeOpenableAEList) ;
#else
   v4pBuildOpenablePolygonsList(*(v4p->scene)) ;
   v4p->openablePolygonsList = v4pSortPolygon(v4p->openablePolygonsList) ;
#endif

   // list of opened ActiveEdges
   v4p->openedAEList = NULL ;
#ifdef NEW_DEAL
   yu = v4p->yvu0 ;
   ou1 = v4p->divyvub ;
   ou2 = v4p->divyvub + 1 ;
   su = -v4p->dyvu ;
   ru1 = v4p->modyvub ;
   ru2 = v4p->modyvub - v4p->dyvu ;
#ifdef NEW_DEAL_2
   v4p->currentAbsoluteOpenableAE = v4p->absoluteOpenableAEList ;
   v4p->currentRelativeOpenableAE = v4p->relativeOpenableAEList ;
#endif
#endif

   // scan-line loop
   for (y = 0 ; y < lineNb ; y++) {
#ifdef NEW_DEAL
      if (su >= 0) {
         su+= ru2 ;
         yu+= ou2 ;
      } else {
         su+= ru1 ;
         yu+= ou1 ;
      }
#endif
      sortNeeded = false ;

      // opened ActiveEdge loop
      l = v4p->openedAEList ;
      pl = NULL ;
      px = -(0x7FFF) ; // not sure its really the min, but we dont care
      while (l) {
         if (!--((b = (ActiveEdgeP)ListData(l))->h)) { // close ActiveEdge
            if (pl)
               ListSetNext(pl, l = ListFree(l)) ;
            else
               v4p->openedAEList = l = ListFree(l) ;
         } else { // shift ActiveEdge
            int x ;
            if (b->o2) {
              if (b->s > 0) {
                x = b->x+= b->o2 ;
                b->s+= b->r2 ;
              } else {
                x = b->x+= b->o1 ;
                b->s+= b->r1 ;
              }
            } else
              x = b->x ;

            if (x < px) sortNeeded = true ;
            px = x ;
            pl = l ;
            l = ListNext(l) ;
         }
      } // opened ActiveEdge loop

#ifndef NEW_DEAL_2
      //  polygon closing loop
#ifdef NEW_DEAL
      for (l = v4p->openedPolygonsList ;
           l && y > (p = (PolygonP)ListData(l))->maxyv ; l = ListFree(l)) ;
#else
      for (l = v4p->openedPolygonsList ;
           l && y > (p = (PolygonP)ListData(l))->maxyv ;
           v4pPolygonDelActiveEdges(p), l = ListFree(l)) ;
#endif
      v4p->openedPolygonsList = l ;
#endif

#ifndef NEW_DEAL_2
      // open newly intersected polygons
      v4pOpenPolygons(y) ;
#endif

      // open newly intersected ActiveEdge
#ifdef NEW_DEAL
      sortNeeded |= v4pOpenActiveEdge(y, yu) ;
#else
      sortNeeded |= v4pOpenActiveEdge(y) ;
#endif

      // sort ActiveEdge
      if (sortNeeded)
         v4p->openedAEList = v4pSortActiveEdge(v4p->openedAEList) ;

      // reset layers
      bz = 0 ;
      polyVisible = NULL ;
      zMax = -1 ;

      // reset collides
      bi = 0 ;
      nColli = 0 ;

      // loop among scanline slices
      px = px_collide = 0 ;
      for (l = v4p->openedAEList ; l ; l = ListNext(l)) { // loop ActiveEdge ouvert / x
          // pb = b ;
          b = (ActiveEdgeP)ListData(l) ;
          p = b->p ;
          z = p->z & 15 ;
          bz ^= ((UInt16)1 << z) ;
          //if (b->x > 1000) v4pDisplayDebug("problem %d %d %d", (int)b->x, (int)px, (pb ? pb->x : -1));
          //if (px > b->x) v4pDisplayError("pb slice %d %d %d", (int)y, (int)px, (int)b->x);
          if ((int)z >= zMax) {
            if (px < lineWidth && b->x > 0)
               v4pDrawSlice(y, imax(px, 0), imin(b->x, lineWidth), polyVisible);
            px = b->x ;
            if ((int)z > zMax) {
               polyVisible = layers[z] = p ;
               zMax = z ;
            } else { // z == zMax
               zMax = floorLog2(bz) ;
               polyVisible = (zMax >= 0 ? layers[zMax] : NULL) ;
            }
          } else { // z < zMax
            layers[z] = p ;
          }
          if (nColli > 1)
             v4pDisplayCollide(colli1, colli2, y, px_collide, b->x, pColli[colli1], pColli[colli2]);
          px_collide = b->x;
          i = p->i ;
          mi = (i == (ICollide)-1 ? (UInt16)0 : (UInt16)1 << (i & 15)) ;
          if (layers[z]) {
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
          v4pDrawSlice(y, imax(0, px), lineWidth, polyVisible) ;

   } // y loop ;

#ifndef NEW_DEAL_2
   // clean lists
   l = v4p->openablePolygonsList ;
   while (l) l = ListFree(l) ;
   v4p->openablePolygonsList = NULL ;
#endif

   l = v4p->openedAEList ;
   while (l) l = ListFree(l) ;
   v4p->openedAEList = NULL ;

#ifndef NEW_DEAL_2
   // close still opened polygons
   l = v4p->openedPolygonsList ;
   while (l) {
#ifndef NEW_DEAL
      v4pPolygonDelActiveEdges((PolygonP)ListData(l)) ;
#endif
      l = ListFree(l) ;
   }
   l = v4p->openedPolygonsList = NULL ;
#endif

#ifdef NEW_DEAL
#ifndef NEW_DEAL_2
   l = v4p->absoluteOpenableAEList ;
   while (l) l = ListFree(l) ;
   v4p->absoluteOpenableAEList = NULL ;

   l = v4p->relativeOpenableAEList ;
   while (l) l = ListFree(l) ;
   v4p->relativeOpenableAEList = NULL ;
#endif
#else
   l = v4p->openableAEList ;
   while (l) l = ListFree(l) ;
   v4p->openableAEList = NULL ;

   QuickHeapReset(v4p->activeEdgeHeap) ;
#endif

   v4p->changes = 0 ;
   v4pDisplayEnd() ;
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

// Set a list of polygons as a scene
void v4pSetScene(PolygonP *scene) {
   v4p->scene = scene ;
}
