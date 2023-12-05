#pragma once
#include <stdint.h>

// Signed
typedef signed __int8 i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

constexpr i8    I8_MAX = INT8_MAX;
constexpr i16   I16_MAX = INT16_MAX;
constexpr i32   I32_MAX = INT32_MAX;
constexpr i64   I64_MAX = INT64_MAX;

// Unsigned
typedef unsigned __int8     u8;
typedef unsigned __int16    u16;
typedef unsigned __int32    u32;
typedef unsigned __int64    u64;

constexpr u8    U8_MAX = UINT8_MAX;
constexpr u16   U16_MAX = UINT16_MAX;
constexpr u32   U32_MAX = UINT32_MAX;
constexpr u64   U64_MAX = UINT64_MAX;

// Floating pointer

using f32 = float;