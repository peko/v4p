/* V4P Implementation
**
*/
#include "v4pi.h"
#include <stdlib.h>
#include "lowmath.h"
#include "v4p.h"

Couleur
   gray=225, marron=226, purple=227, green=228, cyan=229,
   black=215, red=125, blue=95, yellow=120, dark=217, oliver=58,
   fluo=48;
Coord
   largeurEcran=160, hauteurEcran=160, margeX=10, margeY=10,
   octetsEntreLigne=2*10, largeurLigne = 160-2*10, nbLignes = 160-2*10;

Couleur couleurDeFond = 0 ;


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


Boolean v4pErreurRappel(char *formatString, ...) {
  va_list args ; Char text[0x100] ;
  va_start(args, formatString) ;
  StrVPrintF(text, formatString, args) ;
  va_end(args);
  WinDrawChars(text, StrLen(text), 0, 0) ;
}

Boolean v4pCollisionRappel(ICollision i1, ICollision i2, Coord py, Coord x1, Coord x2, PolyP p1, PolyP p2) {
 int l, dx, dy ;
 l = x2 - x1 ;
 dx = x1 * l + (l + 1) * l / 2 ;
 dy = l * py ;
 collisions[i1].q += l ;
 collisions[i1].x += dx ;
 collisions[i1].y += dy ;
 collisions[i1].poly = p2 ;
 collisions[i2].q += l ;
 collisions[i2].x += dx ;
 collisions[i2].y += dy ;
 collisions[i2].poly = p1 ;
 return success ;
}

Boolean v4pAffichageRappel() {
  int i ;

  iBuffer = margeY * largeurEcran + margeX ;

  //Init collisions
  for (i = 0 ; i < 16 ; i++) {
    collisions[i].q = 0 ;
    collisions[i].x = 0 ;
    collisions[i].y = 0 ;
    collisions[i].poly = NULL ;
  }
  return success ;
}

Boolean v4pAffichageFinRappel() {
   int i ;

   // Bilan des collisions
   for (i = 0 ; i < 16 ; i++) {
      if (!collisions[i].q) continue ;
      collisions[i].x /= collisions[i].q ;
      collisions[i].y /= collisions[i].q ;
   }
   return success ;
}

Boolean v4pTranconRappel(Coord y, Coord x0, Coord x1, Couleur c) {
 int l ;
 if (x1 <= x0) return success ;
 l = x1 - x0 ;
 WinSetForeColor((IndexedColorType)c);
 WinDrawLine(x0 + 10, y+10, x1+9, y+10);
 //MemSet(&buffer[iBuffer], l, (char)c) ;
 //iBuffer+= l ;
 //if (x1 == largeurLigne)
 //   iBuffer+= octetsEntreLigne ;

 return success ;
}

extern int v4pXToD(char c) ;
PolyP v4pDecodePoly(char *s) {
   ICouche z ;
   PolyProps t ;
   Couleur col ;
   PolyP p ;
   int i = 0 ;
   if (strlen(s) < 6) return NULL ;
   t = v4pXToD(s[i]) << 4 + v4pXToD(s[i + 1]) ;
   i = 2 ;
   col = v4pXToD(s[i]) << 4 + v4pXToD(s[i + 1]) ;
   i = 4 ;
   z = v4pXToD(s[i]) << 4 + v4pXToD(s[i + 1]) ;
   p = v4pPolyCree(t, col, z) ;
   return v4pDecodeSommets(p, s + 6) ;
}

char *v4pEncodePoly(PolyP p) {
  const char *t = "0123456789ABCDEF" ;
  UInt16 i, v ;
  char *s, *ss, *sss ;

  s = (char *)malloc(7) ;
  ss = s ;
  for (i = 0 ; i <= 2 ; i++) {
   if (i == 0) v = v4pPolyPoseProp(p,0) ;
   else if (i == 1) v = v4pPolyDonneCouleur(p) ;
   else if (i == 2) v = v4pPolyDonneZ(p) ;
   *ss = t[v & 15] ;
   ss++ ;
   v = v >> 4 ;
   *ss = t[v & 15] ;
   ss++ ;
  }
  *ss = '\0' ;

  sss = v4pEncodeSommets(p);
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

Boolean v4pFlavorInit(Couleur fond) {
  couleurDeFond=fond;
  buffer = BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
  v4pInit();
}
