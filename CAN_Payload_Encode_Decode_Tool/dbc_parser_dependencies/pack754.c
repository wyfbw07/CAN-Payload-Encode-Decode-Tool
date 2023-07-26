
//
//  pack754.c
//  CAN_Databse_Encode_Decode_Tool
//
//  Created by Yifan Wang on 7/19/23.
//

#include "pack754.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

// Various bits for floating point types--varies for different architectures
typedef float float32_t;
typedef double float64_t;

// Macros for packing floats and doubles:
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

// Pack a floating point number into IEEE-754 format
uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    // get this special case out of the way
    if (f == 0.0){ return 0; }
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit
    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }
    // get the normalized form of f and track the exponent
    shift = 0;
    while (fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while (fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;
    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL << significandbits) + 0.5f);
    // get the biased exponent
    exp = shift + ((1 << (expbits - 1)) - 1); // shift + bias
    // return the final answer
    return (sign << (bits - 1)) | (exp << (bits - expbits - 1)) | significand;
}

// Unpack a floating point number from IEEE-754 format
long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    if (i == 0) { return 0.0; }
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit
    // pull the significand
    result = (i & ((1LL << significandbits) - 1)); // mask
    result /= (1LL << significandbits); // convert back to float
    result += 1.0f; // add the one back on
    // deal with the exponent
    bias = (1 << (expbits - 1)) - 1;
    shift = ((i >> significandbits) & ((1LL << expbits) - 1)) - bias;
    while (shift > 0) { result *= 2.0; shift--; }
    while (shift < 0) { result /= 2.0; shift++; }
    // sign it
    result *= (i >> (bits - 1)) & 1 ? -1.0 : 1.0;
    return result;
}
