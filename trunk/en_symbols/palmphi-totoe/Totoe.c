#include <PalmOS.h>
#include <C:\cygwin\usr\src\v4p\english_code\palmphi-totoe\Totoe.h>
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
ILayer z ;
                                                                                #line 10 "editor.c"

                                                                                #line 11 "editor.c"
PolygonP  scene;
                                                                                #line 12 "editor.c"

                                                                                #line 13 "editor.c"
Coord x,y,xvu,yvu,lvu,
                                                                                #line 14 "editor.c"
 xpen1,ypen1, yButton = 1,
                                                                                #line 15 "editor.c"
 x0,y0,x1,y1,stepGrid ;
                                                                                #line 16 "editor.c"

                                                                                #line 17 "editor.c"
PolygonP spots[64] ;
                                                                                #line 18 "editor.c"

                                                                                #line 19 "editor.c"
Boolean pen1 ;
                                                                                #line 20 "editor.c"

                                                                                #line 21 "editor.c"
int iButton = 0, sel ;
                                                                                #line 22 "editor.c"
int bAddition,bDel,bLayer,
                                                                                #line 23 "editor.c"
 bScroll,bSave,bCol,bGrid ;
                                                                                #line 24 "editor.c"

                                                                                #line 25 "editor.c"
PolygonP
                                                                                #line 26 "editor.c"
 pSel,
                                                                                #line 27 "editor.c"
 pCol,pSelCol,
                                                                                #line 28 "editor.c"
 pGrid,pSelGrid,
                                                                                #line 29 "editor.c"
 pLayer,pSelLayer,
                                                                                #line 30 "editor.c"
 currentPolygon,brush,focus;
                                                                                #line 31 "editor.c"

                                                                                #line 32 "editor.c"
PointP currentPoint, s ;
                                                                                #line 33 "editor.c"

                                                                                #line 34 "editor.c"
ILayer currentZ;
                                                                                #line 35 "editor.c"

                                                                                #line 36 "editor.c"
Color currentColor, nextColor ;
                                                                                #line 37 "editor.c"

                                                                                #line 38 "editor.c"
PolygonP buttons[16] ;
                                                                                #line 39 "editor.c"

                                                                                #line 40 "editor.c"
int addButton(Color col) {
                                                                                #line 41 "editor.c"
  PolygonP button = v4pListAddPolygon(&scene, relative, col, 14);
                                                                                #line 42 "editor.c"
  buttons[iButton] = button ;
                                                                                #line 43 "editor.c"
  v4pPolygonRect(button, lineWidth - 10, yButton, lineWidth - 1, 9 + yButton);
                                                                                #line 44 "editor.c"
  yButton+=10;
                                                                                #line 45 "editor.c"
  return iButton++;
                                                                                #line 46 "editor.c"
}
                                                                                #line 47 "editor.c"

                                                                                #line 48 "editor.c"
void ajusteSel(int s) {
                                                                                #line 49 "editor.c"
  if (sel != s) {
                                                                                #line 50 "editor.c"
    v4pPolygonTransform(pSel, 0, (s - sel) * 10, 0, 0);
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
int spotNb ;
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
  v4pSetScene(&scene);
                                                                                #line 66 "editor.c"
  xvu = -lineWidth / 2;
                                                                                #line 67 "editor.c"
  yvu = -lineNb / 2;
                                                                                #line 68 "editor.c"
  lvu = lineWidth ;
                                                                                #line 69 "editor.c"
  v4pSetView(xvu, yvu, xvu + lvu, yvu + lvu);
                                                                                #line 70 "editor.c"
  spotNb=0;
                                                                                #line 71 "editor.c"
  guiStatus= idle;
                                                                                #line 72 "editor.c"
  brush = NULL;
                                                                                #line 73 "editor.c"
  focus = NULL;
                                                                                #line 74 "editor.c"
  stepGrid = 8;
                                                                                #line 75 "editor.c"
  bAddition = addButton(red);
                                                                                #line 76 "editor.c"
  bDel = addButton(gray);
                                                                                #line 77 "editor.c"
  bLayer = addButton(blue);
                                                                                #line 78 "editor.c"
  bScroll = addButton(yellow);
                                                                                #line 79 "editor.c"
  bCol = addButton(black);
                                                                                #line 80 "editor.c"
  bGrid = addButton(cyan);
                                                                                #line 81 "editor.c"
  
                                                                                #line 82 "editor.c"
  sel = 0 ;
                                                                                #line 83 "editor.c"
  currentColor = black ;
                                                                                #line 84 "editor.c"
  currentZ = 7 ;
                                                                                #line 85 "editor.c"
  currentPolygon = NULL;
                                                                                #line 86 "editor.c"
  
                                                                                #line 87 "editor.c"
  pSel = v4pListAddPolygon(&scene, relative, black, 13);
                                                                                #line 88 "editor.c"
  v4pPolygonRect(pSel, lineWidth - 11, 0 , lineWidth, 11);
                                                                                #line 89 "editor.c"
  pCol=v4pListAddPolygon(&scene, relative | disabled,black, 14);
                                                                                #line 90 "editor.c"
  v4pPolygonRect(pCol, -xvu - 20, -yvu - 20, -xvu + 20, -yvu + 20);
                                                                                #line 91 "editor.c"
  pSelCol=v4pPolygonAddNewSub(pCol, relative, black, 15);
                                                                                #line 92 "editor.c"
  v4pPolygonRect(pSelCol, -xvu - 18, -yvu - 18, -xvu + 18, -yvu + 18);
                                                                                #line 93 "editor.c"
  pLayer=v4pListAddPolygon(&scene, relative | disabled, black, 14);
                                                                                #line 94 "editor.c"
  v4pPolygonRect(pLayer, -xvu - 3, -yvu - 17, -xvu + 3, -yvu + 17);
                                                                                #line 95 "editor.c"
  pSelLayer=v4pPolygonAddNewSub(pLayer, relative, red, 15);
                                                                                #line 96 "editor.c"
  v4pPolygonRect(pSelLayer, -xvu - 2, -yvu - 1, -xvu + 2, -yvu + 1);
                                                                                #line 97 "editor.c"
  pGrid=v4pListAddPolygon(&scene, relative | disabled, black, 14);
                                                                                #line 98 "editor.c"
  v4pPolygonRect(pGrid, -xvu - 9, -yvu - 9, -xvu + 9, -yvu + 9);
                                                                                #line 99 "editor.c"
  pSelGrid=v4pPolygonAddNewSub(pGrid, relative, red, 15);
                                                                                #line 100 "editor.c"
  v4pPolygonRect(pSelGrid, -xvu - 2, -yvu - 2, -xvu + 2, -yvu + 2);
                                                                                #line 101 "editor.c"
  return success ;
                                                                                #line 102 "editor.c"
}
                                                                                #line 103 "editor.c"

                                                                                #line 104 "editor.c"
Coord align(Coord x) {
                                                                                #line 105 "editor.c"
  if (stepGrid <= 1)
                                                                                #line 106 "editor.c"
     return x ;
                                                                                #line 107 "editor.c"
  else if (x>0)
                                                                                #line 108 "editor.c"
     return ((x + stepGrid / 2) / stepGrid) * stepGrid;
                                                                                #line 109 "editor.c"
  else
                                                                                #line 110 "editor.c"
     return ((x - stepGrid / 2) / stepGrid) * stepGrid;
                                                                                #line 111 "editor.c"
}
                                                                                #line 112 "editor.c"

                                                                                #line 113 "editor.c"
Boolean iterateApp() {
                                                                                #line 114 "editor.c"
  Coord  stepGrid0, stepGridPrec,
                                                                                #line 115 "editor.c"
     xs, ys,
                                                                                #line 116 "editor.c"
     xi, yi,
                                                                                #line 117 "editor.c"
     t1, t2 ;
                                                                                #line 118 "editor.c"
  ILayer z0, precZ ;
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
      v4pViewToAbsolute(xpen, ypen, &x, &y);
                                                                                #line 133 "editor.c"
      xs = align(x);
                                                                                #line 134 "editor.c"
      ys = align(y);
                                                                                #line 135 "editor.c"

                                                                                #line 136 "editor.c"
      if (guiStatus == push) { 
                                                                                #line 137 "editor.c"
        if (sel == bGrid) {
                                                                                #line 138 "editor.c"
         stepGridPrec = stepGrid;
                                                                                #line 139 "editor.c"
         stepGrid = 1 << ((iabs(ypen-y0) / 4) % 4) ;
                                                                                #line 140 "editor.c"
         if (stepGrid != stepGridPrec)
                                                                                #line 141 "editor.c"
            v4pPolygonTransform(pSelGrid, stepGrid - stepGridPrec, stepGrid - stepGridPrec, 0, 0);
                                                                                #line 142 "editor.c"
        } else if (sel == bCol) {
                                                                                #line 143 "editor.c"
         nextColor = (((iabs(ypen - y0) + iabs(xpen - x0))) + currentColor) % 255;
                                                                                #line 144 "editor.c"
         v4pPolygonSetColor(pSelCol, nextColor);
                                                                                #line 145 "editor.c"
        } else if (sel == bLayer) {
                                                                                #line 146 "editor.c"
         precZ = currentZ;
                                                                                #line 147 "editor.c"
         currentZ = (z0 + (iabs(ypen - y0) / 4)) % 15 ;
                                                                                #line 148 "editor.c"
         if (precZ != currentZ) v4pPolygonTransform(pSelLayer, 0, (precZ - currentZ) * 2, 0, 0);
                                                                                #line 149 "editor.c"
        }
                                                                                #line 150 "editor.c"
      } else if (guiStatus == edit) { 
                                                                                #line 151 "editor.c"
        if (brush) {
                                                                                #line 152 "editor.c"
          if (sel == bScroll) {
                                                                                #line 153 "editor.c"
            v4pListDelPolygon(&scene, brush);
                                                                                #line 154 "editor.c"
            brush = NULL;
                                                                                #line 155 "editor.c"
          } else {
                                                                                #line 156 "editor.c"
            v4pPolygonTransform(brush, xpen - x0, ypen - y0, 0, 0);
                                                                                #line 157 "editor.c"
            x0 = xpen;
                                                                                #line 158 "editor.c"
            y0 = ypen;
                                                                                #line 159 "editor.c"
          }
                                                                                #line 160 "editor.c"
        }
                                                                                #line 161 "editor.c"
        if (sel == bAddition) {
                                                                                #line 162 "editor.c"
          if (currentPoint) {
                                                                                #line 163 "editor.c"

                                                                                #line 164 "editor.c"

                                                                                #line 165 "editor.c"

                                                                                #line 166 "editor.c"
             v4pPolygonTransform(spots[spotNb],               xs - currentPoint->x,               ys - currentPoint->y,               0, 0);
                                                                                #line 167 "editor.c"
             v4pPolygonMovePoint(currentPolygon, currentPoint, xs, ys);
                                                                                #line 168 "editor.c"
          }
                                                                                #line 169 "editor.c"
        } else if (sel==bGrid && currentPoint) {
                                                                                #line 170 "editor.c"
          v4pPolygonMovePoint(focus, currentPoint, xs, ys);
                                                                                #line 171 "editor.c"
          
                                                                                #line 172 "editor.c"
        } else if (collides[2].q > 0) {
                                                                                #line 173 "editor.c"
          if (sel == bScroll) {
                                                                                #line 174 "editor.c"
            focus = collides[2].poly;
                                                                                #line 175 "editor.c"
            x0 = xs;
                                                                                #line 176 "editor.c"
            y0 = ys;
                                                                                #line 177 "editor.c"
          } else if (sel == bGrid) {
                                                                                #line 178 "editor.c"
            if (!focus) {
                                                                                #line 179 "editor.c"
              focus = collides[2].poly;
                                                                                #line 180 "editor.c"
              s = v4pPolygonGetPoints(focus);
                                                                                #line 181 "editor.c"
              mindist = gaugeDist(s->x - x, s->y - y);
                                                                                #line 182 "editor.c"
              currentPoint = s;
                                                                                #line 183 "editor.c"
              s = s->next;
                                                                                #line 184 "editor.c"
              while (s) {
                                                                                #line 185 "editor.c"
                dist = gaugeDist(s->x - x, s->y - y);
                                                                                #line 186 "editor.c"
                if (dist < mindist) {
                                                                                #line 187 "editor.c"
                  mindist = dist;
                                                                                #line 188 "editor.c"
                  currentPoint = s;
                                                                                #line 189 "editor.c"
                }
                                                                                #line 190 "editor.c"
                s = s->next;
                                                                                #line 191 "editor.c"
              }
                                                                                #line 192 "editor.c"
            }
                                                                                #line 193 "editor.c"
          } else
                                                                                #line 194 "editor.c"
            focus =collides[2].poly;
                                                                                #line 195 "editor.c"
        } else if (sel == bScroll) { 
                                                                                #line 196 "editor.c"
          if (focus) {
                                                                                #line 197 "editor.c"
            v4pPolygonTransform(focus, xs - x0, ys - y0, 0, 0);
                                                                                #line 198 "editor.c"
            x0 = xs; y0 = ys;
                                                                                #line 199 "editor.c"
          } else {
                                                                                #line 200 "editor.c"

                                                                                #line 201 "editor.c"

                                                                                #line 202 "editor.c"

                                                                                #line 203 "editor.c"

                                                                                #line 204 "editor.c"
            v4pSetView(              align(xvu + xpen - x0),              align(yvu + ypen - y0),              align(xvu + xpen - x0) + lvu,              align(yvu + ypen - y0) + lvu);
                                                                                #line 205 "editor.c"
          }
                                                                                #line 206 "editor.c"
        }
                                                                                #line 207 "editor.c"
      } else { 
                                                                                #line 208 "editor.c"
        if (xpen > lineWidth - 10 && ypen < yButton) { 
                                                                                #line 209 "editor.c"
         selPrec = sel;
                                                                                #line 210 "editor.c"
         ajusteSel(ypen / 10);
                                                                                #line 211 "editor.c"
         if (selPrec == bAddition) {
                                                                                #line 212 "editor.c"
           if (currentPolygon && spotNb <= 2)
                                                                                #line 213 "editor.c"
             v4pListDelPolygon(&scene, currentPolygon);
                                                                                #line 214 "editor.c"
           
                                                                                #line 215 "editor.c"
           while (spotNb) {
                                                                                #line 216 "editor.c"
             spotNb--;
                                                                                #line 217 "editor.c"
             if (spotNb < 20) v4pListDelPolygon(&scene, spots[spotNb]);
                                                                                #line 218 "editor.c"
           }
                                                                                #line 219 "editor.c"
         }
                                                                                #line 220 "editor.c"
         spotNb = 0;
                                                                                #line 221 "editor.c"
         focus = NULL;
                                                                                #line 222 "editor.c"
         currentPolygon = NULL;
                                                                                #line 223 "editor.c"
         currentPoint = NULL;
                                                                                #line 224 "editor.c"
         x0 = xpen;
                                                                                #line 225 "editor.c"
         y0 = ypen;
                                                                                #line 226 "editor.c"
         if (sel == bCol) {
                                                                                #line 227 "editor.c"
           v4pPolygonEnable(pCol);
                                                                                #line 228 "editor.c"
         } else if (sel == bScroll) {
                                                                                #line 229 "editor.c"
         } else if (sel == bLayer) {
                                                                                #line 230 "editor.c"
           v4pPolygonEnable(pLayer);
                                                                                #line 231 "editor.c"
           z0 = currentZ;
                                                                                #line 232 "editor.c"
         } else if (sel == bGrid) {
                                                                                #line 233 "editor.c"
           v4pPolygonEnable(pGrid);
                                                                                #line 234 "editor.c"
           stepGrid0 = stepGrid;
                                                                                #line 235 "editor.c"
           y0 = 4 * (ypen - floorLog2(stepGrid));
                                                                                #line 236 "editor.c"
         }
                                                                                #line 237 "editor.c"
         guiStatus = push;
                                                                                #line 238 "editor.c"
        } else { 
                                                                                #line 239 "editor.c"
         if (sel == bAddition) {
                                                                                #line 240 "editor.c"
           if (spotNb == 0) {
                                                                                #line 241 "editor.c"
             currentPolygon = v4pListAddPolygon(&scene,standard, currentColor, currentZ);
                                                                                #line 242 "editor.c"
             v4pPolygonConcrete(currentPolygon, 0);
                                                                                #line 243 "editor.c"
           }
                                                                                #line 244 "editor.c"
           currentPoint = v4pPolygonAddPoint(currentPolygon, xs, ys);
                                                                                #line 245 "editor.c"
           if (spotNb < 20) {
                                                                                #line 246 "editor.c"
             spots[spotNb] = v4pListAddPolygon(&scene,standard, currentColor,14);
                                                                                #line 247 "editor.c"
             v4pPolygonRect(spots[spotNb], xs - 1 , ys - 1 , xs + 1 , ys + 1);
                                                                                #line 248 "editor.c"
           }
                                                                                #line 249 "editor.c"
         }
                                                                                #line 250 "editor.c"
         brush = v4pListAddPolygon(&scene, relative, black, 15);
                                                                                #line 251 "editor.c"
         v4pPolygonRect(brush, xpen - 1 , ypen - 1 , xpen + 1 , ypen + 1);
                                                                                #line 252 "editor.c"
         v4pPolygonConcrete(brush, 2);
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
       v4pPolygonDisable(pCol);
                                                                                #line 261 "editor.c"
       if (sel == bCol) v4pPolygonSetColor(buttons[bCol], (currentColor = nextColor));
                                                                                #line 262 "editor.c"
       if (sel == bLayer) v4pPolygonDisable(pLayer);
                                                                                #line 263 "editor.c"
       if (sel == bGrid) v4pPolygonDisable(pGrid);
                                                                                #line 264 "editor.c"
      } else if (guiStatus==edit) { 
                                                                                #line 265 "editor.c"
       if (sel == bAddition) {
                                                                                #line 266 "editor.c"
         spotNb++;
                                                                                #line 267 "editor.c"
       } else if (focus) {
                                                                                #line 268 "editor.c"
         if (sel == bCol) v4pPolygonSetColor(focus, currentColor);
                                                                                #line 269 "editor.c"
         if (sel == bDel) v4pListDelPolygon(&scene, focus);
                                                                                #line 270 "editor.c"
         if (sel == bLayer) v4pPolygonTransform(focus, 0, 0, 0, currentZ - v4pPolygonGetZ(focus));
                                                                                #line 271 "editor.c"
         focus = NULL;
                                                                                #line 272 "editor.c"
         currentPoint = NULL;
                                                                                #line 273 "editor.c"
       } else if (sel == bScroll && !focus) {
                                                                                #line 274 "editor.c"
         xvu = align(xvu + (xpen - x0));
                                                                                #line 275 "editor.c"
         yvu = align(yvu + (ypen - y0));
                                                                                #line 276 "editor.c"
         v4pSetView(xvu, yvu, xvu + lvu, yvu + lvu);
                                                                                #line 277 "editor.c"
       }
                                                                                #line 278 "editor.c"
       if (brush) {
                                                                                #line 279 "editor.c"
         v4pListDelPolygon(&scene, brush);
                                                                                #line 280 "editor.c"
         brush = NULL;
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

