#ifndef CLOSER_H
#define CLOSER_H

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


#define Unused(arg) (void)(arg)

#define local_persist static
#define global static
#define internal static

#define FALSE 0
#define TRUE 1

#endif
