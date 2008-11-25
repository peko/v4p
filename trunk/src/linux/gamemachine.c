#include <stdio.h>
#include "gamemachine.h"
#include "v4p_ll.h"
#include "v4pi.h"
#include "SDL/SDL.h"

GmState gmMachineState;

int gmMain(int argc, char* argv[])
{
    SDL_Surface *screen;
    SDL_Event event;
  
    Boolean exit=0;

    gmMachineState.buttons[0] = 0;

  
    gmOnInit();
  
    while(!exit) 
      { //main loop

      while(SDL_PollEvent(&event)) 
        { // polling

          switch (event.type) 
            { // event

            case SDL_QUIT:
              exit = 1;
              
              break;
            case SDL_KEYDOWN: {
              SDLKey keypressed = event.key.keysym.sym;
              if (keypressed == SDLK_ESCAPE) 
                exit = 1;
              
              break;
            }
            case SDL_MOUSEBUTTONDOWN:
              
              gmMachineState.buttons[0] = 1;
              break;

            case SDL_MOUSEBUTTONUP:
              
              gmMachineState.buttons[0] = 0;
              break;

            case SDL_MOUSEMOTION:
              gmMachineState.xpen = event.motion.x - marginX;
              gmMachineState.ypen = event.motion.y - marginY;
              break;
            
            }
        }
  
      exit |= gmOnIterate();
      
    }

    SDL_Quit();
  
    return 0;
}
