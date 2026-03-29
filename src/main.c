/*
 * TODO:
 * - extend x11_create_window
 * - introduce memory arenas and extract the x11 initalization
 * - read display from env
 * - try connecting to a port if file not exists
 * - read available x11 extensions
 * - draw custom strings
 * - improve the ui
 */
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/un.h>

#include "closer.h"
#include "x11_client.h"

#include "syscalls.c"

#define PrintCstr(cstr) write(1, cstr, sizeof(cstr)-1)


#if CLOSER_DEBUG
#  define STB_SPRINTF_NOFLOAT
#  define STB_SPRINTF_IMPLEMENTATION
#  include "stb_sprintf.h"

char *__debug_printf_callback(const char *buf, void *user, int len)
{
  Unused(user);
  write(1, buf, len);
  return (char*)buf;
}

void debug_printf(const char* fmt, ...)
{
  va_list va;
  char buffer[STB_SPRINTF_MIN];
  va_start(va, fmt);
  stbsp_vsprintfcb(__debug_printf_callback, NULL, buffer, fmt, va);
  va_end(va);
}
#define DebugPrintf(fmt, ...) debug_printf(fmt, __VA_ARGS__)
#else
#define DebugPrintf(fmt, ...)
#endif

const char* x11_error_strs[] = {
  #define X(e, s) s,
  ERRORS
  #undef X
};

void debug_error_print(X11GenericError err)
{
  #ifndef CLOSER_DEBUG
  Unused(err);
  #endif
  DebugPrintf("error(%u): %s!\n", err.code, x11_error_strs[err.code]);
}

global X11Screen g_screen;
global int g_connfd;
global u32 g_idBase;
global u32 g_idCtr;
global u32 g_idMask;
global char** g_envp;


X11GC x11_create_gc_basic(X11Window window)
{
  X11CreateGCReq req = {
    .opcode = 55,
    .reqLen = 4, // no extra values
    .cid = g_idCtr,
    .drawable = window,
  };
  g_idCtr++;
  send(g_connfd, (void*)&req, sizeof(req), 0);
  return req.cid;
}

void x11_poly_text8(X11Window window, X11GC gc, i16 x, i16 y, char* str, u8 strLen)
{
  u16 textSize = sizeof(X11TextItem8)+strLen;
  u16 padLen = X11Pad(textSize);
  X11PolyText8Req req = {
    .opcode = 74,
    .reqLen = 4 + (textSize + padLen) / 4,
    .drawable = window,
    .gc = gc,
    .x = x,
    .y = y
  };
  X11TextItem8 text = {.strLen = strLen};
  u8 padbytes[4] = {0};
  send(g_connfd, (void*)&req, sizeof(req), 0);
  send(g_connfd, (void*)&text, sizeof(text), 0);
  send(g_connfd, str, (usize)strLen, 0);
  if (padLen > 0) send(g_connfd, padbytes, padLen, 0);
}

X11Window x11_create_window(
  X11Window parent,
  i16 x, i16 y,
  u16 width, u16 height
)
{
  X11Window result = 0;
  X11CreateWindowReq req = {
    .opcode = 1,
    .depth = 0,
    .requestLength = 8 + X11_CW_VALUES_COUNT, // @Hardcode
    .windowId = g_idCtr,
    .parent = parent,
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .borderWidth = 1,
    .wndclass = COPY_FROM_PARENT,
    .visual = COPY_FROM_PARENT,
    .bitmask = X11_CW_BACKGROUND_PIXEL | X11_CW_BORDER_PIXEL | X11_CW_EVENT_MASK,
    .values = { // @Hardcode
      g_screen.whitePixel,
      g_screen.blackPixel,
      X11_EV_KEY_PRESS_MASK | X11_EV_EXPOSE_MASK | X11_EV_BUTTON_PRESS_MASK
    }
  };
  send(g_connfd, (void*)&req, sizeof(req), 0);
  result = g_idCtr;
  g_idCtr += 1;
  return result;
}

void x11_change_property(
  X11Window window,
  X11Atom property,
  X11Atom type,
  u8 format,
  u8 mode,
  u8* data,
  usize elementCount
)
{
  u32 elementSizeInBytes = format/8;
  u32 size = elementSizeInBytes * elementCount;
  u32 pad = X11Pad(size);
  X11ChangePropertyReq req = {
    .opcode = 18,
    .mode = mode,
    .reqLen = 6 + (size + pad)/4,
    .window = window,
    .property = property,
    .type = type,
    .format = format,
    .formatLen = elementCount,
  };
  send(g_connfd, (void*)&req, sizeof(req), 0);
  send(g_connfd, data, size, 0);
  char paddingbytes[4] = {0};
  if (pad > 0)
  {
    send(g_connfd, paddingbytes, pad, 0);
  }
}

void x11_window_set_name(X11Window window, char* name, u16 nameLen)
{
  x11_change_property(
    window,
    X11A_WM_NAME,
    X11A_STRING,
    8,
    X11_CHGP_REPLACE,
    (u8*)name,
    nameLen
  );
}

void x11_map_window(X11Window window)
{
  X11MapWindowReq req = {
    .opcode = 8,
    .requestLen = sizeof(X11MapWindowReq)/4,
    .window = window
  };
  send(g_connfd, (void*)&req, sizeof(X11MapWindowReq), 0);
}

void x11_map_subwindows(X11Window window)
{
  X11MapSubwindowsReq req = {
    .opcode = 8,
    .reqLen = 2,
    .window = window,
  };
  send(g_connfd, (void*)&req, sizeof(req), 0);
}

void x11_clear_area(X11Window window, i16 x, i16 y, u16 width, u16 height, bool8 exposures)
{
  X11ClearAreaReq req = {
    .opcode = 61,
    .exposures = exposures,
    .reqLen = 4,
    .window = window,
    .x = x,
    .y = y,
    .width = width,
    .height = height,
  };
  send(g_connfd, (void*)&req, sizeof(req), 0);
}

void x11_clear_window(X11Window window, bool8 exposures)
{
  x11_clear_area(window, 0, 0, 0, 0, exposures);
}

void x11_change_window_attributes(X11Window window, u32 bitmask, X11WindowAttributes* attribs)
{
  u8 memory[sizeof(X11ChangeWindowAttributesReq) + sizeof(X11WindowAttributes)] = {0};
  usize len = sizeof(X11ChangeWindowAttributesReq);
  X11ChangeWindowAttributesReq* req = (void*)memory;
  u32* values = (u32*)(req + 1);
  i32 attributesCount = sizeof(X11WindowAttributes)/sizeof(u32);
  i32 count = 0;
  for (i32 i = 0;i < attributesCount;++i)
  {
    if (bitmask & (1 << i))
    {
      len += 4;
      values[count] = (u32)(((u32*)attribs)[i]);
      count += 1;
    }
  }
  req->opcode = 2;
  req->reqLen = len/4;
  req->window = window;
  req->bitmask = bitmask;
  send(g_connfd, (void*)memory, len, 0);
}

internal X11Atom x11_intern_atom(const char* name, bool8 onlyIfExists)
{
  usize nameLen = strlen(name);
  usize padLen = X11Pad(nameLen);
  X11InternAtomReq req = {
    .opcode = 16,
    .onlyIfExists = onlyIfExists,
    .reqLen = 2 + (nameLen + padLen)/4,
    .nameLen = nameLen,
  };
  char padbytes[4] = {0};
  send(g_connfd, (void*)&req, sizeof(req), 0);
  send(g_connfd, name, nameLen, 0);
  if (padLen > 0) send(g_connfd, padbytes, padLen, 0);
  
  X11InternAtomReply reply = {0};
  recv(g_connfd, (void*)&reply, sizeof(reply), 0);
  return reply.atom;
}

internal void x11_list_extensions(void)
{
  // if i wanted to make this a usable fuction
  // it would return a list of strings allocated in an arena or
  // something
  // but i just wanna print them
  X11ListExtensionsReq req = {
    .code = 99,
    .reqLen = 1
  };
  X11ListExtensionsReply reply = {0};
  send(g_connfd, (void*)&req, sizeof(req), 0);
  recv(g_connfd, (void*)&reply, sizeof(reply), 0);
  PrintCstr("Available extensions:\n");
  usize n = 0;
  for (card8 i = 0;i < reply.strsCount;++i)
  {
    u8 len = 0;
    char bytes[256] = {0};
    recv(g_connfd, (void*)&len, 1, 0);
    recv(g_connfd, bytes, (usize)len, 0);
    write(1, bytes, (usize)len);
    write(1, "\n", 1);
    n += 1+(usize)len;
  }
  usize pad = X11Pad(n);
  u8 padbytes[4] = {0};
  if (pad > 0) recv(g_connfd, padbytes, pad, 0);
}


void* alloc(usize size)
{
  void* mem = mmap(
    NULL,
    size+(sizeof(usize)),
    PROT_WRITE | PROT_READ,
    MAP_PRIVATE | MAP_ANONYMOUS,
    -1,
    0
  );
  if (mem == MAP_FAILED) mem = NULL;
  usize* memusize = (usize*)mem;
  memusize[0] = size;
  return memusize+1;
}

int dealloc(void* ptr)
{
  usize* memusize = (usize*)ptr;
  memusize -= 1;
  usize memSize = memusize[0];
  return munmap((void*)memusize, memSize);
}

#define WINDOW_WIDTH 300
#define WINDOW_HEIGHT 100

void trigger_command(char* const command)
{
  pid_t p = fork();
  char* const argv[] = {
    "/usr/bin/systemctl",
    command,
    NULL
  };
  if (p == 0)
  {
    int ret = execve("/usr/bin/systemctl", argv, g_envp);
    if (ret == -1) 
    {
      PrintCstr("execve() failed\n");
      exit(1);
    }
  }
}

void action_poweroff(void)
{
#ifdef CLOSER_DISABLE
  PrintCstr("trigger poweroff here\n");
#else
  trigger_command("poweroff");
#endif
}

void action_reboot(void)
{
#ifdef CLOSER_DISABLE
  PrintCstr("trigger reboot here\n");
#else
  trigger_command("reboot");
#endif
}

void action_suspend(void)
{
#ifdef CLOSER_DISABLE
  PrintCstr("trigger suspend here\n");
#else
  trigger_command("suspend");
#endif
}

void action_cancel(void)
{
  PrintCstr("cancelled\n");
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

void buttons_init(X11Window parentWindow, X11Window buttons[__BUTTON_COUNT])
{
  for (int i = 0;i < __BUTTON_COUNT;++i)
  {
    buttons[i] = x11_create_window(
       parentWindow,
       i * BUTTON_WIDTH,
       BUTTON_HEIGHT,
       BUTTON_WIDTH,
       BUTTON_HEIGHT
    );
  }
}

typedef void (*ButtonFuncType)(void);

const char* buttonTexts[] = {"Poweroff", "Reboot", "Suspend", "Cancel"};

const ButtonFuncType buttonActions[] = {action_poweroff, action_reboot, action_suspend, action_cancel};

bool32 x11_init_connection(X11State* res)
{
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);


  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  static const char x11path[] = "/tmp/.X11-unix/X0"; // TODO: get display from env
  memcpy(addr.sun_path, x11path, sizeof(x11path));

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
    PrintCstr("Could not open x11 server\n");
    goto fail;
  }


  X11Connection connCookie = {
    .byteOrder = X11_LITTLE_ENDIAN,
    .protocolMajorVersion = 11,
    .protocolMinorVersion = 0,
    .authProtocolDataLength = 0,
    .authProtocolNameLength = 0,
  };

  send(fd, &connCookie, sizeof(connCookie), 0);
  u8 x11returnCode = 0;
  recv(fd, &x11returnCode, 1, 0);
  if (x11returnCode == X11_RESPONSE_SUCCESS)
  {
    PrintCstr("Established x11 server connection!\n");
  }
  else
  {
    PrintCstr("x11 server refused connection!\n");
    goto fail;
  }


  recv(fd, &res->response, sizeof(res->response), 0);

  DebugPrintf("id base: %x, mask: %x\n", res->response.resourceIdBase, res->response.resourceIdMask);
  g_idBase = res->response.resourceIdBase;
  g_idCtr = g_idBase;
  g_idMask = res->response.resourceIdMask;

  DebugPrintf("screens count: %d\n", res->response.screenCount);
  DebugPrintf("formats count: %u\n", res->response.pixmapFormatsCount);
  DebugPrintf("vendor length: %u\n", res->response.vendorLength);
  DebugPrintf("extra info len: %d\n", 4 * (int)res->response.extraDataLengthIn4Bytes);


  // align to nearest multiple of 4
  usize vendorLenAligned = (res->response.vendorLength+3) & ~0x03;

  res->screens.len = res->response.screenCount;
  res->formats.len = res->response.pixmapFormatsCount;
  res->vendorInfo.len = res->response.vendorLength;


  // X11 connection initialization
  //usize screensSize = res->response.screenCount * sizeof(X11Screen);
  usize formatsSize = res->response.pixmapFormatsCount * sizeof(X11Format);
  usize extrasSize  = (usize)res->response.extraDataLengthIn4Bytes * 4;

  usize memLen = (usize)(
    //(isize)screensSize +
    //(isize)formatsSize +
    //(isize)vendorLenAligned +
    (isize)extrasSize
  );
  DebugPrintf("bytes to alloc = %zu\n", memLen);
  void* mem = alloc(memLen);
  if (mem == NULL)
  {
    PrintCstr("Failed to map memory\n");
    goto fail;
  }
  char* vendor = mem;
  X11Format* formats = (void*)((char*)mem + vendorLenAligned);
  X11Screen* screens = (void*)(formats + res->response.pixmapFormatsCount);

  // NOTE: extraDataLength specifies ALL the memory necessery to
  // init x11
  // you could just recv everything at once
  // recv(fd, mem, (usize)response.extraDataLengthIn4Bytes * 4ULL, 0);
  DebugPrintf("receiving extars\n", 0);

  if (vendorLenAligned > 0)
  {
    recv(fd, vendor, vendorLenAligned, 0);
  }
  DebugPrintf("vendor: %.*s\n", vendorLenAligned, vendor);
  res->vendorInfo.data = vendor;

  // receive format infos
  if (res->response.pixmapFormatsCount > 0)
  {
    recv(fd, formats, formatsSize, 0);
  }
  res->formats.data = formats;

  DebugPrintf("Formats(%u):\n", res->response.pixmapFormatsCount);
  for (usize i = 0;i < res->response.pixmapFormatsCount;++i)
  {
    DebugPrintf("%zu: depth = %d, bitsPerLine = %d, scanLinePad = %d\n", i+1, formats[i].depth, formats[i].bitsPerLine, formats[i].scanlinePad);
  }


  // screens
  for (usize i = 0;i < res->response.screenCount;++i)
  {
    X11Screen screen = {0};
    recv(fd, (void*)&screen, sizeof(screen), 0);
    screens[i] = screen;
    DebugPrintf("Screens(%d):\n", res->response.screenCount);
    for (i32 i = 0;i < res->response.screenCount;++i)
    {
      DebugPrintf("%u: depthsCount = %d, width = %u, height = %u\n", i+1, screens[i].allowedDepthsCount, screens[i].widthInPixels, screens[i].heightInPixels);

      // reveice depths info
      X11Depth depth = {0};
      for (i8 j = 0;j < screens[i].allowedDepthsCount;++j)
      {
        recv(fd, &depth, sizeof(X11Depth), 0);
        //DebugPrintf("--depth%d: visual = %u\n", j+1, depth.numberOfVisuals);
        usize visualTypesSize = sizeof(X11VisualType) * (usize)depth.numberOfVisuals;

        if (visualTypesSize > 0)
        {
          X11VisualType* visualTypes = alloc(visualTypesSize);
          recv(fd, visualTypes, visualTypesSize, 0);
          //for (u16 k = 0;k < depth.numberOfVisuals;++k)
          //{
          //  X11VisualType* v = &visualTypes[k];
          //  //DebugPrintf("----visual%u: bits = %d, red = %x, green = %x, blue = %x\n", k, v->bitsPerRgbValue, v->redMask, v->greenMask, v->blueMask);
          //}
          dealloc(visualTypes);
        }
      }
    }
  }
  res->screens.data = screens;
  g_screen = screens[0];


  res->connfd = g_connfd = fd;

  return TRUE;
fail:
  close(fd);
  return FALSE;
}

int main(int argc, char* argv[], char* env[])
{
  Unused(argc);
  Unused(argv);
  g_envp = env;

  X11State state = {0};
  bool32 res = x11_init_connection(&state);
  if (res != TRUE) return 1;

  //x11_list_extensions();

// APP INITIALIZATION
  i16 middleX = state.screens.data[0].widthInPixels/2;
  i16 middleY = state.screens.data[0].heightInPixels/2;
  i16 windowWidth = WINDOW_WIDTH;
  i16 windowHeight = WINDOW_HEIGHT;
  i16 windowX = middleX - (windowWidth/2);
  i16 windowY = middleY - (windowHeight/2);
  X11Window window = x11_create_window(g_screen.rootWindow, windowX, windowY, windowWidth, windowHeight);

  char mainWindowName[] = "Closer";
  x11_window_set_name(window, mainWindowName, sizeof(mainWindowName)-1);

  X11Atom windowManager = x11_intern_atom("_NET_WM_WINDOW_TYPE", 0);
  X11Atom util = x11_intern_atom("_NET_WM_WINDOW_TYPE_UTILITY", 0);

  x11_change_property(
    window,
    windowManager,
    X11A_ATOM,
    32,
    X11_CHGP_REPLACE,
    (unsigned char*)&util,
    1
  );

  X11Atom wmProtocols = x11_intern_atom("WM_PROTOCOLS", FALSE);
  wmProtocols = x11_intern_atom("WM_PROTOCOLS", FALSE);
  X11Atom deleteWindow = x11_intern_atom("WM_DELETE_WINDOW", FALSE);
  x11_change_property(
    window,
    wmProtocols,
    X11A_ATOM,
    32,
    X11_CHGP_REPLACE,
    (u8*)&deleteWindow,
    1
  );

  X11Window buttonWnds[__BUTTON_COUNT] = {0};
  X11GC buttonGCs[__BUTTON_COUNT]= {0};

  buttons_init(window, buttonWnds);

  x11_map_window(window);

  for (int i = 0;i < __BUTTON_COUNT;++i)
  {
    x11_map_window(buttonWnds[i]);
  }

  X11GC mainGC = x11_create_gc_basic(window);

  for (int i = 0;i < __BUTTON_COUNT;++i)
  {
    buttonGCs[i] = x11_create_gc_basic(buttonWnds[i]);
  }

  u32 valuemask = X11_CW_BACKGROUND_PIXEL;// | X11_CW_BORDER_PIXEL | X11_CW_EVENT_MASK;

  X11WindowAttributes defAttribs = {
    .backgroundPixel = g_screen.whitePixel,
  };

  X11WindowAttributes selectedAttribs = {
    .backgroundPixel = 0xFFee8080,
  };
  
  bool32 running = 1;
  u32 selectedIdx = 0;
  static char mainMsg[] = "Poweroff the device?";
  x11_change_window_attributes(buttonWnds[selectedIdx], valuemask, &selectedAttribs);


  X11GenericMessage msg = {0};
  while (running)
  {
    recv(g_connfd, (void*)&msg, sizeof(X11GenericMessage), 0);
    switch (msg.code)
    {
      case MSG_ERROR:
        {
          X11GenericError* err = (X11GenericError*)&msg;
          debug_error_print(*err);
          break;
        }
      case MSG_EXPOSE:
        {
          X11EventExpose* ev = (void*)&msg;
          DebugPrintf("Expose event, window = %u\n", ev->window);
          if (ev->window == window)
          {
            x11_poly_text8(window, mainGC, WINDOW_WIDTH/4, WINDOW_HEIGHT/2-30, mainMsg, sizeof(mainMsg)-1);
          }
          for (int i = 0;i < __BUTTON_COUNT;++i)
          {
            if (ev->window == buttonWnds[i])
            {
              x11_poly_text8(buttonWnds[i], buttonGCs[i], 10, WINDOW_HEIGHT/2/2, (char*)buttonTexts[i], strlen(buttonTexts[i]));
            }
          }
          break;
        }
      case MSG_KEYPRESS:
        {
          X11EventInput* kv = (X11EventInput*)&msg;
          if (kv->detail == KEY_PRESS_Q || kv->detail == KEY_PRESS_ESC) running = 0;
          if (kv->detail == KEY_PRESS_RIGHT || kv->detail == KEY_PRESS_LEFT)
          {
            x11_change_window_attributes(buttonWnds[selectedIdx], valuemask, &defAttribs);
            x11_clear_window(buttonWnds[selectedIdx], FALSE);
            x11_poly_text8(buttonWnds[selectedIdx], buttonGCs[selectedIdx], 10, WINDOW_HEIGHT/2/2, (char*)buttonTexts[selectedIdx], strlen(buttonTexts[selectedIdx]));

            selectedIdx = (selectedIdx + (kv->detail == KEY_PRESS_RIGHT ? 1 : -1)) % __BUTTON_COUNT;

            x11_change_window_attributes(buttonWnds[selectedIdx], valuemask, &selectedAttribs);
            x11_clear_window(buttonWnds[selectedIdx], FALSE);
            x11_poly_text8(buttonWnds[selectedIdx], buttonGCs[selectedIdx], 10, WINDOW_HEIGHT/2/2, (char*)buttonTexts[selectedIdx], strlen(buttonTexts[selectedIdx]));
            DebugPrintf("selectedIdx = %u\n", selectedIdx);
          }
          if (kv->detail == KEY_PRESS_RETURN)
          {
            buttonActions[selectedIdx]();
            running = FALSE;
          }
          DebugPrintf("Keypress: %u\n", kv->detail);
          break;
        }
      case MSG_BUTTONPRESS:
        {
          X11EventInput* kb = (X11EventInput*)&msg;
          for (int i = 0;i < __BUTTON_COUNT;++i)
          {
            if (kb->event == buttonWnds[i])
            {
              buttonActions[i]();
              running = FALSE;
            }
          }
          DebugPrintf("Buttonpress: %u\n", kb->detail);
          break;
        }
      case (MSG_CLIENT_MESSAGE + 128): // most significant bit set means message sent through SendEvent
      case MSG_CLIENT_MESSAGE:
        {
          X11EventClientMessage* cl = (X11EventClientMessage*)&msg;
          DebugPrintf("Client message\n", 0);
          if (cl->data.l[0] == deleteWindow) running = 0;
          break;
        }
      default:
        {
          DebugPrintf("Other msg %u\n", msg.code);
          break;
        }
    }
  }
  return 0;
}
