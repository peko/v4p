#ifndef V4P
#define V4P
#include "v4p_ll.h"


/*
** v4p = Vectors rendition engine for Pocket
*/

/* Abbrevs
** poly: Polygone = chemin fermé de n sommets, à une certaine profondeur
** scene : un ensemble de polygones
** vue : une zone rectangulaire qui délimite une portion de scène
** sommet : coordonnées x,y dans le repère de la scène
** BA : Bord Actif = bord non horizontal, dans le champ de la vue, formé par 2 sommets consécutifs
** sub : sous-polygone couplé au poly père (rotation,déplacement,optimisation)
** z : numéro de couche des polygones ("profondeur")
** tasXXX : résa mémoire pour XXX
** maxXXX : nb max de XXX
** XXXP : pointeur d'un XXX dans tasXXX
** IXXX : Indice d'un XXX dans une table
** XXXRappel : fonction implémentée à l'extérieur du présent fichier (CallBack)
*/

#define MAX_SOMMET 320
#define MAX_POLY 80
#define MAX_BA 200

typedef UInt16 Flags ;
#define plein (Flags)0
#define complement (Flags)1 // troue/étend le père (forme avec héritage)
#define invisible (Flags)2 // utile pour réunir des poly en invisible
#define translucide (Flags)4 // non-implémenté

#define absolu (Flags)0
#define relatif (Flags)16 // coordonnées dans le repère de la vue et pas la scène
#define inactif (Flags)32 // rend invisible un poly et ses fils sans le retirer de la hiérarchie

typedef UInt16 ICouche ; // < 16
typedef UInt16 ICollision ; // < 8

typedef UInt32 Couleur ;
typedef UInt16 PolyProps ;
typedef Int16  Coord ;
typedef struct sommet_s *SommetP ;
typedef struct poly_s *PolyP ;
typedef struct ba_s *BAP ;


typedef struct sommet_s {
 Coord x, y ;
 SommetP suivant ;
} Sommet ;

// v4p
int  v4pInit() ;
int  v4pChangeVue(Coord x0, Coord y0, Coord x1, Coord y1) ;
void v4pVueEnAbsolu(Coord x, Coord y, Coord *xa, Coord *ya) ;
void v4pPrendScene(PolyP *scene) ;
Boolean v4pAffiche() ;

// v4pPoly
PolyP v4pPolyCree(PolyProps t, Couleur col, ICouche pz) ;
PolyP v4pPolyClone(PolyP p) ;
PolyP v4pPolyCognable(PolyP p, ICollision i) ;
PolyP v4pPolyDansListe(PolyP p, PolyP* liste) ;
Boolean v4pPolyHorsListe(PolyP p, PolyP* liste) ;
PolyP v4pPolyApoly(PolyP parent, PolyP p) ;
PolyP v4pPolyAjouteSubPoly(PolyP parent, PolyProps t, Couleur col, ICouche z) ;
Boolean  v4pPolySuprSubPoly(PolyP parent, PolyP p) ;
PolyP v4pPolySuprSommet(PolyP p, SommetP s) ;
PolyProps v4pPolyPoseProp(PolyP p, PolyProps i) ;
PolyProps v4pPolyOteProp(PolyP p, PolyProps i) ;
SommetP   v4pPolyAjouteSommet(PolyP p, Coord x, Coord y) ;
SommetP   v4pPolyBougeSommet(PolyP p, SommetP s, Coord x, Coord y) ;
Couleur   v4pPolyPrendCouleur(PolyP p, Couleur c) ;
SommetP   v4pPolyDonneSommets(PolyP p) ;
ICouche   v4pPolyDonneZ(PolyP p) ;
Couleur   v4pPolyDonneCouleur(PolyP p) ;

// v4pListe
#define v4pListeCree(LISTE) (LISTE)=NULL
PolyP v4pListeAjoutePoly(PolyP *liste, PolyProps t, Couleur col, ICouche z) ;
PolyP v4pListeAjouteClone(PolyP *liste, PolyP p) ;
Boolean   v4pListeSuprPoly(PolyP *liste, PolyP p) ;

// encodage
char *v4pEncodeSommets(PolyP p) ;
PolyP v4pDecodeSommets(PolyP p, char *s) ;
PolyP v4pPolyRapid(PolyProps t, Couleur col, ICouche pz, char* s) ;

// transformation
PolyP v4pAjusteClone(PolyP c, PolyP p, Coord dx, Coord dy, int angle, ICouche dz) ;
PolyP v4pAjustePoly(PolyP p, Coord dx, Coord dy, int angle, ICouche dz) ;

// helpers
PolyP v4pPoly4S(PolyP p, Coord x0, Coord y0, Coord x1, Coord y1) ;
PolyProps v4pActivePoly(PolyP p) ;
PolyProps v4pDesactivePoly(PolyP p) ;

#endif
