#include <endian.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/un.h>


#define Unused(arg) (void)(arg)

#if CLOSER_DEBUG
#  define STB_SPRINTF_NOFLOAT
#  define STB_SPRINTF_IMPLEMENTATION
#  include "stb_sprintf.h"


ssize_t write(int fd, const void* buf, size_t count);

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


__attribute__((naked)) void* syscall1
(
  __attribute__((unused)) void* sys_code,
  __attribute__((unused)) void* arg0
)
{
  __asm__ __volatile__
  (
    "mov %%rdi, %%rax\n"
    "mov %%rsi, %%rdi\n"
    "syscall\n"
    "ret\n"
    :
    :
  );
}


__attribute__((naked)) void* syscall2
(
  __attribute__((unused)) void* sys_code,
  __attribute__((unused)) void* arg0,
  __attribute__((unused)) void* arg1
)
{
  __asm__ __volatile__
  (
    "mov %%rdi, %%rax\n"
    "mov %%rsi, %%rdi\n"
    "mov %%rdx, %%rsi\n"
    "syscall\n"
    "ret\n"
    :
    :
  );
}

__attribute__((naked)) void* syscall3
(
  __attribute__((unused)) void* sys_code,
  __attribute__((unused)) void* arg0,
  __attribute__((unused)) void* arg1,
  __attribute__((unused)) void* arg2
)
{
  __asm__ __volatile__
  (
    "mov %%rdi, %%rax\n"
    "mov %%rsi, %%rdi\n"
    "mov %%rdx, %%rsi\n"
    "mov %%rcx, %%rdx\n"
    "syscall\n"
    "ret\n"
    :
    :
  );
}


__attribute__((naked)) void* syscall5
(
  __attribute__((unused)) void* sys_code,
  __attribute__((unused)) void* arg0,
  __attribute__((unused)) void* arg1,
  __attribute__((unused)) void* arg2,
  __attribute__((unused)) void* arg3,
  __attribute__((unused)) void* arg4
)
{
  __asm__ __volatile__
  (
    "mov %%rdi, %%rax\n"
    "mov %%rsi, %%rdi\n"
    "mov %%rdx, %%rsi\n"
    "mov %%rcx, %%rdx\n"
    "mov %%r8 , %%r10\n"
    "mov %%r9 , %%r8\n"
    "syscall\n"
    "ret\n"
    :
    :
    : "memory"
  );
}


__attribute__((naked)) void* syscall6
(
  __attribute__((unused)) void* sys_code,
  __attribute__((unused)) void* arg0,
  __attribute__((unused)) void* arg1,
  __attribute__((unused)) void* arg2,
  __attribute__((unused)) void* arg3,
  __attribute__((unused)) void* arg4,
  __attribute__((unused)) void* arg5
)
{
  __asm__ __volatile__
  (
    "mov %%rdi, %%rax\n"
    "mov %%rsi, %%rdi\n"
    "mov %%rdx, %%rsi\n"
    "mov %%rcx, %%rdx\n"
    "mov %%r8 , %%r10\n"
    "mov %%r9 , %%r8\n"
    "mov 16(%%rsp), %%r9\n"
    "syscall\n"
    "ret\n"
    :
    :
    : "memory"
  );
}

#define SYS_READ  0x00
#define SYS_WRITE 0x01
#define SYS_CLOSE 0x03
#define SYS_POLL 0x07
#define SYS_MMAP 0x09
#define SYS_MUNMAP 0x0b
#define SYS_SOCKET 0x29
#define SYS_CONNECT 0x2a
#define SYS_SENDTO 0x2c
#define SYS_RECVFROM 0x2d
#define SYS_SETSOCKOPT 0x36
#define SYS_GETSOCKOPT 0x37

ssize_t write(int fd, const void* buf, size_t count)
{
  return (ssize_t)syscall3(
    (void*)SYS_WRITE,
    (void*)(isize)fd,
    (void*)buf,
    (void*)count
  );
}

ssize_t read(int fd, void* buf, size_t count)
{
  return (ssize_t)syscall3(
    (void*)SYS_READ,
    (void*)(isize)fd,
    (void*)buf,
    (void*)count
  );
}


void *mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  return syscall6(
    (void*)SYS_MMAP,
    (void*)addr,
    (void*)length,
    (void*)(isize)prot,
    (void*)(isize)flags,
    (void*)(isize)fd,
    (void*)offset
  );
}
int munmap(void* addr, size_t length)
{
  return (int)(isize)syscall2(
    (void*)SYS_MUNMAP,
    addr,
    (void*)length
  );
}

int close(int fd)
{
  return (int)(isize)syscall1(
    (void*)SYS_CLOSE,
    (void*)(isize)fd
  );
}

int socket(int domain, int type, int protocol)
{
  return (int)(isize)syscall3(
    (void*)SYS_SOCKET,
    (void*)(isize)domain,
    (void*)(isize)type,
    (void*)(isize)protocol
  );
}


ssize_t sendto(
  int sockfd,
  const void* buf,
  size_t size,
  int flags,
  const struct sockaddr *dest_addr,
  socklen_t addrlen
)
{
  return (ssize_t)syscall6(
    (void*)(isize)SYS_SENDTO,
    (void*)(isize)sockfd,
    (void*)buf,
    (void*)size,
    (void*)(isize)flags,
    (void*)dest_addr,
    (void*)(isize)addrlen
  );
}

ssize_t send(int sockfd, const void* buf, size_t size, int flags)
{
  return sendto(sockfd, buf, size, flags, NULL, 0);
}

ssize_t recvfrom(
  int sockfd,
  void* buf,
  size_t size,
  int flags,
  struct sockaddr *src_addr,
  socklen_t* addrlen
)
{
  return (ssize_t)syscall6(
    (void*)(isize)SYS_RECVFROM,
    (void*)(isize)sockfd,
    (void*)buf,
    (void*)size,
    (void*)(isize)flags,
    (void*)src_addr,
    (void*)(isize)addrlen
  );
}

ssize_t recv(int sockfd, void* buf, size_t size, int flags)
{
  return recvfrom(sockfd, buf, size, flags, NULL, NULL);
}


int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t *restrict optlen)
{
  return (int)(isize)syscall5(
    (void*)(isize)SYS_GETSOCKOPT,
    (void*)(isize)sockfd,
    (void*)(isize)level,
    (void*)(isize)optname,
    optval,
    (void*)optlen
  );
}


int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
  return (int)(isize)syscall5(
    (void*)(isize)SYS_GETSOCKOPT,
    (void*)(isize)sockfd,
    (void*)(isize)level,
    (void*)(isize)optname,
    (void*)optval,
    (void*)(isize)optlen
  );
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  return (int)(isize)syscall3(
    (void*)(isize)SYS_CONNECT,
    (void*)(isize)sockfd,
    (void*)addr,
    (void*)(isize)addrlen
  );
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  return (int)(isize)syscall3(
    (void*)SYS_POLL,
    (void*)fds,
    (void*)(isize)nfds,
    (void*)(isize)timeout
  );
}



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

const char* x11_error_strs[] = {
  #define X(e, s) s,
  ERRORS
  #undef X
};


typedef struct X11Request{
  u8 majorOpcode;
  u16 length;
  u8 data;
} __attribute__((packed)) X11Request;

typedef u32 X11Window;
typedef u32 X11Colormap;
typedef u32 X11Visual;
typedef u32 X11Drawable;
typedef u32 X11Atom;

typedef struct X11Connection{
  u8 byteOrder;
  u8 unused; // special thanks to the writers of the spec for inconsistent info between the appendix and inital connection chapter
  u16 protocolMajorVersion;
  u16 protocolMinorVersion;
  u16 authProtocolNameLength;
  u16 authProtocolDataLength;
  u16 unused2;
  //char data[]; // do not use! not gonna support auth anyway
} __attribute__((packed)) X11Connection;

typedef struct X11SetupSuccessResponse {
  // u8 code; // NOTE: only not in the struct because i read it first
  u8  unused;
  i16 protocolMajorVersion;
  i16 protocolMinorVersion;
  u16 extraDataLengthIn4Bytes;
  i32 releaseNumber;
  i32 recourceIdBase;
  i32 recourceIdMask;
  i32 motionBufferSize;
  u16 vendorLength;
  i16 maximumRequestLength;
  i8  screenCount;
  u8  pixmapFormatsCount;
  u8  imageByteOrder;
  u8  bitmapBitOrder;
  u8  bitmapFormatScanlineUnit;
  u8  bitmapFormatScanlinePad;
  u8  minKeycode;
  u8  maxKeycode;
  u32 unused2;
  // char* vendor;
  // *formats;
  // *screens;
} __attribute__((packed)) X11SetupSuccessResponse;

typedef struct X11Format {
  i8 depth, bitsPerLine, scanlinePad;
  u8 unused[5];
} __attribute__((packed)) X11Format;

typedef struct X11Screen {
  X11Window rootWindow;
  X11Colormap colormap;
  i32 whitePixel;
  i32 blackPixel;
  i32 currentInputMask;
  i16 widthInPixels;
  i16 heightInPixels;
  i16 widthInMillimeters;
  i16 heightInMillimeters;
  i16 minInstalledMaps;
  i16 maxInstalledMaps;
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
  u8 class;
  i8 bitsPerRgbValue;
  i16 colormapEntries;
  i32 redMask;
  i32 greenMask;
  i32 blueMask;
  u32 unused;
} __attribute__((packed)) X11VisualType;

#define X11_RESPONSE_SUCCESS 1

typedef struct X11CreateWindowRequest {
  u8 opcode;
  u8 depth;
  u16 requestLength; // 8 + n of values
  X11Window windowId;
  X11Window parent;
  i16 x, y;
  u16 width, height, borderWidth;
  u16 class;
  X11Visual visual;
  u32 bitmask;
  // list of values should be arbitrary
  // but here i only need one
  u32 values[1]; // list of value
} __attribute__((packed)) X11CreateWindowRequest;


void *memcpy(void *dest, const void *src, size_t n)
{
  for (size_t i = 0; i < n; i++)
  {
    ((char*)dest)[i] = ((char*)src)[i];
  }
  return dest;
}
#define PrintCstr(cstr) write(1, cstr, sizeof(cstr)-1)

void* alloc(usize size)
{
  void* mem = mmap(
    NULL,
    size,
    PROT_WRITE | PROT_READ,
    MAP_PRIVATE | MAP_ANONYMOUS,
    -1,
    0
  );
  if (mem == MAP_FAILED) mem = NULL;
  return mem;
}


int main(void)
{
  int programResult = 0;
  
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  int availableSocketBytes = 0;
  unsigned int optLen = 0;

  int res = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &availableSocketBytes, &optLen);
  if (res != 0)
  {
    PrintCstr("Could not set the bytes availability!\n");
    programResult = 1;
    goto end;
  }

  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  static const char x11path[] = "/tmp/.X11-unix/X0"; // TODO: get display from env
  memcpy(addr.sun_path, x11path, sizeof(x11path));

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
    PrintCstr("Could not open x11 server\n");
    programResult = 1;
    goto end;
  }

  // X11 connection initialization
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
    programResult = 1;
    goto end;
  }
  X11SetupSuccessResponse response = {0};
  recv(fd, &response, sizeof(response), 0);

  DebugPrintf("screens count: %d\n", response.screenCount);
  DebugPrintf("formats count: %u\n", response.pixmapFormatsCount);
  DebugPrintf("vendor length: %u\n", response.vendorLength);
  DebugPrintf("extra info len: %d\n", 4 * (int)response.extraDataLengthIn4Bytes);

  // align to nearest multiple of 4
  usize vendorLenAligned = (response.vendorLength+3) & ~0x03;

  usize screensSize = response.screenCount * sizeof(X11Screen);
  usize formatsSize = response.pixmapFormatsCount * sizeof(X11Format);
  usize extrasSize  = (usize)response.extraDataLengthIn4Bytes * 4;

  usize memLen = (usize)(
    (isize)screensSize +
    (isize)formatsSize +
    (isize)vendorLenAligned +
    (isize)extrasSize // @Cleanup no need to include this
  );
  DebugPrintf("bytes to alloc = %zu\n", memLen);
  void* mem = alloc(memLen);
  if (mem == NULL)
  {
    PrintCstr("Failed to map memory\n");
    goto end;
  }
  char* vendor = mem;
  X11Format* formats = (void*)((char*)mem + vendorLenAligned);
  X11Screen* screens = (void*)(formats + response.pixmapFormatsCount);
  char* extraData = (void*)(screens + response.screenCount);

  DebugPrintf("receiving extars\n", 0);
  //
  // NOTE: extraDataLength specifies ALL the memory necessery to
  // init x11
  //recv(fd, mem, (usize)response.extraDataLengthIn4Bytes * 4ULL, 0);
  //goto poll;
  
  // reveive vendor info
  if (vendorLenAligned > 0)
  {
    recv(fd, vendor, vendorLenAligned, 0);
  }
  DebugPrintf("vendor: %.*s\n", vendorLenAligned, vendor);

  // receive format infos
  if (response.pixmapFormatsCount > 0)
  {
    recv(fd, formats, formatsSize, 0);
  }

  DebugPrintf("Formats(%u):\n", response.pixmapFormatsCount);
  for (usize i = 0;i < response.pixmapFormatsCount;++i)
  {
    DebugPrintf("%zu: depth = %d, bitsPerLine = %d, scanLinePad = %d\n", i+1, formats[i].depth, formats[i].bitsPerLine, formats[i].scanlinePad);
  }

  // receive screens info
  usize ctr = 0;
  {
    recv(fd, screens, screensSize, 0);
    DebugPrintf("Screens(%d):\n", response.screenCount);
    for (i32 i = 0;i < response.screenCount;++i)
    {
      DebugPrintf("%u: depthsCount = %d\n", i+1, screens[i].allowedDepthsCount);

      // reveice depths info
      X11Depth depth = {0};
      for (i8 j = 0;j < screens[i].allowedDepthsCount;++j)
      {
        ctr += sizeof(X11Depth);
        recv(fd, &depth, sizeof(X11Depth), 0);
        //DebugPrintf("--depth%d: visual = %u\n", j+1, depth.numberOfVisuals);
        usize visualTypesSize = sizeof(X11VisualType) * (usize)depth.numberOfVisuals;

        if (visualTypesSize > 0)
        {
          X11VisualType* visualTypes = alloc(visualTypesSize);
          recv(fd, visualTypes, visualTypesSize, 0);
          for (u16 k = 0;k < depth.numberOfVisuals;++k)
          {
            ctr += sizeof(X11VisualType);
            X11VisualType* v = &visualTypes[k];
            //DebugPrintf("----visual%u: bits = %d, red = %x, green = %x, blue = %x\n", k, v->bitsPerRgbValue, v->redMask, v->greenMask, v->blueMask);
          }
          munmap(visualTypes, visualTypesSize);
        }
      }
    }
    
  }


  // receive extra info

#ifdef CLOSER_DEBUG
  DebugPrintf("\n", 0);
  struct pollfd pfd = {
    .fd = fd,
    .events = POLLIN,
  };
  int pollRes = poll(&pfd, 1, 1000);
  if (pollRes == 0)
  {
    DebugPrintf("No more initialization to do\n", 0);
  }
  else
  {
    DebugPrintf("Still work to do!\n", 0);
  }
#endif

// APP INITIALIZATION


//mem_unmap:
  munmap(mem, memLen);
end:
  close(fd);
  return programResult;
}

__attribute__((naked)) void _start(void)
{
  __asm__ __volatile__
  (
    "and $0xfffffffffffffff0, %rsp\n"
    "call main\n"
    "mov %rax, %rdi\n"
    "mov $60, %rax\n"
    "syscall\n"

    "ret\n" // should not get here
  );
}


// clang requires some libc functions
void* memset(void* dest, int value, size_t count)
{
  u8* buf = dest;
  usize i = 0;
  for (; i < count;++i)
  {
    buf[i] = (u8)value;
  }
  return dest;
}
