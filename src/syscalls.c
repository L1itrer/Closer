#include "closer.h"
#ifdef __x86_64__
  #include "./arch/x64.c"
#else
  #error No syscall implementations defined for this architecture!
#endif

#include <sys/socket.h>
#include <sys/mman.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>
typedef ptrdiff_t ssize_t;
typedef ssize_t isize ;


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

pid_t fork(void)
{
  return (int)(isize)syscall0((void*)SYS_FORK);
}


int execve(const char *pathname, char *const argv[], char *const envp[])
{
  return (int)(isize)syscall3(
    (void*)SYS_EXECVE,
    (void*)pathname,
    (void*)argv,
    (void*)envp
  );
}


void exit(int status)
{
  syscall1((void*)SYS_EXIT, (void*)(isize)status);
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

void *memcpy(void *dest, const void *src, size_t n)
{
  for (size_t i = 0; i < n; i++)
  {
    ((char*)dest)[i] = ((char*)src)[i];
  }
  return dest;
}


size_t strlen(const char *s)
{
  const char* a = s;
  size_t count = 0;
  while (*a++ != 0) count++;
  return count;
}


