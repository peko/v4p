#include <PalmOS.h>
#include <C:\PALMPHI\toto\Toto.h>
#include "v4pi.h"

#define EVENT int
EventPtr __evt;
#define StrToInt(x) StrAToI(x)
#define FIELDTYPE_STRING 0
#define FIELDTYPE_INTEGER 1
#define StrToInt(x) StrAToI(x)
enum _BorderStyles {bsNone,bsSingle,bsBold};
typedef struct {
    Char Visible,Border;
    UInt16 Left,Top,Width,Height;
    UInt16 *ids;
    UInt32 color;
} PanelType;
typedef struct {
    DmOpenRef bd;
    UInt16 cat;
} PalmDBType;
char StopEvent=0,EditAutoDraw=1;
#define TotoF_top 0
#define TotoF_left 0
#define TotoF_width 160
#define TotoF_height 160
register void *reg_a4 asm("%a4");
#define CALLBACK_PROLOGUE void *save_a4 = reg_a4; asm("move.l %%a5,%%a4; sub.l #edata,%%a4" : :);
#define CALLBACK_EPILOGUE reg_a4 = save_a4;


// Definition of Function : SetScreenMode
void SetScreenMode(int bpp)
{
   UInt32 requestedDepth=bpp;
   WinScreenMode(winScreenModeSet,NULL,NULL,&requestedDepth,NULL);
}

// Definition of Function : Application_ProcessMessages
Boolean Application_EventHandler(EventPtr event);
void Application_ProcessMessages(int ticks)
{
     EventType event;
     UInt32 until=TimGetTicks()+ticks;
     UInt16 error;
     do {
	     EvtGetEvent(&event, 3);
	     if(event.eType == appStopEvent) {
		     StopEvent=1;
		     break;
	     }
	     if (! SysHandleEvent(&event))
	         if (! MenuHandleEvent(0, &event, &error))
	             if (! Application_EventHandler(&event))
	                FrmDispatchEvent(&event);
     } while(TimGetTicks()<until);
}

EVENT TOTO_OnStart(EventPtr event) ;
EVENT TOTO_OnTerminate(EventPtr event) ;
FormPtr TotoF;
EVENT TotoF_OnOpen(EventPtr event) ;
EVENT TotoF_OnPenDownXY(UInt16 X,UInt16 Y) ;
EVENT TotoF_OnPenUpXY(UInt16 X,UInt16 Y) ;
EVENT TotoF_OnPenMoveXY(UInt16 X,UInt16 Y) ;
Err StartApplication(void)
{
           TOTO_OnStart(NULL);
    FrmGotoForm(_TotoF);
    return 0;
}

void StopApplication(void)
{
    if (EditAutoDraw == EditAutoDraw) {} 
           TOTO_OnTerminate(NULL);
	FrmSaveAllForms();
	FrmCloseAllForms();
}


Boolean TotoF_EventHandler(EventPtr event)
{
    Boolean handled = false;
    CALLBACK_PROLOGUE
    __evt=event;
    TotoF=FrmGetFormPtr(_TotoF);
    switch(event->eType) {

        case ctlSelectEvent:
            break;
        case ctlRepeatEvent:
            break;
        case penDownEvent:
        {
           TotoF_OnPenDownXY(event->screenX,event->screenY);
            break;
        }
        case penUpEvent:
        {
           TotoF_OnPenUpXY(event->screenX,event->screenY);
            break;
        }
        case frmUpdateEvent:
        {
              FrmHandleEvent(TotoF,event);
               handled=true;
            break;
        }
        case penMoveEvent:
        {
           RectangleType __bounds;
           TotoF_OnPenMoveXY(event->screenX,event->screenY);
            break;
        }
        case tblSelectEvent:
            break;
        case lstSelectEvent:
            break;
        case popSelectEvent:
            break;
        case fldEnterEvent:
            break;
        case sclRepeatEvent:
            break;
        case sclExitEvent:
            break;
        case frmOpenEvent:
           {
           }
           FrmDrawForm(TotoF);
           TotoF_OnOpen(event);
           handled = true;
           break;
        case frmSaveEvent:
           break;
        case frmCloseEvent:
           break;
       default:
           break;
    }
    CALLBACK_EPILOGUE
    return handled;
}
Boolean Application_EventHandler(EventPtr event)
{
    FormPtr frm;
    int     formId;
    Boolean handled = false;

    switch (event->eType) {
       case frmLoadEvent : 
       {
           formId = event->data.frmLoad.formID;
           frm = FrmInitForm(formId);
           FrmSetActiveForm(frm);
                         
           switch(formId) {
                        
               case _TotoF:
                   FrmSetEventHandler(frm, TotoF_EventHandler);
                   break;
                
           }
           handled = true;
       }
       case menuEvent : 
       {
            switch (event->data.menu.itemID) {
            }
            handled = true;
       }
       default:
           break;
    }
    return handled;
}
void EventLoop(void)
{
    EventType event;
    UInt16 error;

    do {
        if(!StopEvent)
           EvtGetEvent(&event, evtWaitForever);
        if (! SysHandleEvent(&event))
            if (! MenuHandleEvent(0, &event, &error))
                if (! Application_EventHandler(&event))
                   FrmDispatchEvent(&event);
        } while (event.eType != appStopEvent && !StopEvent);
}

UInt32 PilotMain(UInt16 launchCode, MemPtr cmdPBP, UInt16 launchFlags)
{

    Err err;
    if (launchCode == sysAppLaunchCmdNormalLaunch) {
        if ((err = StartApplication()) == 0) {
            EventLoop();
            StopApplication();
        }
    }
    return 0;
}

Boolean pen = false ;
                                                                                #line 2 "TotoF.c"
Coord xpen, ypen ;
                                                                                #line 3 "TotoF.c"
extern Boolean initApp() ;
                                                                                #line 4 "TotoF.c"
extern Boolean iterateApp() ;
                                                                                #line 5 "TotoF.c"

                                                                                #line 6 "TotoF.c"

                                                                                #line 7 "TotoF.c"
static void applicativeLoop(UInt16 almProcCmd, SysAlarmTriggeredParamType *paramP) 
                                                                                #line 8 "TotoF.c"
{ 
                                                                                #line 9 "TotoF.c"
  initApp();
                                                                                #line 10 "TotoF.c"
while(!StopEvent)
                                                                                #line 11 "TotoF.c"
  { 
                                                                                #line 12 "TotoF.c"
    iterateApp() ;
                                                                                #line 13 "TotoF.c"
Application_ProcessMessages(3);
                                                                                #line 14 "TotoF.c"
  }
                                                                                #line 15 "TotoF.c"
}
                                                                                #line 16 "TotoF.c"

                                                                                #line 17 "TotoF.c"
Boolean startApplicativeLoop() {
                                                                                #line 18 "TotoF.c"
   AlmSetProcAlarm(applicativeLoop,0,TimGetSeconds()+1);
                                                                                #line 19 "TotoF.c"
   
                                                                                #line 20 "TotoF.c"
   return success ;
                                                                                #line 21 "TotoF.c"
}
                                                                                #line 22 "TotoF.c"

                                                                                #line 23 "TotoF.c"
EVENT TotoF_OnOpen(EventPtr event)
                                                                                #line 24 "TotoF.c"
{
                                                                                #line 25 "TotoF.c"
   startApplicativeLoop();
                                                                                #line 26 "TotoF.c"
   return true;
                                                                                #line 27 "TotoF.c"
}
                                                                                #line 28 "TotoF.c"

                                                                                #line 29 "TotoF.c"
EVENT TOTO_OnStart(EventPtr event)
                                                                                #line 30 "TotoF.c"
{
                                                                                #line 31 "TotoF.c"
SetScreenMode(8);
                                                                                #line 32 "TotoF.c"
   return false;
                                                                                #line 33 "TotoF.c"
}
                                                                                #line 34 "TotoF.c"

                                                                                #line 35 "TotoF.c"
EVENT TOTO_OnTerminate(EventPtr event)
                                                                                #line 36 "TotoF.c"
{
                                                                                #line 37 "TotoF.c"
WinScreenMode(winScreenModeSetToDefaults,NULL,NULL,NULL,NULL);
                                                                                #line 38 "TotoF.c"
   return false;
                                                                                #line 39 "TotoF.c"
}
                                                                                #line 40 "TotoF.c"

                                                                                #line 41 "TotoF.c"
EVENT TotoF_OnPenDownXY(UInt16 X,UInt16 Y)
                                                                                #line 42 "TotoF.c"
{
                                                                                #line 43 "TotoF.c"
   
                                                                                #line 44 "TotoF.c"
   pen=true;
                                                                                #line 45 "TotoF.c"
   xpen=X-10;
                                                                                #line 46 "TotoF.c"
   ypen=Y-10;
                                                                                #line 47 "TotoF.c"
   return false;
                                                                                #line 48 "TotoF.c"
}
                                                                                #line 49 "TotoF.c"

                                                                                #line 50 "TotoF.c"
EVENT TotoF_OnPenUpXY(UInt16 X,UInt16 Y)
                                                                                #line 51 "TotoF.c"
{
                                                                                #line 52 "TotoF.c"
   
                                                                                #line 53 "TotoF.c"
   pen=false;
                                                                                #line 54 "TotoF.c"
   return false;
                                                                                #line 55 "TotoF.c"
}
                                                                                #line 56 "TotoF.c"

                                                                                #line 57 "TotoF.c"
EVENT TotoF_OnPenMoveXY(UInt16 X,UInt16 Y)
                                                                                #line 58 "TotoF.c"
{
                                                                                #line 59 "TotoF.c"
   
                                                                                #line 60 "TotoF.c"
   xpen=X-10;
                                                                                #line 61 "TotoF.c"
   ypen=Y-10;
                                                                                #line 62 "TotoF.c"
   return false;
                                                                                #line 63 "TotoF.c"
}
#include "v4pi.h"
                                                                                #line 2 "editor.c"

                                                                                #line 3 "editor.c"

                                                                                #line 4 "editor.c"
extern Boolean pen ;
                                                                                #line 5 "editor.c"
extern Coord xpen, ypen ;
                                                                                #line 6 "editor.c"

                                                                                #line 7 "editor.c"
int  i,j,k,dist,mindist;
                                                                                #line 8 "editor.c"

                                                                                #line 9 "editor.c"
ICouche z ;
                                                                                #line 10 "editor.c"

                                                                                #line 11 "editor.c"
PolyP  scene;
                                                                                #line 12 "editor.c"

                                                                                #line 13 "editor.c"
Coord x,y,xvu,yvu,lvu,
                                                                                #line 14 "editor.c"
 xpen1,ypen1, yBouton = 1,
                                                                                #line 15 "editor.c"
 x0,y0,x1,y1,pasGrid ;
                                                                                #line 16 "editor.c"

                                                                                #line 17 "editor.c"
PolyP marques[64] ;
                                                                                #line 18 "editor.c"

                                                                                #line 19 "editor.c"
Boolean pen1 ;
                                                                                #line 20 "editor.c"

                                                                                #line 21 "editor.c"
int iBouton = 0, sel ;
                                                                                #line 22 "editor.c"
int bAjout,bDel,bHauteur,
                                                                                #line 23 "editor.c"
 bScroll,bSave,bCol,bGrid ;
                                                                                #line 24 "editor.c"

                                                                                #line 25 "editor.c"
PolyP
                                                                                #line 26 "editor.c"
 pSel,
                                                                                #line 27 "editor.c"
 pCol,pSelCol,
                                                                                #line 28 "editor.c"
 pGrid,pSelGrid,
                                                                                #line 29 "editor.c"
 pHauteur,pSelHauteur,
                                                                                #line 30 "editor.c"
 polyCourant,brosse,focus;
                                                                                #line 31 "editor.c"

                                                                                #line 32 "editor.c"
SommetP sommetCourant,s;
                                                                                #line 33 "editor.c"

                                                                                #line 34 "editor.c"
ICouche zCourant;
                                                                                #line 35 "editor.c"

                                                                                #line 36 "editor.c"
Couleur couleurCourante, prochaineCouleur ;
                                                                                #line 37 "editor.c"

                                                                                #line 38 "editor.c"
PolyP boutons[16] ;
                                                                                #line 39 "editor.c"

                                                                                #line 40 "editor.c"
int ajouteBouton(Couleur col) {
                                                                                #line 41 "editor.c"
  PolyP bouton = v4pListeAjoutePoly(&scene, relatif, col, 14);
                                                                                #line 42 "editor.c"
  boutons[iBouton] = bouton ;
                                                                                #line 43 "editor.c"
  v4pPoly4S(bouton, largeurLigne - 10, yBouton, largeurLigne - 1, 9 + yBouton);
                                                                                #line 44 "editor.c"
  yBouton+=10;
                                                                                #line 45 "editor.c"
  return iBouton++;
                                                                                #line 46 "editor.c"
}
                                                                                #line 47 "editor.c"

                                                                                #line 48 "editor.c"
void ajusteSel(int s) {
                                                                                #line 49 "editor.c"
  if (sel != s) {
                                                                                #line 50 "editor.c"
    v4pAjustePoly(pSel, 0, (s - sel) * 10, 0, 0);
                                                                                #line 51 "editor.c"
    sel=s;
                                                                                #line 52 "editor.c"
  }
                                                                                #line 53 "editor.c"
}
                                                                                #line 54 "editor.c"

                                                                                #line 55 "editor.c"
typedef enum { idle, push, edit} GuiStatus ;
                                                                                #line 56 "editor.c"

                                                                                #line 57 "editor.c"
int nMarque ;
                                                                                #line 58 "editor.c"
GuiStatus guiStatus ;
                                                                                #line 59 "editor.c"

                                                                                #line 60 "editor.c"

                                                                                #line 61 "editor.c"

                                                                                #line 62 "editor.c"
Boolean initApp() {
                                                                                #line 63 "editor.c"
  v4pFlavorInit(green);
                                                                                #line 64 "editor.c"
  scene = NULL;
                                                                                #line 65 "editor.c"
  v4pPrendScene(&scene);
                                                                                #line 66 "editor.c"
  xvu = -largeurLigne / 2;
                                                                                #line 67 "editor.c"
  yvu = -nbLignes / 2;
                                                                                #line 68 "editor.c"
  lvu = largeurLigne ;
                                                                                #line 69 "editor.c"
  v4pChangeVue(xvu, yvu, xvu + lvu, yvu + lvu);
                                                                                #line 70 "editor.c"
  nMarque=0;
                                                                                #line 71 "editor.c"
  guiStatus= idle;
                                                                                #line 72 "editor.c"
  brosse = NULL;
                                                                                #line 73 "editor.c"
  focus = NULL;
                                                                                #line 74 "editor.c"
  pasGrid = 8;
                                                                                #line 75 "editor.c"
  bAjout = ajouteBouton(red);
                                                                                #line 76 "editor.c"
  bDel = ajouteBouton(gray);
                                                                                #line 77 "editor.c"
  bHauteur = ajouteBouton(blue);
                                                                                #line 78 "editor.c"
  bScroll = ajouteBouton(yellow);
                                                                                #line 79 "editor.c"
  bCol = ajouteBouton(black);
                                                                                #line 80 "editor.c"
  bGrid = ajouteBouton(cyan);
                                                                                #line 81 "editor.c"
  
                                                                                #line 82 "editor.c"
  sel = 0 ;
                                                                                #line 83 "editor.c"
  couleurCourante = black ;
                                                                                #line 84 "editor.c"
  zCourant = 7 ;
                                                                                #line 85 "editor.c"
  polyCourant = NULL;
                                                                                #line 86 "editor.c"
  
                                                                                #line 87 "editor.c"
  pSel = v4pListeAjoutePoly(&scene, relatif, black, 13);
                                                                                #line 88 "editor.c"
  v4pPoly4S(pSel, largeurLigne - 11, 0 , largeurLigne, 11);
                                                                                #line 89 "editor.c"
  pCol=v4pListeAjoutePoly(&scene, relatif | inactif,black, 14);
                                                                                #line 90 "editor.c"
  v4pPoly4S(pCol, -xvu - 20, -yvu - 20, -xvu + 20, -yvu + 20);
                                                                                #line 91 "editor.c"
  pSelCol=v4pPolyAjouteSubPoly(pCol, relatif, black, 15);
                                                                                #line 92 "editor.c"
  v4pPoly4S(pSelCol, -xvu - 18, -yvu - 18, -xvu + 18, -yvu + 18);
                                                                                #line 93 "editor.c"
  pHauteur=v4pListeAjoutePoly(&scene, relatif | inactif, black, 14);
                                                                                #line 94 "editor.c"
  v4pPoly4S(pHauteur, -xvu - 3, -yvu - 17, -xvu + 3, -yvu + 17);
                                                                                #line 95 "editor.c"
  pSelHauteur=v4pPolyAjouteSubPoly(pHauteur, relatif, red, 15);
                                                                                #line 96 "editor.c"
  v4pPoly4S(pSelHauteur, -xvu - 2, -yvu - 1, -xvu + 2, -yvu + 1);
                                                                                #line 97 "editor.c"
  pGrid=v4pListeAjoutePoly(&scene, relatif | inactif, black, 14);
                                                                                #line 98 "editor.c"
  v4pPoly4S(pGrid, -xvu - 9, -yvu - 9, -xvu + 9, -yvu + 9);
                                                                                #line 99 "editor.c"
  pSelGrid=v4pPolyAjouteSubPoly(pGrid, relatif, red, 15);
                                                                                #line 100 "editor.c"
  v4pPoly4S(pSelGrid, -xvu - 2, -yvu - 2, -xvu + 2, -yvu + 2);
                                                                                #line 101 "editor.c"
  return success ;
                                                                                #line 102 "editor.c"
}
                                                                                #line 103 "editor.c"

                                                                                #line 104 "editor.c"
Coord aligne(Coord x) {
                                                                                #line 105 "editor.c"
  if (pasGrid <= 1)
                                                                                #line 106 "editor.c"
     return x ;
                                                                                #line 107 "editor.c"
  else if (x>0)
                                                                                #line 108 "editor.c"
     return ((x + pasGrid / 2) / pasGrid) * pasGrid;
                                                                                #line 109 "editor.c"
  else
                                                                                #line 110 "editor.c"
     return ((x - pasGrid / 2) / pasGrid) * pasGrid;
                                                                                #line 111 "editor.c"
}
                                                                                #line 112 "editor.c"

                                                                                #line 113 "editor.c"
Boolean iterateApp() {
                                                                                #line 114 "editor.c"
  Coord  pasGrid0, pasGridPrec,
                                                                                #line 115 "editor.c"
     xs, ys,
                                                                                #line 116 "editor.c"
     xi, yi,
                                                                                #line 117 "editor.c"
     t1, t2 ;
                                                                                #line 118 "editor.c"
  ICouche z0, zPrec ;
                                                                                #line 119 "editor.c"
  int selPrec ;
                                                                                #line 120 "editor.c"

                                                                                #line 121 "editor.c"
  if (true) {
                                                                                #line 122 "editor.c"
    
                                                                                #line 123 "editor.c"
    v4pAffiche();
                                                                                #line 124 "editor.c"
    if (pen) {
                                                                                #line 125 "editor.c"
      if (pen1) {
                                                                                #line 126 "editor.c"
        xpen = (2 * xpen + xpen1) / 3;
                                                                                #line 127 "editor.c"
        ypen = (2 * ypen + ypen1) / 3;
                                                                                #line 128 "editor.c"
      }
                                                                                #line 129 "editor.c"
      xpen1 = xpen;
                                                                                #line 130 "editor.c"
      ypen1 = ypen;
                                                                                #line 131 "editor.c"

                                                                                #line 132 "editor.c"
      v4pVueEnAbsolu(xpen, ypen, &x, &y);
                                                                                #line 133 "editor.c"
      xs = aligne(x);
                                                                                #line 134 "editor.c"
      ys = aligne(y);
                                                                                #line 135 "editor.c"

                                                                                #line 136 "editor.c"
      if (guiStatus == push) { 
                                                                                #line 137 "editor.c"
        if (sel == bGrid) {
                                                                                #line 138 "editor.c"
         pasGridPrec = pasGrid;
                                                                                #line 139 "editor.c"
         pasGrid = 1 << ((iabs(ypen-y0) / 4) % 4) ;
                                                                                #line 140 "editor.c"
         if (pasGrid != pasGridPrec)
                                                                                #line 141 "editor.c"
            v4pAjustePoly(pSelGrid, pasGrid - pasGridPrec, pasGrid - pasGridPrec, 0, 0);
                                                                                #line 142 "editor.c"
        } else if (sel == bCol) {
                                                                                #line 143 "editor.c"
         prochaineCouleur = (((iabs(ypen - y0) + iabs(xpen - x0))) + couleurCourante) % 255;
                                                                                #line 144 "editor.c"
         v4pPolyPrendCouleur(pSelCol, prochaineCouleur);
                                                                                #line 145 "editor.c"
        } else if (sel == bHauteur) {
                                                                                #line 146 "editor.c"
         zPrec = zCourant;
                                                                                #line 147 "editor.c"
         zCourant = (z0 + (iabs(ypen - y0) / 4)) % 15 ;
                                                                                #line 148 "editor.c"
         if (zPrec != zCourant) v4pAjustePoly(pSelHauteur, 0, (zPrec - zCourant) * 2, 0, 0);
                                                                                #line 149 "editor.c"
        }
                                                                                #line 150 "editor.c"
      } else if (guiStatus == edit) { 
                                                                                #line 151 "editor.c"
        if (brosse) {
                                                                                #line 152 "editor.c"
          if (sel == bScroll) {
                                                                                #line 153 "editor.c"
            v4pListeSuprPoly(&scene, brosse);
                                                                                #line 154 "editor.c"
            brosse = NULL;
                                                                                #line 155 "editor.c"
          } else {
                                                                                #line 156 "editor.c"
            v4pAjustePoly(brosse, xpen - x0, ypen - y0, 0, 0);
                                                                                #line 157 "editor.c"
            x0 = xpen;
                                                                                #line 158 "editor.c"
            y0 = ypen;
                                                                                #line 159 "editor.c"
          }
                                                                                #line 160 "editor.c"
        }
                                                                                #line 161 "editor.c"
        if (sel == bAjout) {
                                                                                #line 162 "editor.c"
          if (sommetCourant) {
                                                                                #line 163 "editor.c"

                                                                                #line 164 "editor.c"

                                                                                #line 165 "editor.c"

                                                                                #line 166 "editor.c"
             v4pAjustePoly(marques[nMarque],               xs - sommetCourant->x,               ys - sommetCourant->y,               0, 0);
                                                                                #line 167 "editor.c"
             v4pPolyBougeSommet(polyCourant, sommetCourant, xs, ys);
                                                                                #line 168 "editor.c"
          }
                                                                                #line 169 "editor.c"
        } else if (sel==bGrid && sommetCourant) {
                                                                                #line 170 "editor.c"
          v4pPolyBougeSommet(focus, sommetCourant, xs, ys);
                                                                                #line 171 "editor.c"
          
                                                                                #line 172 "editor.c"
        } else if (collisions[2].q > 0) {
                                                                                #line 173 "editor.c"
          if (sel == bScroll) {
                                                                                #line 174 "editor.c"
            focus = collisions[2].poly;
                                                                                #line 175 "editor.c"
            x0 = xs;
                                                                                #line 176 "editor.c"
            y0 = ys;
                                                                                #line 177 "editor.c"
          } else if (sel == bGrid) {
                                                                                #line 178 "editor.c"
            if (!focus) {
                                                                                #line 179 "editor.c"
              focus = collisions[2].poly;
                                                                                #line 180 "editor.c"
              s = v4pPolyDonneSommets(focus);
                                                                                #line 181 "editor.c"
              mindist = estimeDist(s->x - x, s->y - y);
                                                                                #line 182 "editor.c"
              sommetCourant = s;
                                                                                #line 183 "editor.c"
              s = s->suivant;
                                                                                #line 184 "editor.c"
              while (s) {
                                                                                #line 185 "editor.c"
                dist = estimeDist(s->x - x, s->y - y);
                                                                                #line 186 "editor.c"
                if (dist < mindist) {
                                                                                #line 187 "editor.c"
                  mindist = dist;
                                                                                #line 188 "editor.c"
                  sommetCourant = s;
                                                                                #line 189 "editor.c"
                }
                                                                                #line 190 "editor.c"
                s = s->suivant;
                                                                                #line 191 "editor.c"
              }
                                                                                #line 192 "editor.c"
            }
                                                                                #line 193 "editor.c"
          } else
                                                                                #line 194 "editor.c"
            focus =collisions[2].poly;
                                                                                #line 195 "editor.c"
        } else if (sel == bScroll) { 
                                                                                #line 196 "editor.c"
          if (focus) {
                                                                                #line 197 "editor.c"
            v4pAjustePoly(focus, xs - x0, ys - y0, 0, 0);
                                                                                #line 198 "editor.c"
            x0 = xs; y0 = ys;
                                                                                #line 199 "editor.c"
          } else {
                                                                                #line 200 "editor.c"

                                                                                #line 201 "editor.c"

                                                                                #line 202 "editor.c"

                                                                                #line 203 "editor.c"

                                                                                #line 204 "editor.c"
            v4pChangeVue(              aligne(xvu + xpen - x0),              aligne(yvu + ypen - y0),              aligne(xvu + xpen - x0) + lvu,              aligne(yvu + ypen - y0) + lvu);
                                                                                #line 205 "editor.c"
          }
                                                                                #line 206 "editor.c"
        }
                                                                                #line 207 "editor.c"
      } else { 
                                                                                #line 208 "editor.c"
        if (xpen > largeurLigne - 10 && ypen < yBouton) { 
                                                                                #line 209 "editor.c"
         selPrec = sel;
                                                                                #line 210 "editor.c"
         ajusteSel(ypen / 10);
                                                                                #line 211 "editor.c"
         if (selPrec == bAjout) {
                                                                                #line 212 "editor.c"
           if (polyCourant && nMarque <= 2)
                                                                                #line 213 "editor.c"
             v4pListeSuprPoly(&scene, polyCourant);
                                                                                #line 214 "editor.c"
           
                                                                                #line 215 "editor.c"
           while (nMarque) {
                                                                                #line 216 "editor.c"
             nMarque--;
                                                                                #line 217 "editor.c"
             if (nMarque < 20) v4pListeSuprPoly(&scene, marques[nMarque]);
                                                                                #line 218 "editor.c"
           }
                                                                                #line 219 "editor.c"
         }
                                                                                #line 220 "editor.c"
         nMarque = 0;
                                                                                #line 221 "editor.c"
         focus = NULL;
                                                                                #line 222 "editor.c"
         polyCourant = NULL;
                                                                                #line 223 "editor.c"
         sommetCourant = NULL;
                                                                                #line 224 "editor.c"
         x0 = xpen;
                                                                                #line 225 "editor.c"
         y0 = ypen;
                                                                                #line 226 "editor.c"
         if (sel == bCol) {
                                                                                #line 227 "editor.c"
           v4pActivePoly(pCol);
                                                                                #line 228 "editor.c"
         } else if (sel == bScroll) {
                                                                                #line 229 "editor.c"
         } else if (sel == bHauteur) {
                                                                                #line 230 "editor.c"
           v4pActivePoly(pHauteur);
                                                                                #line 231 "editor.c"
           z0 = zCourant;
                                                                                #line 232 "editor.c"
         } else if (sel == bGrid) {
                                                                                #line 233 "editor.c"
           v4pActivePoly(pGrid);
                                                                                #line 234 "editor.c"
           pasGrid0 = pasGrid;
                                                                                #line 235 "editor.c"
           y0 = 4 * (ypen - bitMax(pasGrid));
                                                                                #line 236 "editor.c"
         }
                                                                                #line 237 "editor.c"
         guiStatus = push;
                                                                                #line 238 "editor.c"
        } else { 
                                                                                #line 239 "editor.c"
         if (sel == bAjout) {
                                                                                #line 240 "editor.c"
           if (nMarque == 0) {
                                                                                #line 241 "editor.c"
             polyCourant = v4pListeAjoutePoly(&scene,plein, couleurCourante, zCourant);
                                                                                #line 242 "editor.c"
             v4pPolyCognable(polyCourant, 0);
                                                                                #line 243 "editor.c"
           }
                                                                                #line 244 "editor.c"
           sommetCourant = v4pPolyAjouteSommet(polyCourant, xs, ys);
                                                                                #line 245 "editor.c"
           if (nMarque < 20) {
                                                                                #line 246 "editor.c"
             marques[nMarque] = v4pListeAjoutePoly(&scene,plein, couleurCourante,14);
                                                                                #line 247 "editor.c"
             v4pPoly4S(marques[nMarque], xs - 1 , ys - 1 , xs + 1 , ys + 1);
                                                                                #line 248 "editor.c"
           }
                                                                                #line 249 "editor.c"
         }
                                                                                #line 250 "editor.c"
         brosse = v4pListeAjoutePoly(&scene, relatif, black, 15);
                                                                                #line 251 "editor.c"
         v4pPoly4S(brosse, xpen - 1 , ypen - 1 , xpen + 1 , ypen + 1);
                                                                                #line 252 "editor.c"
         v4pPolyCognable(brosse, 2);
                                                                                #line 253 "editor.c"
         x0 = xpen;
                                                                                #line 254 "editor.c"
         y0 = ypen;
                                                                                #line 255 "editor.c"
         guiStatus = edit;
                                                                                #line 256 "editor.c"
       }
                                                                                #line 257 "editor.c"
     }
                                                                                #line 258 "editor.c"
    } else { 
                                                                                #line 259 "editor.c"
      if (guiStatus == push) { 
                                                                                #line 260 "editor.c"
       v4pDesactivePoly(pCol);
                                                                                #line 261 "editor.c"
       if (sel == bCol) v4pPolyPrendCouleur(boutons[bCol], (couleurCourante = prochaineCouleur));
                                                                                #line 262 "editor.c"
       if (sel == bHauteur) v4pDesactivePoly(pHauteur);
                                                                                #line 263 "editor.c"
       if (sel == bGrid) v4pDesactivePoly(pGrid);
                                                                                #line 264 "editor.c"
      } else if (guiStatus==edit) { 
                                                                                #line 265 "editor.c"
       if (sel == bAjout) {
                                                                                #line 266 "editor.c"
         nMarque++;
                                                                                #line 267 "editor.c"
       } else if (focus) {
                                                                                #line 268 "editor.c"
         if (sel == bCol) v4pPolyPrendCouleur(focus, couleurCourante);
                                                                                #line 269 "editor.c"
         if (sel == bDel) v4pListeSuprPoly(&scene, focus);
                                                                                #line 270 "editor.c"
         if (sel == bHauteur) v4pAjustePoly(focus, 0, 0, 0, zCourant - v4pPolyDonneZ(focus));
                                                                                #line 271 "editor.c"
         focus = NULL;
                                                                                #line 272 "editor.c"
         sommetCourant = NULL;
                                                                                #line 273 "editor.c"
       } else if (sel == bScroll && !focus) {
                                                                                #line 274 "editor.c"
         xvu = aligne(xvu + (xpen - x0));
                                                                                #line 275 "editor.c"
         yvu = aligne(yvu + (ypen - y0));
                                                                                #line 276 "editor.c"
         v4pChangeVue(xvu, yvu, xvu + lvu, yvu + lvu);
                                                                                #line 277 "editor.c"
       }
                                                                                #line 278 "editor.c"
       if (brosse) {
                                                                                #line 279 "editor.c"
         v4pListeSuprPoly(&scene, brosse);
                                                                                #line 280 "editor.c"
         brosse = NULL;
                                                                                #line 281 "editor.c"
       }
                                                                                #line 282 "editor.c"
      }
                                                                                #line 283 "editor.c"
    guiStatus = idle;
                                                                                #line 284 "editor.c"
  }
                                                                                #line 285 "editor.c"
  pen1 = pen;
                                                                                #line 286 "editor.c"
 }
                                                                                #line 287 "editor.c"
 return success ;
                                                                                #line 288 "editor.c"
}
                                                                                #line 289 "editor.c"

                                                                                #line 290 "editor.c"

