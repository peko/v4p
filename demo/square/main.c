#include "gamemachine.h"
#include "v4p.h"
#include "v4pi.h"

#define STRESS_AMOUNT 128
PolygonP  scene;
Boolean gmOnInit() {
  int i = 0, j, k, loop;
  v4pDisplayInit(1, 0);
  v4pInit();
  scene = NULL;
  v4pSetScene(&scene);
  v4pSetBGColor(blue);
 
  PolygonP pColMatrix[STRESS_AMOUNT][STRESS_AMOUNT];
  PolygonP pCol=v4pPolygonNew(absolute, red, 10);
  v4pPolygonRect(pCol, -v4pDisplayWidth / 3 + v4pDisplayWidth, -v4pDisplayHeight / 3, v4pDisplayWidth / 3, v4pDisplayHeight / 3);

  for (j= 0; j < STRESS_AMOUNT; j++) {
    for (k = 0; k < STRESS_AMOUNT; k++) {
      pColMatrix[j][k] = v4pPolygonClone(pCol);
      v4pPolygonIntoList(pColMatrix[j][k], &scene);
      //v4pPolygonTransformClone(pCol, pColMatrix[j][k], v4pDisplayWidth * (k - STRESS_AMOUNT / 2), v4pDisplayWidth * (j - STRESS_AMOUNT / 2), 10, 0);
    }
  }

  for (i = 0 ; i < (1 + STRESS_AMOUNT / 2) * v4pDisplayWidth ; i+= 2 * STRESS_AMOUNT) {
    v4pSetView(-v4pDisplayWidth * (256 + i) / 256,-v4pDisplayHeight * (256 + i) / 256, v4pDisplayWidth * (256 + i) / 256, v4pDisplayHeight * (256 + i) / 256);
    for (j= 0; j < STRESS_AMOUNT; j++) {
      for (k = 0; k < STRESS_AMOUNT; k++) {
        v4pPolygonTransformClone(pCol, pColMatrix[j][k], v4pDisplayWidth * (k - STRESS_AMOUNT / 2), v4pDisplayWidth * (j - STRESS_AMOUNT / 2), (j * k) + i / 16, 0);
      }
    }
    v4pRender();
  }

  for (; i > -50 ; i-= 2 * STRESS_AMOUNT) {
    v4pSetView(-v4pDisplayWidth * (256 + i) / 256,-v4pDisplayHeight * (256 + i) / 256, v4pDisplayWidth * (256 + i) / 256, v4pDisplayHeight * (256 + i) / 256);
    for (j= 0; j < STRESS_AMOUNT; j++) {
      for (k = 0; k < STRESS_AMOUNT; k++) {
        v4pPolygonTransformClone(pCol, pColMatrix[j][k], v4pDisplayWidth * (k - STRESS_AMOUNT / 2), v4pDisplayWidth * (j - STRESS_AMOUNT / 2), (j * k) + i / 16, 0);
      }
    }
    v4pRender();
  }

  for (i; i < (1 + STRESS_AMOUNT / 2) * v4pDisplayWidth ; i+= 2 * STRESS_AMOUNT) {
    v4pSetView(-v4pDisplayWidth * (256 + i) / 256,-v4pDisplayHeight * (256 + i) / 256, v4pDisplayWidth * (256 + i) / 256, v4pDisplayHeight * (256 + i) / 256);
    v4pRender();
  }
}

Boolean gmOnIterate() {
  return 0;
}

int main(int argc, char** argv) {
     return gmMain(argc, argv);
}

