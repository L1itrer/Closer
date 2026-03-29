#ifndef X11_CLIENT_H
#define X11_CLIENT_H
#include "closer.h"
#include <stdint.h>
#include <stddef.h>


typedef size_t usize;
typedef ptrdiff_t isize;
typedef isize ssize_t;


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef u8 char8;
typedef u16 char16;

typedef i8 bool8;
typedef i32 bool32;

typedef uint32_t card32;
typedef uint16_t card16;
typedef uint8_t  card8;


typedef uint32_t X11Window;
typedef uint32_t X11GC;
typedef uint32_t X11Colormap;
typedef uint32_t X11Visual;
typedef uint32_t X11Drawable;
typedef uint32_t X11Atom;
typedef uint32_t X11Pixmap;
typedef uint32_t X11Cursor;


// ERRORS
#define ERRORS \
X(Success, "No Error") \
X(BadRequest, "Bad Request Code") \
X(BadValue, "Bad Value") \
X(BadWindow, "Bad Window") \
X(BadPixmap, "BadPixmap") \
X(BadAtom, "BadAtom") \
X(BadCursor, "BadCursor") \
X(BadFont, "BadFont") \
X(BadMatch, "BadMatch") \
X(BadDrawable, "BadDrawable") \
X(BadAccess, "BadAccess") \
X(BadAlloc, "BadAlloc") \
X(BadColor, "BadColor") \
X(BadGC, "BadGC") \
X(BadIDChoice, "BadIDChoice") \
X(BadName, "BadName") \
X(BadLength, "BadLength") \
X(BadImplementation, "BadImplementation")

#define X11_LITTLE_ENDIAN 'l'

typedef enum X11ErrorCodes{
  #define X(e, s) X11##e,
  ERRORS
  #undef X
}X11ErrorCodes;


typedef struct X11Request{
  u8 majorOpcode;
  u16 length;
  u8 data;
} __attribute__((packed)) X11Request;

// Events and errors both have 32 bytes
// you can read 32 bytes 
// check the code and pointer-cast to the right structure
typedef struct X11GenericMessage {
  u8 code;
  u8 otherBytes[31];
} X11GenericMessage;

typedef struct X11EventExpose {
  u8 code;
  u8 unused;
  card16 sequenceNumber;
  X11Window window;
  card16 x, y, width, height, count;
  u8 unused2[14];
} __attribute__((packed)) X11EventExpose;

// the structure for key press and button press events
// they both share the exact same layout
// only keycodes are different
typedef struct X11EventInput{
  u8 code;
  card8 detail;
  card16 sequenceNumber;
  card32 timestamp;
  X11Window root;
  X11Window event;
  X11Window child;
  i16 rootX;
  i16 rootY;
  i16 eventX;
  i16 eventY;
  u16 state;
  bool8 sameScreen;
  u8 unused;
} __attribute__((packed)) X11EventInput;

typedef struct X11EventClientMessage {
  u8 code;
  card8 format;
  card16 sequenceNumber;
  X11Window window;
  X11Atom type;
	union {
		u8 b[20];
		u16 s[10];
		u32 l[5];
		} data;
} __attribute__((packed)) X11EventClientMessage;


#define KEY_PRESS_Q 24
#define KEY_PRESS_ESC 9
#define KEY_PRESS_RETURN 36

#define KEY_PRESS_RIGHT 114
#define KEY_PRESS_LEFT 113

#define BUTTON_PRESS_LEFT 1
#define BUTTON_PRESS_RIGHT 3

typedef struct X11GenericError {
  u8 error;
  u8 code;
  u8 otherBytes[30];
} __attribute__((packed)) X11GenericError;


#define MSG_ERROR 0
#define MSG_KEYPRESS 2
#define MSG_BUTTONPRESS 4
#define MSG_EXPOSE 12
#define MSG_CLIENT_MESSAGE 33


#define X11Pad(E) ((4 - (E % 4)) % 4)


typedef struct X11Connection{
  u8 byteOrder;
  u8 unused; // special thanks to the writers of the spec for inconsistent info between the appendix and inital connection chapter
  card16 protocolMajorVersion;
  card16 protocolMinorVersion;
  u16 authProtocolNameLength;
  u16 authProtocolDataLength;
  u16 unused2;
  //char data[]; // do not use! not gonna support auth anyway
} __attribute__((packed)) X11Connection;

typedef struct X11SetupSuccessResponse {
  u8 code; // NOTE: only not in the struct because i read it first
  u8  unused;
  card16 protocolMajorVersion;
  card16 protocolMinorVersion;
  u16 extraDataLengthIn4Bytes;
  card32 releaseNumber;
  card32 resourceIdBase;
  card32 resourceIdMask;
  card32 motionBufferSize;
  u16 vendorLength;
  card16 maximumRequestLength;
  card8  screenCount;
  u8  pixmapFormatsCount;
  u8  imageByteOrder;
  u8  bitmapBitOrder;
  card8  bitmapFormatScanlineUnit;
  card8  bitmapFormatScanlinePad;
  u8  minKeycode;
  u8  maxKeycode;
  u32 unused2;
  // char* vendor;
  // *formats;
  // *screens;
} __attribute__((packed)) X11SetupSuccessResponse;

typedef struct X11SetupAuthenticateResponse {
  u8 code;
  u8 unused[5];
  u16 extraInfoLen;
} __attribute__((packed)) X11SetupAuthenticateResponse;

typedef struct X11SetupFailedResponse {
  u8 code;
  u8 reasonLen;
  card16 protocolMajorVersion;
  card16 protocolMinorVersion;
  u16 responseLen;
} __attribute__((packed)) X11SetupFailedResponse;

typedef struct X11Format {
  card8 depth, bitsPerLine, scanlinePad;
  u8 unused[5];
} __attribute__((packed)) X11Format;

typedef struct X11Screen {
  X11Window rootWindow;
  X11Colormap colormap;
  card32 whitePixel;
  card32 blackPixel;
  u32 currentInputMask;
  card16 widthInPixels;
  card16 heightInPixels;
  card16 widthInMillimeters;
  card16 heightInMillimeters;
  card16 minInstalledMaps;
  card16 maxInstalledMaps;
  X11Visual visual;
  u8 backingStores;
  u8 saveUnders;
  i8 rootDepth;
  i8 allowedDepthsCount;
  // ListOfDepthsHere
} __attribute__((packed)) X11Screen;

typedef struct X11Depth {
  i8 depth;
  u8 unused;
  u16 numberOfVisuals;
  u32 unused2;
  // ListOfVisualsHere
} __attribute__((packed)) X11Depth;

typedef struct X11VisualType {
  u32 visualId;
  u8 visclass;
  i8 bitsPerRgbValue;
  i16 colormapEntries;
  i32 redMask;
  i32 greenMask;
  i32 blueMask;
  u32 unused;
} __attribute__((packed)) X11VisualType;

typedef struct X11Screens {
  X11Screen* data;
  usize len;
} X11Screens;


typedef struct X11Formats {
  X11Format* data;
  usize len;
} X11Formats;

typedef struct X11State {
  int connfd;
  X11SetupSuccessResponse response;
  X11Screens screens;
  X11Formats formats;
  StringView vendorInfo;
} X11State;

typedef struct X11WindowAttributes {
    X11Pixmap backgroundPixmap;	/* background or None or ParentRelative */
    u32 backgroundPixel;	/* background pixel */
    X11Pixmap borderPixmap;	/* border of the window */
    u32 borderPixel;	/* border pixel value */
    i32 bitGravity;		/* one of bit gravity values */
    i32 winGravity;		/* one of the window gravity values */
    i32 backingStore;		/* NotUseful, WhenMapped, Always */
    u32 backingPlanes;/* planes to be preserved if possible */
    u32 backingPixel;/* value to use in restoring planes */
    bool32 saveUnder;		/* should bits under be saved? (popups) */
    u32 eventMask;		/* set of events that should be saved */
    u32 doNotPropagateMask;	/* set of events that should not propagate */
    bool32 overrideRedirect;	/* boolean value for override-redirect */
    X11Colormap colormap;		/* color map to be associated with window */
    X11Cursor cursor;		/* cursor to be displayed (or None) */
} X11WindowAttributes;


#define COPY_FROM_PARENT 0

#define X11_CW_BACKGOUND_PIXMAP (1 << 0)
#define X11_CW_BACKGROUND_PIXEL (1 << 1)
#define X11_CW_BORDER_PIXEL 0x08
#define X11_CW_EVENT_MASK 0x800

#define X11_EV_KEY_PRESS_MASK (1L << 0)
#define X11_EV_BUTTON_PRESS_MASK (1L << 2)
#define X11_EV_EXPOSE_MASK (1L << 15)

/********** REQUESTS **********/

#define X11_RESPONSE_SUCCESS 1
#define X11_RESPONSE_AUTHENTICATE 2
#define X11_CW_VALUES_COUNT 3

typedef struct X11CreateWindowReq{
  u8 opcode;
  u8 depth;
  u16 requestLength; // 8 + n of values
  X11Window windowId;
  X11Window parent;
  i16 x, y;
  u16 width, height, borderWidth;
  u16 wndclass;
  X11Visual visual;
  u32 bitmask;
  // FIXME: @Hardcode
  // list of values should be arbitrary
  // but in this project i will always need 3
  u32 values[X11_CW_VALUES_COUNT]; // list of value
} __attribute__((packed)) X11CreateWindowReq;

typedef struct X11CreateGCReq {
  u8 opcode;
  u8 unused;
  u16 reqLen;
  X11GC cid;
  u32 drawable;
  u32 bitmask;
  // list of values follows
} __attribute__((packed)) X11CreateGCReq;


#define X11_CHGP_REPLACE 0
#define X11_CHGP_PREPEND 1
#define X11_CHGP_APPEND 2

typedef struct X11ChangePropertyReq{
  u8 opcode;
  u8 mode;
  u16 reqLen;
  X11Window window;
  X11Atom property;
  X11Atom type;
  u8 format;
  u8 unused[3];
  u32 formatLen;
  // bytes follow...
} __attribute__((packed)) X11ChangePropertyReq;


// taken from https://gitlab.freedesktop.org/xorg/lib/libx11/-/blob/master/src/StName.c
// @Verify
#define X11A_WM_NAME ((X11Atom)39)
#define X11A_STRING ((X11Atom)31)
#define X11A_ATOM ((X11Atom)4)


typedef struct X11MapWindowReq {
  u8 opcode, unused;
  u16 requestLen;
  X11Window window;
} __attribute__((packed)) X11MapWindowReq;


typedef struct X11MapSubwindowsReq {
  u8 opcode;
  u8 unused;
  u16 reqLen;
  X11Window window;
} __attribute__((packed)) X11MapSubwindowsReq;


typedef struct X11ClearAreaReq {
  u8 opcode;
  bool8 exposures;
  u16 reqLen;
  X11Window window;
  i16 x, y;
  card16 width, height;
} __attribute__((packed)) X11ClearAreaReq;

typedef struct X11ChangeWindowAttributesReq {
  u8 opcode;
  u8 unused;
  u16 reqLen;
  X11Window window;
  u32 bitmask;
} __attribute__((packed)) X11ChangeWindowAttributesReq;


typedef struct X11InternAtomReq {
  u8 opcode;
  bool8 onlyIfExists;
  u16 reqLen;
  u16 nameLen;
  u16 unused;
} __attribute__((packed)) X11InternAtomReq;

typedef struct X11InternAtomReply {
  u8 reply;
  u8 unused;
  card16 sequenceNumber;
  u32 replyLen; // always 0
  X11Atom atom; // can be null!
  u8 unused2[20];
} __attribute__((packed))  X11InternAtomReply;


typedef struct X11PolyText8Req {
  u8 opcode;
  u8 unused;
  u16 reqLen;
  u32 drawable;
  X11GC gc;
  i16 x, y;
  // text and padding
} __attribute__((packed)) X11PolyText8Req;

typedef struct X11TextItem8 {
  u8 strLen;
  i8 delta;
} X11TextItem8;

typedef struct X11ListExtensionsReq {
  u8 code, unused;
  u16 reqLen;
} __attribute__((packed)) X11ListExtensionsReq;

typedef struct X11ListExtensionsReply {
  u8 reply;
  card8 strsCount;
  card16 sequenceNumber;
  u32 replyLen;
  u8 unused[24];
} __attribute__((packed)) X11ListExtensionsReply;

#endif // X11_CLIENT_H
