#include "gamemachine.h"
#include "v4p.h"
#include "v4pi.h"

PolygonP  scene;
Boolean gmOnInit() {
  v4pDisplayInit(1, 1);
  v4pInit();
  scene = NULL;
  v4pSetScene(&scene);
  PolygonP pCol=v4pListAddPolygon(&scene, absolute, green, 10);
  v4pPolygonRect(pCol, 20, 20, v4pDisplayWidth - 20, v4pDisplayHeight - 20);
  v4pRender();
  sleep(2);
}
Boolean gmOnIterate() {
  return failure;
}

int main(int argc, char** argv) {
     return gmMain(argc, argv);
}

