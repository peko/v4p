#include "v4pi.h"


extern Boolean pen ;
extern Coord xpen, ypen ;

int  i,j,k,dist,mindist;

ICouche z ;

PolyP  scene;

Coord x,y,xvu,yvu,lvu,
 xpen1,ypen1, yBouton = 1,
 x0,y0,x1,y1,pasGrid ;

PolyP marques[64] ;

Boolean pen1 ;

int iBouton = 0, sel ;
int bAjout,bDel,bHauteur,
 bScroll,bSave,bCol,bGrid ;

PolyP
 pSel,
 pCol,pSelCol,
 pGrid,pSelGrid,
 pHauteur,pSelHauteur,
 polyCourant,brosse,focus;

SommetP sommetCourant,s;

ICouche zCourant;

Couleur couleurCourante, prochaineCouleur ;

PolyP boutons[16] ;

int ajouteBouton(Couleur col) {
  PolyP bouton = v4pListeAjoutePoly(&scene, relatif, col, 14);
  boutons[iBouton] = bouton ;
  v4pPoly4S(bouton, largeurLigne - 10, yBouton, largeurLigne - 1, 9 + yBouton);
  yBouton+=10;
  return iBouton++;
}

void ajusteSel(int s) {
  if (sel != s) {
    v4pAjustePoly(pSel, 0, (s - sel) * 10, 0, 0);
    sel=s;
  }
}

typedef enum { idle, push, edit} GuiStatus ;

int nMarque ;
GuiStatus guiStatus ;



Boolean initApp() {
  v4pFlavorInit(green);
  scene = NULL;
  v4pPrendScene(&scene);
  xvu = -largeurLigne / 2;
  yvu = -nbLignes / 2;
  lvu = largeurLigne ;
  v4pChangeVue(xvu, yvu, xvu + lvu, yvu + lvu);
  nMarque=0;
  guiStatus= idle;
  brosse = NULL;
  focus = NULL;
  pasGrid = 8;
  bAjout = ajouteBouton(red);
  bDel = ajouteBouton(gray);
  bHauteur = ajouteBouton(blue);
  bScroll = ajouteBouton(yellow);
  bCol = ajouteBouton(black);
  bGrid = ajouteBouton(cyan);
  //bSave = ajouteBouton(fluo);
  sel = 0 ;
  couleurCourante = black ;
  zCourant = 7 ;
  polyCourant = NULL;
  //(-xvu,-yvu)=milieu écran
  pSel = v4pListeAjoutePoly(&scene, relatif, black, 13);
  v4pPoly4S(pSel, largeurLigne - 11, 0 , largeurLigne, 11);
  pCol=v4pListeAjoutePoly(&scene, relatif | inactif,black, 14);
  v4pPoly4S(pCol, -xvu - 20, -yvu - 20, -xvu + 20, -yvu + 20);
  pSelCol=v4pPolyAjouteSubPoly(pCol, relatif, black, 15);
  v4pPoly4S(pSelCol, -xvu - 18, -yvu - 18, -xvu + 18, -yvu + 18);
  pHauteur=v4pListeAjoutePoly(&scene, relatif | inactif, black, 14);
  v4pPoly4S(pHauteur, -xvu - 3, -yvu - 17, -xvu + 3, -yvu + 17);
  pSelHauteur=v4pPolyAjouteSubPoly(pHauteur, relatif, red, 15);
  v4pPoly4S(pSelHauteur, -xvu - 2, -yvu - 1, -xvu + 2, -yvu + 1);
  pGrid=v4pListeAjoutePoly(&scene, relatif | inactif, black, 14);
  v4pPoly4S(pGrid, -xvu - 9, -yvu - 9, -xvu + 9, -yvu + 9);
  pSelGrid=v4pPolyAjouteSubPoly(pGrid, relatif, red, 15);
  v4pPoly4S(pSelGrid, -xvu - 2, -yvu - 2, -xvu + 2, -yvu + 2);
  return success ;
}

Coord aligne(Coord x) {
  if (pasGrid <= 1)
     return x ;
  else if (x>0)
     return ((x + pasGrid / 2) / pasGrid) * pasGrid;
  else
     return ((x - pasGrid / 2) / pasGrid) * pasGrid;
}

Boolean iterateApp() {
  Coord  pasGrid0, pasGridPrec,
     xs, ys,
     xi, yi,
     t1, t2 ;
  ICouche z0, zPrec ;
  int selPrec ;

  if (true) {
    //v4pChangeVue(xvu,yvu,xvu+lvu,yvu+lvu);
    v4pAffiche();
    if (pen) {
      if (pen1) {
        xpen = (2 * xpen + xpen1) / 3;
        ypen = (2 * ypen + ypen1) / 3;
      }
      xpen1 = xpen;
      ypen1 = ypen;

      v4pVueEnAbsolu(xpen, ypen, &x, &y);
      xs = aligne(x);
      ys = aligne(y);

      if (guiStatus == push) { // bar move
        if (sel == bGrid) {
         pasGridPrec = pasGrid;
         pasGrid = 1 << ((iabs(ypen-y0) / 4) % 4) ;
         if (pasGrid != pasGridPrec)
            v4pAjustePoly(pSelGrid, pasGrid - pasGridPrec, pasGrid - pasGridPrec, 0, 0);
        } else if (sel == bCol) {
         prochaineCouleur = (((iabs(ypen - y0) + iabs(xpen - x0))) + couleurCourante) % 255;
         v4pPolyPrendCouleur(pSelCol, prochaineCouleur);
        } else if (sel == bHauteur) {
         zPrec = zCourant;
         zCourant = (z0 + (iabs(ypen - y0) / 4)) % 15 ;
         if (zPrec != zCourant) v4pAjustePoly(pSelHauteur, 0, (zPrec - zCourant) * 2, 0, 0);
        }
      } else if (guiStatus == edit) { // screen move
        if (brosse) {
          if (sel == bScroll) {
            v4pListeSuprPoly(&scene, brosse);
            brosse = NULL;
          } else {
            v4pAjustePoly(brosse, xpen - x0, ypen - y0, 0, 0);
            x0 = xpen;
            y0 = ypen;
          }
        }
        if (sel == bAjout) {
          if (sommetCourant) {
             v4pAjustePoly(marques[nMarque],
               xs - sommetCourant->x,
               ys - sommetCourant->y,
               0, 0);
             v4pPolyBougeSommet(polyCourant, sommetCourant, xs, ys);
          }
        } else if (sel==bGrid && sommetCourant) {
          v4pPolyBougeSommet(focus, sommetCourant, xs, ys);
          
        } else if (collisions[2].q > 0) {
          if (sel == bScroll) {
            focus = collisions[2].poly;
            x0 = xs;
            y0 = ys;
          } else if (sel == bGrid) {
            if (!focus) {
              focus = collisions[2].poly;
              s = v4pPolyDonneSommets(focus);
              mindist = estimeDist(s->x - x, s->y - y);
              sommetCourant = s;
              s = s->suivant;
              while (s) {
                dist = estimeDist(s->x - x, s->y - y);
                if (dist < mindist) {
                  mindist = dist;
                  sommetCourant = s;
                }
                s = s->suivant;
              }
            }
          } else
            focus =collisions[2].poly;
        } else if (sel == bScroll) { // scroll fond
          if (focus) {
            v4pAjustePoly(focus, xs - x0, ys - y0, 0, 0);
            x0 = xs; y0 = ys;
          } else {
            v4pChangeVue(
              aligne(xvu + xpen - x0),
              aligne(yvu + ypen - y0),
              aligne(xvu + xpen - x0) + lvu,
              aligne(yvu + ypen - y0) + lvu);
          }
        }
      } else { //pen down
        if (xpen > largeurLigne - 10 && ypen < yBouton) { //bar pen down
         selPrec = sel;
         ajusteSel(ypen / 10);
         if (selPrec == bAjout) {
           if (polyCourant && nMarque <= 2)
             v4pListeSuprPoly(&scene, polyCourant);
           
           while (nMarque) {
             nMarque--;
             if (nMarque < 20) v4pListeSuprPoly(&scene, marques[nMarque]);
           }
         }
         nMarque = 0;
         focus = NULL;
         polyCourant = NULL;
         sommetCourant = NULL;
         x0 = xpen;
         y0 = ypen;
         if (sel == bCol) {
           v4pActivePoly(pCol);
         } else if (sel == bScroll) {
         } else if (sel == bHauteur) {
           v4pActivePoly(pHauteur);
           z0 = zCourant;
         } else if (sel == bGrid) {
           v4pActivePoly(pGrid);
           pasGrid0 = pasGrid;
           y0 = 4 * (ypen - bitMax(pasGrid));
         }
         guiStatus = push;
        } else { // screen pen down
         if (sel == bAjout) {
           if (nMarque == 0) {
             polyCourant = v4pListeAjoutePoly(&scene,plein, couleurCourante, zCourant);
             v4pPolyCognable(polyCourant, 0);
           }
           sommetCourant = v4pPolyAjouteSommet(polyCourant, xs, ys);
           if (nMarque < 20) {
             marques[nMarque] = v4pListeAjoutePoly(&scene,plein, couleurCourante,14);
             v4pPoly4S(marques[nMarque], xs - 1 , ys - 1 , xs + 1 , ys + 1);
           }
         }
         brosse = v4pListeAjoutePoly(&scene, relatif, black, 15);
         v4pPoly4S(brosse, xpen - 1 , ypen - 1 , xpen + 1 , ypen + 1);
         v4pPolyCognable(brosse, 2);
         x0 = xpen;
         y0 = ypen;
         guiStatus = edit;
       }//tap ecran
     }//pen down
    } else { // no pen
      if (guiStatus == push) { // bar pen up
       v4pDesactivePoly(pCol);
       if (sel == bCol) v4pPolyPrendCouleur(boutons[bCol], (couleurCourante = prochaineCouleur));
       if (sel == bHauteur) v4pDesactivePoly(pHauteur);
       if (sel == bGrid) v4pDesactivePoly(pGrid);
      } else if (guiStatus==edit) { // screen pen up
       if (sel == bAjout) {
         nMarque++;
       } else if (focus) {
         if (sel == bCol) v4pPolyPrendCouleur(focus, couleurCourante);
         if (sel == bDel) v4pListeSuprPoly(&scene, focus);
         if (sel == bHauteur) v4pAjustePoly(focus, 0, 0, 0, zCourant - v4pPolyDonneZ(focus));
         focus = NULL;
         sommetCourant = NULL;
       } else if (sel == bScroll && !focus) {
         xvu = aligne(xvu + (xpen - x0));
         yvu = aligne(yvu + (ypen - y0));
         v4pChangeVue(xvu, yvu, xvu + lvu, yvu + lvu);
       }
       if (brosse) {
         v4pListeSuprPoly(&scene, brosse);
         brosse = NULL;
       }
      }//pen up ecran;
    guiStatus = idle;
  }//no pen
  pen1 = pen;
 }//buffer
 return success ;
}

