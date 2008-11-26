#ifndef V4PI
#define V4PI
/* V4P Implementation IF
**
*/

#include "v4p_ll.h"
#include "v4p.h"

extern const Color
   gray, marron, purple, green, cyan,
   black, red, blue, yellow, dark, oliver,
   fluo;

extern const Coord marginX, marginY, lineNb, lineWidth; // view size

typedef struct v4pDisplay_s* v4pDisplayP;

extern v4pDisplayP v4pDisplayDefaultContext;

#define V4P_QUALITY_LOW 0
#define V4P_QUALITY_NORMAL 1
#define V4P_QUALITY_HIGH 2

Boolean v4pDisplayInit(int quality, Boolean fullscreen);
v4pDisplayP v4pDisplayNewContext(int width, int height);
Boolean v4pDisplaySetContext(v4pDisplayP);
void    v4pDisplayFree();

Boolean v4pDisplayStart();
Boolean v4pDisplaySlice(Coord y, Coord x0, Coord x1, Color c);
Boolean v4pDisplayEnd();

void    v4pDisplayDebug(char *formatString,...);
Boolean v4pDisplayError(char *s,...);
Boolean v4pDisplayCollide(ICollide i1, ICollide i2, Coord py, Coord x1, Coord x2, PolygonP p1, PolygonP p2);

PolygonP v4pDecodePolygon(char *s);
char*    v4pEncodePolygon(PolygonP p);

void    v4pDisplayQuit();


#endif
