#include "gamemachine.h"
#include "v4p.h"
#include "v4pi.h"
#include "v4pserial.h"

#define STRESS_AMOUNT 48
PolygonP  scene;
PolygonP pCol;
PolygonP pColMatrix[STRESS_AMOUNT][STRESS_AMOUNT];

int iu = 0;
int diu = STRESS_AMOUNT/10;
int liu  = 3;
int iu2 = 0;

Boolean gmOnInit() {
  int j, k;
  scene = NULL;

  v4pDisplayInit(1, 1);
  v4pInit();

  v4pSetScene(&scene);
  v4pSetBGColor(blue);
 
  pCol = v4pPolygonNew(absolute, red, 0);
  //v4pPolygonDecodeSVGPath(pCol, "M 478.1117,4.99999  L 490.52087,43.198877 L 530.68482,43.196598 L 498.19016,66.802524 \
  //        L 510.60367,105.00001 L 478.1117,81.390382 L 445.61972,105.00001 L 458.03324,66.80253 \
  //        L 425.53858,43.196598 L 465.70253,43.198877 L 478.1117,4.99999 z", 700);
  v4pPolygonDecodeSVGPath(pCol, "m277,318l-10,-69l-18,-58l-17,-36l-19,-25l14,-7l30,30l26,49l21,65l18,53l-45,-2z\
       m-95.5,-194l-37.5,16l-40,40l-31,61l3,-69l34,-57l63,-15l23,6l-14.5,18z\
       m-9.5,-40l-46,-9l-60,9l-40,26l-22,27l16,-48l57,-35l59,0l51,17l15,21l-30,-8z\
       m46,7l8,-46l26,-32l32,-8l41,11l25,38l0,28l-30,-27l-32,-8l-42,13l-12,38l-16,-7z\
       m31,20l39,-17l51,9l42,44l10,55l-41,-58l-35,-28l-40,-7l-26,2z",
       200);
			     v4pPolygonAddJump(pCol);
  v4pPolygonTransform(pCol, -200, -300, 0, 0); // x-=400
  
  // v4pPolygonDecodePoints(pCol, "4e05 5a2b 822b 6242 6e69 4e51 2d69 3a42 192b 412b", 700);
  // Path extracted from http://upload.wikimedia.org/wikipedia/commons/4/4e/3_stars.svg
  // path = "M 478.1117,4.99999  L 490.52087,43.198877 L 530.68482,43.196598 L 498.19016,66.802524
  //         L 510.60367,105.00001 L 478.1117,81.390382 L 445.61972,105.00001 L 458.03324,66.80253
  //         L 425.53858,43.196598 L 465.70253,43.198877 L 478.1117,4.99999 z"
  // path/x-400 (by hand) = "M 78,5 L 90,43 L 130,43 L 98,66 L 110,105 L 78,81 L 45,105 L 58,66 L25,43 L 65,43 L 78,5
  // printf "%02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x" \
  //                       78 5  90 43  130 43  98 66  110 105  78 81  45 105  58 66  25 43  65 43  78 5
  //         => 4e05 5a2b 822b 6242 6e69 4e51 2d69 3a42 192b 412b 4e05
  // The last point is useless in v4p
  v4pPolygonTransform(pCol, -83, - 7, 0, 0);
  for (j= 0; j < STRESS_AMOUNT; j++) {
    for (k = 0; k < STRESS_AMOUNT; k++) {
      pColMatrix[j][k] = v4pPolygonClone(pCol);
      v4pPolygonIntoList(pColMatrix[j][k], &scene);
      v4pPolygonTransformClone(pCol, pColMatrix[j][k], v4pDisplayWidth * (k - STRESS_AMOUNT / 2), v4pDisplayWidth * (j - STRESS_AMOUNT / 2), 10, 0);
    }
  }
  return success;
}

Boolean gmOnIterate() {
	int i = iu, j, k;
	if (diu>0 && i >100 * STRESS_AMOUNT) diu=-diu;
	if (diu<0 && i + diu < -100) {
	  diu=-diu;
	  liu--;
	}
    v4pSetView(-v4pDisplayWidth * i / 256, -v4pDisplayHeight * i / 256, v4pDisplayWidth + v4pDisplayWidth * i / 256, v4pDisplayHeight + v4pDisplayHeight * i / 256);
    
    if (liu & 1)
      for (j= 0; j < STRESS_AMOUNT; j++) {
        for (k = 0; k < STRESS_AMOUNT; k++) {
          v4pPolygonTransformClone(pCol, pColMatrix[j][k], v4pDisplayWidth * (1 + k - STRESS_AMOUNT/2) / 2, v4pDisplayWidth * (1 + j - STRESS_AMOUNT/2)/2, (j * k) + iu2 / 4, 0);
        }
      }
  iu+=diu;
  iu2+=diu/2;
  return (liu < 0);
}

Boolean gmOnFrame() {
   v4pRender();
   return success;
}

void gmOnQuit() {
  v4pDisplayQuit();
}

int main(int argc, char** argv) {
  return gmMain(argc, argv);
}

