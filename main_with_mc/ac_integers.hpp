#ifndef MY_CUSTOM_INTS
#define MY_CUSTOM_INTS

#include "ac_int.h"

typedef ac_int<64, true> Int64;
static const Int64 Int64_Max = 0x7FFFFFFFFFFFFFFF;  // +9,223,372,036,854,775,807
static const Int64 Int64_Min = 0x8000000000000000;  // -9,223,372,036,854,775,808

typedef ac_int<64, false> UInt64;
static const UInt64 UInt64_Max = 0xFFFFFFFFFFFFFFFF;// +18,446,744,073,709,551,615

typedef ac_int<32, true> Int32;
static const Int32 Int32_Max = 0x7FFFFFFF;          // +2,147,483,647
static const Int32 Int32_Min = 0x80000000;          // -2,147,483,648

typedef ac_int<32, false> UInt32;
static const UInt32 UInt32_Max = 0xFFFFFFFF;        // +4,294,967,295

typedef ac_int<16, true> Int16;
static const Int16 Int16_Max = 0x7FFF;              // +32,767
static const Int16 Int16_Min = 0x8000;              // −32,768

typedef ac_int<16, false> UInt16;
static const UInt16 UInt16_Max = 0xFFFF;            // +65,535

typedef ac_int<8, true> Int8;
static const Int8 Int8_Max = 0x7F;                  // +127
static const Int8 Int8_Min = 0x80;                  // -128

typedef ac_int<8, false> UInt8;
static const UInt8 UInt8_Max = 0xFF;                // +255

typedef ac_int<4, true> Int4;
static const Int4 Int4_Max = 0x7;                   // +7
static const Int4 Int4_Min = 0x8;                   // -8

typedef ac_int<4, false> UInt4;
static const UInt4 UInt4_Max = 0xF;                 // +15

typedef ac_int<2, false> UInt2;
static const UInt2 UInt2_Max = 0x3;                 // +3

typedef Int64 LongInt;
typedef Int32 Int;

typedef UInt64 UInt;
typedef UInt8 Depth;  // Depth 0-63 for 64-bit memory positions
typedef UInt2 Axis;   // Axis 0-2 for a max of 3 dimensions

#endif