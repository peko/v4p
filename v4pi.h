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
extern Couleur
   gray, marron, purple, green, cyan,
   black, red, blue, yellow, dark, oliver,
   fluo;
extern Coord
   largeurEcran, hauteurEcran, margeX, margeY,
   octetsEntreLigne, largeurLigne, nbLignes;

typedef struct collision_s {
   Coord x ;
   Coord y ;
   UInt16 q ;
   PolyP poly ;
} Collision ;

Collision collisions[16] ;

extern Coord nbLignes, largeurLigne ; // taille zone d'affichage
extern Couleur    couleurDeFond ; // couleur du fond de la zone

Boolean v4pAffichageRappel() ;
Boolean v4pErreurRappel(char *s,...) ;
Boolean v4pTranconRappel(Coord y, Coord x0, Coord x1, Couleur c) ;
Boolean v4pCollisionRappel(ICollision i1, ICollision i2, Coord py, Coord x1, Coord x2, PolyP p1, PolyP p2) ;
Boolean v4pAffichageFinRappel() ;

PolyP v4pDecodePoly(char *s) ;
char *v4pEncodePoly(PolyP p) ;

Boolean v4pFlavorInit(Couleur fond) ;

//helpers
void dprintf(Coord x, Coord y, Char *formatString, ...) ;

#endif
