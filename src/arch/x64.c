
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

