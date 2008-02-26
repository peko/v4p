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
** zone d'affichage = rectangle nbLignes*largeurLigne
** vue = zone rectangulaire de ce qui est afficher dans la scène
** scene = un ensemble de polygones
** poly = polygone = chemin fermé de n sommets, avec une certaine couleur et une certaine profondeur
** couleur = info de remplissage (pas seulement une couleur selon l'implémentation)
** sommet = coordonnées x,y dans le repère de la scène
** BA : Bord Actif = bord non horizontal à l'intérieur de la vue, formé par 2 sommets consécutifs
** sub : sous-polygone couplé au poly père (rotation,déplacement,optimisation)
** z = profondeur = numéro de couche d'un polygone
** tasXXX : résa mémoire pour XXX
** XXXP : pointeur d'un XXX
** IXXX : Indice d'un XXX dans une table
** XXXRappel : fonction définie à l'extérieur de V4P (CallBack)
** poly ouvert : poly couramment intersecté par la ligne de balayage : min(y(sommets)) < y < max(y(sommets))
** poly non encore ouvert : si y(scan-line) < min(y(sommets))
** poly fermé : y(scan-line) > max(y(sommets))
*/
#include "v4p.h"
#include <stdlib.h>
#include "lowmath.h"

// external data&func
extern Coord      nbLignes, largeurLigne ; // taille zone d'affichage
extern Couleur    couleurDeFond ; // couleur de fond

extern Boolean v4pAffichageRappel() ;
extern Boolean v4pErreurRappel(char *s,...) ;
extern Boolean v4pTranconRappel(Coord y, Coord x0, Coord x1, Couleur c) ;
extern Boolean v4pCollisionRappel(ICollision i1, ICollision i2, Coord py, Coord x1, Coord x2, PolyP p1, PolyP p2) ;
extern Boolean v4pAffichageFinRappel() ;


// type Polygone
typedef struct poly_s {
 PolyProps props ; // mot-bits des propriétées
 SommetP sommet1 ; // tête de liste des sommets
 Couleur couleur ; // couleur du polygône (en fait n'importe quelle info de remplissage)
 ICouche z ; // profondeur
 ICollision i ; // couche de collision (i!=z)
 PolyP sub1 ; // liste subs
 PolyP suivant ; // liens entre subs
 Coord minx,maxx,miny,maxy; // rectangle minimal autour
 Coord minyv, maxyv ;
 BAP BA1 ; // tête liste BA actif
} Poly ;

// type BA
typedef struct ba_s {
 Coord x0, y0, x1, y1 ; // vecteur-bord
 PolyP p ; // polygone propriétaire
 Coord x, o1, o2, s, h, r1, r2 ; // bresenham: offset, sommes, nbLignes, restes
} BA ;

// contexte V4P
typedef struct v4pContext_s {
 PolyP *scene ; // pointeur vers une scene a dessiner
 Coord pas ; // pas
 int debug1 ;
 Coord   xvu0,yvu0,xvu1,yvu1 ; // vue affichée, coordonées dans le repère de la scène
 QuickHeap tasSommet, tasPoly, tasBA ;
 List    listePolyOuverts, listePolyOuvrables ; // listes des poly ouverts courants et futures
 List    listeBAx, listeBAy ; //liste des BA courants en x et futures en y0
 Coord   dxvu,dyvu, // infos sur la vue
  divxvu,modxvu,divxvub,modxvub,
  divyvu,modyvu,divyvub,modyvub;
 Boolean echellage; // faut il appliquer un ratio sur les tailles de polygones?
} V4pContextS, *V4pContext ;

V4pContext v4p = NULL ; // Contexte V4P courant.

// Définit la vue
int v4pChangeVue(Coord x0, Coord y0, Coord x1, Coord y1) {
  v4p->xvu0=x0;
  v4p->yvu0=y0;
  v4p->xvu1=x1;
  v4p->yvu1=y1;
  v4p->dxvu=x1-x0;
  v4p->dyvu=y1-y0;
  if (!v4p->dxvu || !v4p->dyvu) return failure; // can't divide by 0
  v4p->divxvu = largeurLigne / v4p->dxvu;
  v4p->modxvu = largeurLigne % v4p->dxvu;
  v4p->divxvub = v4p->dxvu / largeurLigne;
  v4p->modxvub = v4p->dxvu % largeurLigne;
  v4p->divyvu = nbLignes / v4p->dyvu;
  v4p->modyvu = nbLignes % v4p->dyvu;
  v4p->divyvub = v4p->dyvu / nbLignes;
  v4p->modyvub = v4p->dyvu % nbLignes;
  v4p->echellage = !(v4p->divxvu==1 && v4p->divyvu==1 && v4p->modxvu==0 && v4p->modyvu==0);
  return success;
}


// Initialise le contexte V4P
V4pContext v4pContextCree() {
  v4p = (V4pContext)malloc(sizeof(V4pContextS)) ;
  v4p->scene = NULL;
  v4p->pas = 8;
  v4p->tasSommet = QuickHeapNewFor(Sommet) ;
  v4p->tasPoly = QuickHeapNewFor(Poly) ;
  v4p->tasBA = QuickHeapNewFor(BA) ;
  v4pChangeVue(0,0, largeurLigne, nbLignes);
  return v4p ;
}
int v4pInit() {
  v4pContextCree() ;
  return succes ;
}

int v4pContexteChange(V4pContext p) {
  v4p = p ;
}

// crée un poly
PolyP v4pPolyCree(PolyProps t, Couleur col, ICouche z) {
  PolyP p = QuickHeapAlloc(v4p->tasPoly) ;
  p->props = t;
  p->z = z;
  p->i = -1;
  p->couleur = col;
  p->sommet1 = NULL;
  p->sub1 = NULL;
  p->suivant = NULL;
  p->miny = ~0; // miny = bcp trop => calcul bornes a faire
  return p;
}

// supprime un poly (ses sommets, ses subs)
int v4pPolySupr(PolyP p) {
 while (p->sommet1) v4pPolySuprSommet(p,p->sommet1);
 while (p->sub1) v4pPolySuprSubPoly(p,p->sub1);
 QuickHeapFree(v4p->tasPoly,p);
 return success ;
}

// ajoute un poly dans une liste chainée par 'suivant'
PolyP v4pPolyDansListe(PolyP p, PolyP* liste) {
  p->suivant=*liste;
  *liste=p;
  return p ;
}

// retire un poly d'une liste chainée par 'suivant'
Boolean v4pPolyHorsListe(PolyP p, PolyP* liste) {
  PolyP ppl,pl;

  if (*liste == p)
    *liste = p->suivant;
  else {
    ppl = *liste;
    pl = ppl->suivant;
    while (pl != p && pl) {
      ppl = pl;
      pl = pl->suivant;
    }
    if (!pl) return (v4pErreurRappel("poly perdu"), failure) ;
    ppl->suivant = p->suivant;
  }
  p->suivant=NULL;
  return success ;
}

// ajoute un poly dans la liste des subs d'un autre poly
PolyP v4pPolyApoly(PolyP parent, PolyP p) {
  return v4pPolyDansListe(p, &parent->sub1);
}

// combo PolyCree+PolyDansListe
PolyP v4pListeAjoutePoly(PolyP *liste, PolyProps t, Couleur col, ICouche z) {
  return v4pPolyDansListe(v4pPolyCree(t, col, z), liste);
}

// combo ListeSuprPoly+PolyHorsListe
Boolean v4pListeSuprPoly(PolyP *liste, PolyP p) {
  return v4pPolyHorsListe(p, liste) || v4pPolySupr(p) ;
}

// combo PolyAPoly+PolyCree
PolyP v4pPolyAjouteSubPoly(PolyP parent, PolyProps t, Couleur col, ICouche z) {
   return v4pPolyApoly(parent, v4pPolyCree(t, col, z)) ;
}

// retire un poly de la liste des subs d'un autre poly et le supprime
Boolean v4pPolySuprSubPoly(PolyP parent, PolyP p) {
   return v4pPolyHorsListe(p, &parent->sub1) || v4pPolySupr(p) ;
}

// ajoute un flag dans les propriétés d'un poly
PolyProps v4pPolyPoseProp(PolyP p, PolyProps i) {
  return (p->props |= i) ;
}

// retire un flag des propriétés d'un poly
PolyProps v4pPolyOteProp(PolyP p, PolyProps i) {
  return (p->props &= ~i) ;
}

// ajoute un sommet à un poly
SommetP v4pPolyAjouteSommet(PolyP p, Coord x, Coord y) {
   SommetP s = QuickHeapAlloc(v4p->tasSommet) ;
   s->x = x;
   s->y = y;
   if (!p->sommet1) {
      p->minx = p->maxx = x ;
      p->miny = p->maxy = y ;
   } else {
      if (x < p->minx) p->minx = x ;
      else if (x > p->maxx) p->maxx = x ;
      if (y < p->miny) p->miny = y ;
      else if (y > p->maxy) p->maxy = y ;
   }
   s->suivant = p->sommet1;
   p->sommet1 = s;
   // p modif
   return s ;
}

// fixe la couleur d'un poly
Couleur v4pPolyPrendCouleur(PolyP p, Couleur c) {
   return p->couleur = c;
}

// retourne la liste des sommets d'un poly
SommetP v4pPolyDonneSommets(PolyP p) {
   return p->sommet1 ;
}

// retourne la profondeur d'un poly
ICouche v4pPolyDonneZ(PolyP p) {
   return p->z ;
}

// retourne la couleur d'un poly
Couleur v4pPolyDonneCouleur(PolyP p) {
   return p->couleur ;
}

// change les coordonnées d'un sommet d'un poly
SommetP v4pPolyBougeSommet(PolyP p, SommetP s, Coord x, Coord y) {
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

// supprime un sommet d'un poly
PolyP v4pPolySuprSommet(PolyP p, SommetP s) {
   SommetP pps, ps;

   if (p->sommet1 == s)
      p->sommet1 = s->suivant;
   else {
     pps = p->sommet1;
     ps = pps->suivant;
     while (ps != s && ps) {
        pps = ps;
        ps = ps->suivant;
     }
     if (!ps) return NULL ;
     pps->suivant = ps->suivant;
   }

   if (p->miny != ~0
       && (s->x == p->minx || s->y == p->miny
           || s->x == p->maxx || s->y == p->maxy)) {
      p->miny = ~0 ; // calcul a refaire
   }

   QuickHeapFree(v4p->tasSommet, s) ;

   // p modif
   return p ;
}

// transforme un chiffre hexa ('0-9,A-F') en int
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

// crée des sommets à partir d'une représentation en chaine
PolyP v4pDecodeSommets(PolyP p, char *s) {
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
            v4pPolyAjouteSommet(p, xs1 * v4p->pas, ys1 * v4p->pas);
         continue;
      }

      sep = false;

      xs = v4pXToD(c) << 4 + v4pXToD(s[++j]);
      j++ ;
      ys = v4pXToD(c) << 4 + v4pXToD(s[++j]);
      j++;
      v4pPolyAjouteSommet(p, xs * v4p->pas, ys * v4p->pas);

      if (sep) {
         xs1 = xs ;
         ys1 = ys ;
         psep = true;
      }
  }
  if (psep)
     v4pPolyAjouteSommet(p, xs1 * v4p->pas, ys1 * v4p->pas);

  return p;
}

// crée un poly et ajoute des sommets encodés dans une chaine
PolyP v4pPolyRapid(PolyProps t, Couleur col, ICouche z, char* s) {
  return v4pDecodeSommets(v4pPolyCree(t, col, z), s);
}

// encode les sommets d'un poly dans une chaine
char *v4pEncodeSommets(PolyP p) {
  static char *t = "0123456789ABCDEF" ;
  char *s;
  SommetP s1, m, pm;
  int i, l ;
  Coord v;

  s = (char *)malloc(32 * sizeof(char)) ;
  l = 0;
  s1 = p->sommet1 ;
  m = s1;
  pm = NULL;
  while (m) {
     if (pm && m == s1) {
        s[l++] = '.';
        pm = NULL;
        s1 = m->suivant;
        m = s1;
     } else {
        for (i = 0; i <= 1; i++) {
           if (!i) v = m->x;
              else v = m->y;
           v /= v4p->pas;
           s[l++] = t[v & 15];
           s[l++] = t[(v >> 4) & 15];
         }
         pm = m;
         m = m->suivant;
     }
     if (l % 32 >= 28) {
        s = (char *)realloc(s, (64 + l - l % 32) * sizeof(char)) ;
        if (!s) { v4pErreurRappel("tas plein") ; return NULL ; }
     }
  }
  s[l] = '\0';
  return s;
}

// fixe la couche de collision d'un poly
PolyP v4pPolyCognable(PolyP p, ICollision i) {
   p->i = i ;
   return p ;
}

// crée un BA chainé par 'suivant' aux autres BA du poly
BAP v4pBACree(PolyP p) {
   BAP b = QuickHeapAlloc(v4p->tasBA) ;
   b->p = p;
   b->suivant = p->BA1;
   return (p->BA1 = b);
}

// supprime tous les BA d'un poly
PolyP v4pPolySuprBAs(PolyP p) {
   BAP b, s;
   for (b = p->BA1; b; b = s) {
      s = b->suivant;
      QuickHeapFree(v4p->tasBA, b) ;
   }
   p->BA1 = NULL;
   return p;
}

// fonction pour merge-sort
int compareListeBAy0(void *data1, void *data2) {
   BAP b1 = data1, b2 = data2 ;
   return (b1->y0 >= b2->y0) ;
}

List v4pTriListeBAy(List liste) {
   ListSetCompareData(compareListeBAy0) ;
   return ListSort(liste) ;
}

// fonction récursive appelée par v4pAjusteClone()
PolyP v4pRecAjusteClone(Boolean estSub, PolyP c, PolyP p, Coord dx, Coord dy, ICouche dz) {
   SommetP sp, sc;
   Coord x, y, x2, y2;
   p->z = c->z+dz;
   sp = p->sommet1;
   sc = c->sommet1;
   while (sp) {
      x = sp->x;
      y = sp->y;
      redresse(x, y, &x2, &y2);
      x2+= dx;
      y2+= dy;
      v4pPolyBougeSommet(c, sc, x2, y2) ;
      sp = sp->suivant;
      sc = sc->suivant;
   }
   if (estSub && p->suivant)
      v4pRecAjusteClone(true, c->suivant, p->suivant, dx, dy, dz);
   if (p->sub1)
      v4pRecAjusteClone(true, c->sub1, p->sub1, dx, dy, dz);
   // c modif
   return c ;
}

// met à jour un clone c d'un poly p, tel que sommets(c) = transfo(sommets(p),delta-x/y, angle-rotation)
PolyP v4pAjusteClone(PolyP c, PolyP p, Coord dx, Coord dy, int angle, ICouche dz) {
  calculeCosSin(angle);
  /* a voir: ratiox et ratioy :
       cosa:=(cosa*ratiox) shr 7;
       sina:=(sina*ratioy) shr 7;
   */
  return v4pRecAjusteClone(false, c, p, dx, dy, dz);
}

// transforme un poly (fn de transfo appliquée au poly)
PolyP v4pAjustePoly(PolyP p, Coord dx, Coord dy, int angle, ICouche dz) {
   return v4pAjusteClone(p, p, dx, dy, angle, dz);
}

// fonction récursive appelée par v4pPolyClone
PolyP v4pRecClonePoly(Boolean estSub, PolyP p) {
   SommetP s;
   PolyP c = v4pPolyCree(p->props, p->couleur, p->z);

   for (s = p->sommet1 ; s ; s = s->suivant)
      v4pPolyAjouteSommet(c, s->x, s->y);

   if (estSub && p->suivant)
      c->suivant = v4pRecClonePoly(true, p->suivant);
   if (p->sub1)
      c->sub1 = v4pRecClonePoly(true, p->sub1);

   return c;
}

// clone un poly (et tous ses descendants)
PolyP v4pPolyClone(PolyP p) {
   return v4pRecClonePoly(false, p);
}

// combo PolyClone+PolyDansListe
PolyP v4pListeAjouteClone(PolyP *liste, PolyP p) {
   return v4pPolyDansListe(v4pPolyClone(p), liste);
}

#define v4pMinCoord(A,B) ((A) < (B) ? (A) : (B))
#define v4pMaxCoord(A,B) ((A) < (B) ? (B) : (A))

// calcule le rectangle minimal entourant p
PolyP v4pPolyCalcBornes(PolyP p) {
   SommetP s = p->sommet1;
   if (!s) { // pas de sommet
      p->miny = ~0;
   } else {
      Coord minx, maxx, miny, maxy;
      maxx = minx = s->x;
      maxy = miny = s->y;
      for (s = s->suivant; s; s = s->suivant) {
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

// renvoie faux si le polygone est complètement en dehors de la vue (vrai sinon)
Boolean v4pEstVisible(PolyP p) {
   if (!p->sommet1)
      return false ;
   if (p->miny == ~0) // bornes pas a jour
      v4pPolyCalcBornes(p) ;

   { Coord minx = p->minx, maxx = p->maxx, miny = p->miny, maxy = p->maxy;

   if (!(p->props & relatif)) {
      minx -= v4p->xvu0;
      miny -= v4p->yvu0;
      maxx -= v4p->xvu0;
      maxy -= v4p->yvu0;
      if (v4p->echellage) {
         minx = minx * v4p->divxvu + (minx * v4p->modxvu) / v4p->dxvu;
         maxx = maxx * v4p->divxvu + (maxx * v4p->modxvu) / v4p->dxvu;
         miny = miny * v4p->divyvu + (miny * v4p->modyvu) / v4p->dyvu;
         maxy = maxy * v4p->divyvu + (maxy * v4p->modyvu) / v4p->dyvu;
      }
   }
   p->minyv = miny ;
   p->maxyv = maxy ;
   return (maxx >= 0 && maxy >= 0 && minx < largeurLigne && miny < nbLignes) ;
   }
}

// genere les BA associés à un poly et les ajoute à la liste
PolyP v4pPolyCompileBA(PolyP p) {
   SommetP sa, sb, s1;
   Boolean boucle, fin;
   int dx, dy;
   Coord sx0, sx1, sy0, sy1;
   BAP b ;


   p->BA1 = NULL;
   if (!v4pEstVisible(p) || (p->props & invisible))
      return p;

   s1 = p->sommet1;
   while (s1) { // lacots
      sa = s1;
      sb = s1->suivant;
      boucle = false;
      fin = false;
      while (!fin) { // sommets
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
            dx = sx1 - sx0;
            dy = sy1 - sy0;

            if (!(p->props & relatif)) {
                sx0 -= v4p->xvu0;
                sy0 -= v4p->yvu0;
                sx1 -= v4p->xvu0;
                sy1 -= v4p->yvu0;
                if (v4p->echellage) {
                   sx0 = sx0 * v4p->divxvu + (sx0 * v4p->modxvu) / v4p->dxvu;
                   sy0 = sy0 * v4p->divyvu + (sy0 * v4p->modyvu) / v4p->dyvu;
                   sx1 = sx1 * v4p->divxvu + (sx1 * v4p->modxvu) / v4p->dxvu;
                   sy1 = sy1 * v4p->divyvu + (sy1 * v4p->modyvu) / v4p->dyvu;
               }
            }

            // troncage haut
            if (sy0 < 0 && dy > 0) {
               sx0 -= sy0 * (dx / dy) + sy0 * (dx % dy) / dy;
               sy0 = 0;
            }

            b = v4pBACree(p) ;
            b->x0 = sx0;
            b->y0 = sy0;
            b->x1 = sx1;
            b->y1 = sy1;
            if (dy > 0) {
               b->o1 = dx / dy;
               if (dx > 0) {
                  b->r1 = dx % dy;
                  b->o2 = b->o1 + 1;
               } else {
                  b->r1 = (-dx) % dy;
                  b->o2 = b->o1 - 1;
               }
               b->r2 = b->r1 - dy;
            } else {
               b->o1 = b->o2 = b->r1 = b->r2 = 0 ;
            }

            // BA dans liste y
            ListAddData(v4p->listeBAy, b) ;

         } // actif
         if (boucle) {
            s1 = NULL ;
            fin = true ;
         } else if (sb->x == s1->x && sb->y == s1->y) {
            // nouveau lacot
            s1 = sb->suivant ;
            fin = true ;
         } else {
            sa = sb ;
            sb = sb->suivant ;
            if (!sb) {
               sb = s1 ;
               boucle = true ;
            }
         }
      } //  sommets
   } // lacots
   return p ;
}

// compare le minyv de 2 polys
int comparePolyMinyv(void *data1, void *data2) {
  PolyP p1 = data1, p2 = data2 ;
  return (p1->minyv >= p2->minyv) ;
}

// trie une liste de poly dans l'ordre de 'miny'
List v4pTriPoly(List liste) {
   ListSetCompareData(comparePolyMinyv) ;
   return ListSort(liste) ;
}

// compare le maxyv de 2 polys
int comparePolyMaxyv(void *data1, void *data2) {
  PolyP p1 = data1, p2 = data2 ;
  return (p1->maxyv >= p2->maxyv) ;
}

// trie une liste de poly dans l'ordre de 'maxy'
List v4pTriPolyOuverts(List liste) {
   ListSetCompareData(comparePolyMaxyv) ;
   return ListSort(liste) ;
}

// charge une liste de poly dans v4p.listePolyOuvrables, ainsi que leurs descendants
void v4pChargelistePolyOuvrables(PolyP l) {
   PolyP p ;
   for (p = l ; p ; p = p->suivant) {
      if (p->props & inactif) continue ;

      if (!v4pEstVisible(p)) continue ;

      if (p->sub1) v4pChargelistePolyOuvrables(p->sub1) ;

      ListAddData(v4p->listePolyOuvrables, p) ;
   }
}

// fonction appelée par v4pTriBA()
int compareBAx(void *data1, void *data2) {
   BAP p1=data1, p2=data2 ;
   return (p1->x >= p2->x) ;
}

// trie une liste de BA dans l'ordre de 'x'
List v4pTriBA(List liste) {
   ListSetCompare(compareBAx) ;
   return ListSort(liste) ;
}

// dessine un trancon de scan-line en fonction du poly auquel il appartient
Boolean v4pDessinTrancon(int y, int x0, int x1, PolyP p) {
   return v4pTranconRappel(y, x0, x1, p ? p->couleur : couleurDeFond) ;
}

// projette des coordonnées relatives à la vue dans le repère de la scène
void v4pVueEnAbsolu(Coord x, Coord y, Coord *xa, Coord *ya) {
   *xa = v4p->xvu0 + x * v4p->divxvub + x * v4p->modxvub / largeurLigne ;
   *ya = v4p->yvu0 + y * v4p->divyvub + y * v4p->modyvub / nbLignes ;
}

// ouvre tous les polys nouvellement intersectés par la scan-line
PolyP v4pOuvrePolys(int y) {
   PolyP p ;
   List l ;
   Boolean nouv = false ;

   for (l = v4p->listePolyOuvrables ; l && (p = (PolyP)ListData(l))->miny <= y ; l = ListFree(l)) {
      if (p->maxyv < y) continue ;

      v4pPolyCompileBA(p) ;
      nouv = true ;

      ListAddData(v4p->listePolyOuverts, p) ;
   }

   if (nouv) {
     v4p->listeBAy = v4pTriBAy(v4p->listeBAy) ;
     v4p->listePolyOuverts = v4pTriPolyOuverts(v4p->listePolyOuverts) ;
   }

   return v4p->listePolyOuvrables = l ;
}

// ouvre tous les BA nouvellement intersectés par la scan-line
Boolean v4pOuvreBA(Coord y) {
   Boolean ouvr = false ;
   List l ;
   BAP b ;
   for ( l = v4p->listeBAy ; l && (b = (BAP)ListData(l))->y0 <= y ; l = ListNext(l)) {
      if (b->y1 <= y) continue ;
      b->s = b->r2 - 1 ;
      b->x = b->x0 ;
      v4pBADansListe(b) ;
      ouvr = true ;
   }
   v4p->listeBAy = l ;
   return ouvr ;
}
void dprintf(Coord x, Coord y, Char *formatString, ...) ;

// affiche une scene avec une vue
Boolean v4pAffiche() {
   List l, pl ;
   PolyP p, polyVisible ;
   BAP b, pb ;
   Coord y, px ;
   ICouche z ;
   PolyP couches[16] ;
   int zMax ;
   UInt16 bz, bi ; // mot des couches et des collisions
   UInt16 mz, mi ; // masques
   ICollision i, colli1, colli2 ;
   int nColli ;
   PolyP pColli[16] ;

   v4pAffichageRappel() ;

   // vide les BA
   v4p->tasBA->size = 0 ;
   v4p->tasBA->hole = NULL ;
   v4p->listeBAy = NULL ; // tri vertical BA

   //listePolyOuvrables = prochain poly a ouvrir
   //listePolyOuverts = prochain poly ouvert a fermer
   v4p->listePolyOuvrables = NULL ;
   v4p->listePolyOuverts= NULL ;
   if (!v4p->scene) return failure;
   v4pChargelistePolyOuvrables(*(v4p->scene)) ;
   v4p->listePolyOuvrables = v4pTriPoly(v4p->listePolyOuvrables) ;
   v4pOuvrePolys(0) ;

   // liste BA ouverts
   v4p->listeBAx = NULL ;
   v4pOuvreBA(0) ;
   v4p->listeBAx = v4pTriBA(v4p->listeBAx) ;
   // bcl scanline
   for (y = 0 ; y < nbLignes ; y++) {

      // bcl ferme poly
      l = v4p->listePolyOuverts ;
      for ( ; l && y > (p = (PolyP)ListData(l))->maxyv) ; l = ListFree(l))
         v4pPolySuprBAs(p) ;
      v4p->listePolyOuverts = l;

      // bcl BA ouvert
      l = v4p->listeBAx ;
      pl = NULL ;
      while (l) {
         b = (BAP)ListData(l) ;
         if (y >= b->y1) { // ferme BA
            if (pl)
               ListSetNext(pl, l = ListFree(l)) ;
            else
               v4p->listeBAx = l = ListFree(l) ;
         } else { // decale BA

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
      } // bcl BA ouvert

      // ouvre poly nouvellement intersectés
      v4pOuvrePolys(y) ;

      // ouvre BA nouvellement intersectés
      v4pOuvreBA(y) ;

      // trie BA
      v4p->listeBAx = v4pTriBA(v4p->listeBAx) ;

      // reset couches
      bz = 0 ;
      polyVisible = NULL ;
      zMax = -1 ;

      // reset collisions
      bi = 0 ;
      nColli = 0 ;

      // parcours trancon de la scanline
      px = 0 ;
      b = NULL ;
      for (l = v4p->listeBAx ; l ; l = ListNext(l)) { // parcours BA ouvert / x
          pb = b ;
          b = (BAP)ListData(l) ;
          if (b->x > 1000) dprintf(0,0, "problem %d %d %d", (int)b->x, (int)px, (pb ? pb->x : -1));
          if (b->x > 0) {
              //if (px > b->x) v4pErreurRappel("pb trancon %d %d %d", (int)y, (int)px, (int)b->x);
              if (y >= 0 && y < nbLignes && px < largeurLigne)
                 v4pDessinTrancon(y, v4pMaxCoord(px, 0), v4pMinCoord(b->x, largeurLigne), polyVisible);
              if (nColli>1)
                 v4pCollisionRappel(colli1, colli2, y, px, b->x, pColli[colli1], pColli[colli2]);
          }
          px = b->x;
          p = b->p;
          i = p->i;
          z = p->z & 15;
          mz = (UInt16)1 << z ;
          mi = (i < 0 ? (UInt16)0 : (UInt16)1 << (i & 15)) ;
          if ((bz ^= mz) & mz) {
             couches[z] = p ;
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
                   zMax = bitMax(bz) ;
                   //dprintf(0,120-5*zMax,"v%d",(int)(zMax));
                   polyVisible = (zMax>=0 ? couches[zMax] : NULL) ;
             }
             if (bi & mi) {
                bi ^= mi ;
                nColli-- ;
                if (nColli == 1 && i == colli1)
                   colli1 = colli2 ;
                else if (nColli == 2)
                   if (i == colli1)
                      colli1 = bitMax(bi ^ (1 << colli2)) ;
                   else if (i == colli2)
                      colli2 = bitMax(bi ^ (1 << colli1)) ;
             }
          }
       } // parcours BA ouvert en x

       // dernier trancon
       if (px < largeurLigne)
          v4pDessinTrancon(y, imax(0, px), largeurLigne, NULL) ;

   } // bcl y ;

   // vide les listes
   while (v4p->listePolyAOuvrir)
     v4p->listePolyAOuvrir = ListFree(v4p->listePolyAOuvrir) ;
   while (v4p->listeBAx)
     v4p->listeBAx = ListFree(v4p->listeBAx) ;

   // ferme les polys encore ouverts
   while (v4p->listePolyOuverts) {
      v4pPolySuprBAs((BAP)ListData(v4p->listePolyOuverts)) ;
      v4p->listePolyOuverts = ListFree(v4p->listePolyOuverts) ;
   }

   v4pAffichageFinRappel() ;
   return success ;
}

// Ajoute 4 sommets en rectangle
PolyP v4pPoly4S(PolyP p, Coord x0, Coord y0, Coord x1, Coord y1) {
   v4pPolyAjouteSommet(p, x0, y0) ;
   v4pPolyAjouteSommet(p, x0, y1) ;
   v4pPolyAjouteSommet(p, x1, y1) ;
   v4pPolyAjouteSommet(p, x1, y0) ;
   return p ;
}

PolyProps v4pActivePoly(PolyP p) {
   return v4pPolyOteProp(p, inactif) ;
}

PolyProps v4pDesactivePoly(PolyP p) {
   return v4pPolyPoseProp(p, inactif) ;
}

// Prend un pointeur de liste: a scene de polys
void v4pPrendScene(PolyP *scene) {
   v4p->scene = scene ;
}
