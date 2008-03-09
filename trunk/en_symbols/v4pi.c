/* V4P Implementation
**
*/
#include "v4pi.h"
#include <stdlib.h>
#include "lowmath.h"
#include "v4p.h"

Color
   gray=225, marron=226, purple=227, green=228, cyan=229,
   black=215, red=125, blue=95, yellow=120, dark=217, oliver=58,
   fluo=48;
Coord
   screenWidth=160, screenHeight=160, marginX=10, marginY=10,
   bytesBetweenLines=2*10, lineWidth = 160-2*10, lineNb = 160-2*10;

Color bgColor = 0 ;


static char *buffer=NULL ;

static int iBuffer ;

void debug(char *s) {
  WinDrawChars(s,StrLen(s),0,0);
}

void idebug(int i) {
  Char s[16] ;
  StrPrintF(s,"%3d",i);
  WinDrawChars(s,StrLen(s),0,0);
}
#include <stdarg.h>
void dprintf(Coord x, Coord y, Char *formatString, ...)
{ va_list args ; Char text[0x100] ;
  va_start(args, formatString) ;
  StrVPrintF(text, formatString, args) ;
  va_end(args);
  WinDrawChars(text, StrLen(text), x, y) ;
}


Boolean v4pErrorCallBack(char *formatString, ...) {
  va_list args ; Char text[0x100] ;
  va_start(args, formatString) ;
  StrVPrintF(text, formatString, args) ;
  va_end(args);
  WinDrawChars(text, StrLen(text), 0, 0) ;
}

Boolean v4pCollideCallBack(ICollide i1, ICollide i2, Coord py, Coord x1, Coord x2, PolygonP p1, PolygonP p2) {
 int l, dx, dy ;
 l = x2 - x1 ;
 dx = x1 * l + (l + 1) * l / 2 ;
 dy = l * py ;
 collides[i1].q += l ;
 collides[i1].x += dx ;
 collides[i1].y += dy ;
 collides[i1].poly = p2 ;
 collides[i2].q += l ;
 collides[i2].x += dx ;
 collides[i2].y += dy ;
 collides[i2].poly = p1 ;
 return success ;
}

Boolean v4pDisplayCallBack() {
  int i ;

  iBuffer = marginY * screenWidth + marginX ;

  //Init collides
  for (i = 0 ; i < 16 ; i++) {
    collides[i].q = 0 ;
    collides[i].x = 0 ;
    collides[i].y = 0 ;
    collides[i].poly = NULL ;
  }
  return success ;
}

Boolean v4pDisplayEndCallBack() {
   int i ;

   // Bilan des collides
   for (i = 0 ; i < 16 ; i++) {
      if (!collides[i].q) continue ;
      collides[i].x /= collides[i].q ;
      collides[i].y /= collides[i].q ;
   }
   return success ;
}

Boolean v4pSliceCallBack(Coord y, Coord x0, Coord x1, Color c) {
 int l ;
 if (x1 <= x0) return success ;
 l = x1 - x0 ;
 WinSetForeColor((IndexedColorType)c);
 WinDrawLine(x0 + 10, y+10, x1+9, y+10);
 //MemSet(&buffer[iBuffer], l, (char)c) ;
 //iBuffer+= l ;
 //if (x1 == lineWidth)
 //   iBuffer+= bytesBetweenLines ;

 return success ;
}

extern int v4pXToD(char c) ;
PolygonP v4pDecodePolygon(char *s) {
   ILayer z ;
   PolygonProps t ;
   Color col ;
   PolygonP p ;
   int i = 0 ;
   if (strlen(s) < 6) return NULL ;
   t = v4pXToD(s[i]) << 4 + v4pXToD(s[i + 1]) ;
   i = 2 ;
   col = v4pXToD(s[i]) << 4 + v4pXToD(s[i + 1]) ;
   i = 4 ;
   z = v4pXToD(s[i]) << 4 + v4pXToD(s[i + 1]) ;
   p = v4pPolygonNew(t, col, z) ;
   return v4pPolygonDecodePoints(p, s + 6) ;
}

char *v4pEncodePolygon(PolygonP p) {
  const char *t = "0123456789ABCDEF" ;
  UInt16 i, v ;
  char *s, *ss, *sss ;

  s = (char *)malloc(7) ;
  ss = s ;
  for (i = 0 ; i <= 2 ; i++) {
   if (i == 0) v = v4pPolygonPutProp(p,0) ;
   else if (i == 1) v = v4pPolygonGetColor(p) ;
   else if (i == 2) v = v4pPolygonGetZ(p) ;
   *ss = t[v & 15] ;
   ss++ ;
   v = v >> 4 ;
   *ss = t[v & 15] ;
   ss++ ;
  }
  *ss = '\0' ;

  sss = v4pPolygonEncodePoints(p);
  if (!sss) {
     free(s) ;
     return NULL ;
  } else {
     s = (char *)realloc(s, 7 + strlen(sss)) ;
     StrCopy(ss, sss) ;
     free(sss) ;
     return s ;
  }
}

Boolean v4pFlavorInit(Color background) {
  bgColor=background;
  buffer = BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
  v4pInit();
}
