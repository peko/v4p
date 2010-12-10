#include <stdio.h>
#include "gamemachine.h"
#include "v4p_ll.h"
#include "v4pi.h"


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

Display *     display;
int           screen_num;
static char * appname;

GmState gmMachineState;

int gmMain(int argc, char* argv[])
{
    /*  Window variables  */
    Window       win;
    int          x, y;
    unsigned int width, height;
    unsigned int border_width;
    char *       window_name = "V4P!";
    char *       icon_name   = "V4P";

    /*  Display variables  */
    char *       display_name = NULL;
    unsigned int display_width, display_height;

    /* Miscellaneous X variables */
    XSizeHints *  size_hints;
    XWMHints   *  wm_hints;
    XClassHint *  class_hints;
    XTextProperty windowName, iconName;
    XEvent        report;
    XFontStruct * font_info;
    XGCValues     values;
    GC            gc;
    
    appname = argv[0];

    gmMachineState.buttons[0] = 0;

    /* Allocate memory for our structures */
    if ( !( size_hints  = XAllocSizeHints() ) || 
	 !( wm_hints    = XAllocWMHints()   ) ||
	 !( class_hints = XAllocClassHint() )    ) {
	fprintf(stderr, "%s: couldn't allocate memory.\n", appname);
	exit(EXIT_FAILURE);
    }


    /* Connect to X server  */
    if ( (display = XOpenDisplay(display_name)) == NULL ) {
	fprintf(stderr, "%s: couldn't connect to X server %s\n",
		appname, display_name);
	exit(EXIT_FAILURE);
    }


    /*  Get screen size from display structure macro  */

    screen_num     = DefaultScreen(display);
    display_width  = DisplayWidth(display, screen_num);
    display_height = DisplayHeight(display, screen_num);


    /*  Set initial window size and position, and create it  */

    x = y = 0;
    width  = display_width / 3;
    height = display_width / 3;

    win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
			      x, y, width, height, border_width,
			      BlackPixel(display, screen_num),
			      WhitePixel(display, screen_num));


    /*  Set hints for window manager before mapping window  */

    if ( XStringListToTextProperty(&window_name, 1, &windowName) == 0 ) {
	fprintf(stderr, "%s: structure allocation for windowName failed.\n",
		appname);
	exit(EXIT_FAILURE);
    }

    if ( XStringListToTextProperty(&icon_name, 1, &iconName) == 0 ) {
	fprintf(stderr, "%s: structure allocation for iconName failed.\n",
		appname);
	exit(EXIT_FAILURE);
    }

    size_hints->flags       = PPosition | PSize | PMinSize;
    size_hints->min_width   = 200;
    size_hints->min_height  = 100;

    wm_hints->flags         = StateHint | InputHint;
    wm_hints->initial_state = NormalState;
    wm_hints->input         = True;

    class_hints->res_name   = appname;
    class_hints->res_class  = "v4px";

    XSetWMProperties(display, win, &windowName, &iconName, argv, argc,
		     size_hints, wm_hints, class_hints);


    /*  Choose which events we want to handle  */
    XSelectInput(display, win, ExposureMask | KeyPressMask |
		 ButtonPressMask | ButtonReleaseMask | PointerMotion |
		 StructureNotifyMask);


 
    /*  Create graphics context  */
    gc = XCreateGC(display, win, 0, &values);

    XSetFont(display, gc, font_info->fid);
    XSetForeground(display, gc, BlackPixel(display, screen_num));

    /* Game Machine */
    gmOnInit();
  
    /*  Display Window  */
    XMapWindow(display, win);
  
   
    while (!exit) {
	XNextEvent(display, &report);

	switch ( report.type ) {

	case Expose:

	    if ( report.xexpose.count != 0 )
		break;

	    /*  Output   */
	    break;


	case ConfigureNotify:

	    /*  Store new window width & height  */
	    width  = report.xconfigure.width;
	    height = report.xconfigure.height;
	    break;


	case ButtonPress:
        gmMachineState.buttons[0] = 1;
        break;

    case ButtonRelease:
        gmMachineState.buttons[0] = 0;
        break;

    case PointerMotion:
        gmMachineState.xpen = event.motion.x;
        gmMachineState.ypen = event.motion.y;
        break;
            
	case KeyPress:
	    /*  Clean up and exit  */
	    XUnloadFont(display, font_info->fid);
	    XFreeGC(display, gc);
	    XCloseDisplay(display);
	    exit =1;

	}
   exit |= gmOnIterate();
    }

    return 0;
}

