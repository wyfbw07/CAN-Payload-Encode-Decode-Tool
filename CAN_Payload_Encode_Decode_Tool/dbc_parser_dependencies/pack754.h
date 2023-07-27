//
//  pack754.h
//  CAN_Databse_Encode_Decode_Tool
//
//  Created by Yifan Wang on 7/20/23.
//

#ifndef pack754_h
#define pack754_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

// macros for packing floats and doubles:
#define unpack754_64(i) (unpack754((i), 64, 11))
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

    // pack754() -- pack a floating point number into IEEE-754 format
    // unpack754() -- unpack a floating point number from IEEE-754 format
    uint64_t pack754(long double f, unsigned bits, unsigned expbits);
    long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

#ifdef __cplusplus
}
#endif

#endif /* pack754_h */
