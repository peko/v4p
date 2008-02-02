#ifndef LOWMATH
#define LOWMATH
#include "v4p_ll.h"
extern int cosa ;
extern int sina ;
int bitMax(UInt16 bz) ;

int bitMax32(UInt32 bz) ;

Boolean calculeCosSin(UInt16 angle) ;

#define imin(A,B) ((A)<(B)?(A):(B))

#define imax(A,B) ((A)>(B)?(A):(B))

#define iborne(A,B,C) ((A)<(B)?(B):((A)>(C)?(C):(A)))

Boolean redresse(Coord x, Coord y, Coord *xn, Coord *yn) ;

UInt16 isqrt(UInt16 v) ; // Jim Henry isqrt

UInt16 atan(Coord x, Coord y) ;

UInt16 atan2p(Coord x1, Coord y1, Coord x0, Coord y0) ;

int angleCmp(UInt16 a1, UInt16 a0) ;

Coord estimeDist(Coord x, Coord y) ;

Coord iabs(Coord x) ;

#endif
