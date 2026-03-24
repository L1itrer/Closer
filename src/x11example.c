#include <X11/X.h>
#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#define WINDOW_X 100
#define WINDOW_Y 100
#define WINDOW_WIDTH 300
#define WINDOW_HEIGHT 100
#define BORDER_WIDTH 1

void action_poweroff(void)
{
  printf("trigger poweroff here\n");
}


void action_reboot(void)
{
  printf("trigger reboot here\n");
}


void action_suspend(void)
{
  printf("trigger suspend here\n");
}


void action_cancel(void)
{
  // yes this is supposed to be empty
}

typedef enum ButtonKind {
  BUTTON_POWEROFF,
  BUTTON_REBOOT,
  BUTTON_SUSPEND,
  BUTTON_CANCEL,
  __BUTTON_COUNT,
}ButtonKind;

#define BUTTON_WIDTH ((WINDOW_WIDTH)/(__BUTTON_COUNT))
#define BUTTON_HEIGHT ((WINDOW_HEIGHT/2))

typedef void (*ButtonFuncType)(void);

const char* buttonTexts[] = {"Poweroff", "Reboot", "Suspend", "Cancel"};

const ButtonFuncType buttonActions[] = {action_poweroff, action_reboot, action_suspend, action_cancel};


void buttons_init(Window parentWindow, Display* d, int s, Window buttons[__BUTTON_COUNT])
{
  XSetWindowAttributes buttonAttribs = {
    .event_mask = ExposureMask | KeyPressMask | ButtonPressMask,
    .border_pixel = BlackPixel(d, s),
    .background_pixel = WhitePixel(d, s),
  };
  for (int i = 0;i < __BUTTON_COUNT;++i)
  {
    buttons[i] = XCreateWindow(
       d,
       parentWindow,
       i * BUTTON_WIDTH,
       BUTTON_HEIGHT,
       BUTTON_WIDTH,
       BUTTON_HEIGHT,
       1,
       CopyFromParent,
       CopyFromParent,
       CopyFromParent,
       CWBackPixel | CWBorderPixel | CWEventMask,
      &buttonAttribs
    );
  }
}


const char msg[] = "Poweroff the computer?";

void draw_ith_string(Display* display, Window buttonWnds[], GC buttonGCS[], int i)
{
  int res = XDrawString(
      display,
      buttonWnds[i],
      buttonGCS[i],
      10,
      WINDOW_HEIGHT/2/2,
      buttonTexts[i],
      strlen(buttonTexts[i])
  );
}

int main(void)
{
  Display* display = XOpenDisplay(NULL);
  int screen = XDefaultScreen(display);
  Window root = XRootWindow(display, screen);

  XSetWindowAttributes wndAttribs= {0};
  wndAttribs.event_mask = ExposureMask | KeyPressMask;
  wndAttribs.border_pixel = BlackPixel(display, screen);
  wndAttribs.background_pixel = WhitePixel(display, screen);

  unsigned long valuemask = CWBackPixel | CWBorderPixel | CWEventMask;

  Window window = XCreateWindow(
    display,
    root,
    WINDOW_X,
    WINDOW_Y,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    BORDER_WIDTH,
    CopyFromParent,
    CopyFromParent,
    CopyFromParent,
    valuemask,
    &wndAttribs
  );

  XStoreName(display, window, "Closer");


  // atoms
  Atom windowManager = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  Atom util = XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);

  XChangeProperty(
    display,
    window,
    windowManager,
    XA_ATOM,
    32,
    PropModeReplace,
    (unsigned char*)&util,
    1
  );



  Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
  Atom deleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &deleteWindow, 1);

  Window buttonWnds[__BUTTON_COUNT] = {0};
  buttons_init(window, display, screen, buttonWnds);

  XMapWindow(display, window);

  for (int i = 0;i < __BUTTON_COUNT; ++i)
  {
    XMapWindow(display, buttonWnds[i]);
  }

  GC mainGC = XCreateGC(display, window, 0, NULL);
  GC buttonGCS[__BUTTON_COUNT] = {0};
  for (int i = 0;i < __BUTTON_COUNT;++i)
  {
    buttonGCS[i] = XCreateGC(display, buttonWnds[i], 0, NULL);
  }

  XEvent event = {0};

  int running = 1;
  unsigned int selectedIdx = 0;


  XSetWindowAttributes defAttribs = {
    .event_mask = ExposureMask | KeyPressMask | ButtonPressMask,
    .border_pixel = BlackPixel(display, screen),
    .background_pixel = WhitePixel(display, screen),
  };


  XSetWindowAttributes selectedAttribs = {
    .event_mask = ExposureMask | KeyPressMask | ButtonPressMask,
    .border_pixel = BlackPixel(display, screen),
    .background_pixel = 0xFFee8080 //WhitePixel(display, screen),
  };

  XChangeWindowAttributes(display, buttonWnds[selectedIdx], valuemask, &selectedAttribs);

  while (running)
  {
    XNextEvent(display, &event);
    if (event.xany.window == window)
    {
      switch (event.type)
      {
        case Expose:
        {
          XDrawString(
            display,
            window,
            mainGC,
            WINDOW_WIDTH/4,
            WINDOW_HEIGHT/2-30,
            msg,
            sizeof(msg)-1
          );
        }
        case KeyPress:
        {
          KeySym key = XLookupKeysym(&event.xkey, 0);
          if (key == XK_Escape)
          {
            running = 0;
          }
          if (key == XK_Return)
          {
            buttonActions[selectedIdx]();
            running = 0;
          }
          if (key == XK_Right || key == XK_Left)
          {
            XChangeWindowAttributes(display, buttonWnds[selectedIdx], valuemask, &defAttribs);
            XClearWindow(display, buttonWnds[selectedIdx]);
            draw_ith_string(display, buttonWnds, buttonGCS, selectedIdx);


            selectedIdx = (selectedIdx + (key == XK_Right ? 1 : -1)) % __BUTTON_COUNT;
            XChangeWindowAttributes(display, buttonWnds[selectedIdx], valuemask, &selectedAttribs);
            XClearWindow(display, buttonWnds[selectedIdx]);
            draw_ith_string(display, buttonWnds, buttonGCS, selectedIdx);
          }
          break;
        }
        case ClientMessage:
        {
          if (event.xclient.data.l[0] == deleteWindow)
          {
            running = 0;
          }
          break;
        }
      }
    }
    for (int i = 0;i < __BUTTON_COUNT;++i)
    {
      if (event.xany.window == buttonWnds[i])
      {
        switch (event.type)
        {
          case Expose:
          {
            draw_ith_string(display, buttonWnds, buttonGCS, i);
            break;
          }
        }
      }
    }
  }

  return 0;
}
