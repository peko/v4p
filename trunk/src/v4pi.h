#ifndef V4PI
#define V4PI
#include "v4p_ll.h"

/* V4P Implementation
**
*/

// 0..215 combination of
//    ff  cc  99  66  33  00
// B : 0   6  12 108 114 120
// R : 0  18  36  54  72  90
// G : 0   1   2   3  4    5
// e.g  12+54+5 = $99 B + $66 R +$0 G
// 216..224 = gray 22 44 55 77 88 aa bb dd
// 225..229 = specials: light gray,marron,purple,green,cyan
// 230..255 = unused (black)
#include "v4p.h"
extern Color
   gray, marron, purple, green, cyan,
   black, red, blue, yellow, dark, oliver,
   fluo;
extern Coord
   screenWidth, screenHeight, marginX, marginY,
   bytesBetweenLines, lineWidth, lineNb;

typedef struct collide_s {
   Coord x ;
   Coord y ;
   UInt16 q ;
   PolygonP poly ;
} Collide ;

Collide collides[16] ;

extern Coord lineNb, lineWidth ; // view size
extern Color    bgColor ; // background color

Boolean v4pDisplayCallBack() ;
Boolean v4pErrorCallBack(char *s,...) ;
Boolean v4pSliceCallBack(Coord y, Coord x0, Coord x1, Color c) ;
Boolean v4pCollideCallBack(ICollide i1, ICollide i2, Coord py, Coord x1, Coord x2, PolygonP p1, PolygonP p2) ;
Boolean v4pDisplayEndCallBack() ;

PolygonP v4pDecodePolygon(char *s) ;
char *v4pEncodePolygon(PolygonP p) ;

Boolean v4pFlavorInit(Color background) ;

//helpers
void dprintf(Coord x, Coord y, Char *formatString, ...) ;

#endif
