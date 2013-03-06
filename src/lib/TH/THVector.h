#ifndef TH_VECTOR_INC
#define TH_VECTOR_INC

#include "THGeneral.h"

#define THVector_(NAME) TH_CONCAT_4(TH,Real,Vector_,NAME)

#if defined USE_SSE2 || defined USE_SSE3 || defined USE_SSSE3 \
  || defined USE_SSE4_1 || defined USE_SSE4_2

#ifdef USE_SSE2
#include <emmintrin.h>
#endif

#ifdef USE_SSE3
#include <pmmintrin.h>
#endif

#ifdef USE_SSSE3
#include <tmmintrin.h>
#endif

#if defined (USE_SSE4_2) || defined (USE_SSE4_1)
#include <smmintrin.h>
#endif

#define THDoubleVector_fill(x, c, n) {          \
    long i;                                     \
    __m128d XMM0 = _mm_set1_pd(c);              \
    for (i=0; i<=((n)-8); i+=8) {               \
      _mm_storeu_pd((x)+i  , XMM0);             \
      _mm_storeu_pd((x)+i+2, XMM0);             \
      _mm_storeu_pd((x)+i+4, XMM0);             \
      _mm_storeu_pd((x)+i+6, XMM0);             \
    }                                           \
    long off = (n) - ((n)%8);                   \
    for (i=0; i<((n)%8); i++) {                 \
      x[off+i] = c;                             \
    }                                           \
  }


#define THDoubleVector_add(y, x, c, n) {        \
    long i = 0;                                 \
    __m128d XMM7 = _mm_set1_pd(c);              \
    __m128d XMM0,XMM2;                          \
    for (; i<=((n)-2); i+=2) {                  \
      XMM0 = _mm_loadu_pd((x)+i);               \
      XMM2 = _mm_loadu_pd((y)+i);               \
      XMM0 = _mm_mul_pd(XMM0, XMM7);            \
      XMM2 = _mm_add_pd(XMM2, XMM0);            \
      _mm_storeu_pd((y)+i  , XMM2);             \
    }                                           \
    for (; i<(n); i++) {                        \
      y[i] += c * x[i];                         \
    }                                           \
  }

#define THDoubleVector_diff(z, x, y, n) {       \
    long i;                                     \
    for (i=0; i<=((n)-8); i+=8) {               \
      __m128d XMM0 = _mm_loadu_pd((x)+i  );     \
      __m128d XMM1 = _mm_loadu_pd((x)+i+2);     \
      __m128d XMM2 = _mm_loadu_pd((x)+i+4);     \
      __m128d XMM3 = _mm_loadu_pd((x)+i+6);     \
      __m128d XMM4 = _mm_loadu_pd((y)+i  );     \
      __m128d XMM5 = _mm_loadu_pd((y)+i+2);     \
      __m128d XMM6 = _mm_loadu_pd((y)+i+4);     \
      __m128d XMM7 = _mm_loadu_pd((y)+i+6);     \
      XMM0 = _mm_sub_pd(XMM0, XMM4);            \
      XMM1 = _mm_sub_pd(XMM1, XMM5);            \
      XMM2 = _mm_sub_pd(XMM2, XMM6);            \
      XMM3 = _mm_sub_pd(XMM3, XMM7);            \
      _mm_storeu_pd((z)+i  , XMM0);             \
      _mm_storeu_pd((z)+i+2, XMM1);             \
      _mm_storeu_pd((z)+i+4, XMM2);             \
      _mm_storeu_pd((z)+i+6, XMM3);             \
    }                                           \
    long off = (n) - ((n)%8);                   \
    for (i=0; i<((n)%8); i++) {                 \
      z[off+i] = x[off+i] - y[off+i];           \
    }                                           \
  }

#define THDoubleVector_scale(y, c, n) {         \
    long i;                                     \
    __m128d XMM7 = _mm_set1_pd(c);              \
    for (i=0; i<=((n)-4); i+=4) {               \
      __m128d XMM0 = _mm_loadu_pd((y)+i  );     \
      __m128d XMM1 = _mm_loadu_pd((y)+i+2);     \
      XMM0 = _mm_mul_pd(XMM0, XMM7);            \
      XMM1 = _mm_mul_pd(XMM1, XMM7);            \
      _mm_storeu_pd((y)+i  , XMM0);             \
      _mm_storeu_pd((y)+i+2, XMM1);             \
    }                                           \
    long off = (n) - ((n)%4);                   \
    for (i=0; i<((n)%4); i++) {                 \
      y[off+i] *= c;                            \
    }                                           \
  }

#define THDoubleVector_mul(y, x, n) {           \
    long i;                                     \
    for (i=0; i<=((n)-8); i+=8) {               \
      __m128d XMM0 = _mm_loadu_pd((x)+i  );     \
      __m128d XMM1 = _mm_loadu_pd((x)+i+2);     \
      __m128d XMM2 = _mm_loadu_pd((x)+i+4);     \
      __m128d XMM3 = _mm_loadu_pd((x)+i+6);     \
      __m128d XMM4 = _mm_loadu_pd((y)+i  );     \
      __m128d XMM5 = _mm_loadu_pd((y)+i+2);     \
      __m128d XMM6 = _mm_loadu_pd((y)+i+4);     \
      __m128d XMM7 = _mm_loadu_pd((y)+i+6);     \
      XMM4 = _mm_mul_pd(XMM4, XMM0);            \
      XMM5 = _mm_mul_pd(XMM5, XMM1);            \
      XMM6 = _mm_mul_pd(XMM6, XMM2);            \
      XMM7 = _mm_mul_pd(XMM7, XMM3);            \
      _mm_storeu_pd((y)+i  , XMM4);             \
      _mm_storeu_pd((y)+i+2, XMM5);             \
      _mm_storeu_pd((y)+i+4, XMM6);             \
      _mm_storeu_pd((y)+i+6, XMM7);             \
    }                                           \
    long off = (n) - ((n)%8);                   \
    for (i=0; i<((n)%8); i++) {                 \
      y[off+i] *= x[off+i];                     \
    }                                           \
  }

#define THFloatVector_fill(x, c, n) {           \
    long i;                                     \
    __m128 XMM0 = _mm_set_ps1(c);               \
    for (i=0; i<=((n)-16); i+=16) {             \
      _mm_storeu_ps((x)+i  ,  XMM0);            \
      _mm_storeu_ps((x)+i+4,  XMM0);            \
      _mm_storeu_ps((x)+i+8,  XMM0);            \
      _mm_storeu_ps((x)+i+12, XMM0);            \
    }                                           \
    long off = (n) - ((n)%16);                  \
    for (i=0; i<((n)%16); i++) {                \
      x[off+i] = c;                             \
    }                                           \
  }

#define THFloatVector_add(y, x, c, n) {         \
    long i = 0;                                 \
    __m128 XMM7 = _mm_set_ps1(c);               \
    __m128 XMM0,XMM2;                           \
    for (; i<=((n)-4); i+=4) {                  \
      XMM0 = _mm_loadu_ps((x)+i);               \
      XMM2 = _mm_loadu_ps((y)+i);               \
      XMM0 = _mm_mul_ps(XMM0, XMM7);            \
      XMM2 = _mm_add_ps(XMM2, XMM0);            \
      _mm_storeu_ps((y)+i  , XMM2);             \
    }                                           \
    for (; i<(n); i++) {                        \
      y[i] += c * x[i];                         \
    }                                           \
  }

#define THFloatVector_diff(z, x, y, n) {        \
    long i;                                     \
    for (i=0; i<=((n)-16); i+=16) {             \
      __m128 XMM0 = _mm_loadu_ps((x)+i   );     \
      __m128 XMM1 = _mm_loadu_ps((x)+i+ 4);     \
      __m128 XMM2 = _mm_loadu_ps((x)+i+ 8);     \
      __m128 XMM3 = _mm_loadu_ps((x)+i+12);     \
      __m128 XMM4 = _mm_loadu_ps((y)+i   );     \
      __m128 XMM5 = _mm_loadu_ps((y)+i+ 4);     \
      __m128 XMM6 = _mm_loadu_ps((y)+i+ 8);     \
      __m128 XMM7 = _mm_loadu_ps((y)+i+12);     \
      XMM0 = _mm_sub_ps(XMM0, XMM4);            \
      XMM1 = _mm_sub_ps(XMM1, XMM5);            \
      XMM2 = _mm_sub_ps(XMM2, XMM6);            \
      XMM3 = _mm_sub_ps(XMM3, XMM7);            \
      _mm_storeu_ps((z)+i   , XMM0);            \
      _mm_storeu_ps((z)+i+ 4, XMM1);            \
      _mm_storeu_ps((z)+i+ 8, XMM2);            \
      _mm_storeu_ps((z)+i+12, XMM3);            \
    }                                           \
    long off = (n) - ((n)%16);                  \
    for (i=0; i<((n)%16); i++) {                \
      z[off+i] = x[off+i] - y[off+i];           \
    }                                           \
  }

#define THFloatVector_scale(y, c, n) {          \
    long i;                                     \
    __m128 XMM7 = _mm_set_ps1(c);               \
    for (i=0; i<=((n)-8); i+=8) {               \
      __m128 XMM0 = _mm_loadu_ps((y)+i  );      \
      __m128 XMM1 = _mm_loadu_ps((y)+i+4);      \
      XMM0 = _mm_mul_ps(XMM0, XMM7);            \
      XMM1 = _mm_mul_ps(XMM1, XMM7);            \
      _mm_storeu_ps((y)+i  , XMM0);             \
      _mm_storeu_ps((y)+i+4, XMM1);             \
    }                                           \
    long off = (n) - ((n)%8);                   \
    for (i=0; i<((n)%8); i++) {                 \
      y[off+i] *= c;                            \
    }                                           \
  }

#define THFloatVector_mul(y, x, n) {            \
    long i;                                     \
    for (i=0; i<=((n)-16); i+=16) {             \
      __m128 XMM0 = _mm_loadu_ps((x)+i   );     \
      __m128 XMM1 = _mm_loadu_ps((x)+i+ 4);     \
      __m128 XMM2 = _mm_loadu_ps((x)+i+ 8);     \
      __m128 XMM3 = _mm_loadu_ps((x)+i+12);     \
      __m128 XMM4 = _mm_loadu_ps((y)+i   );     \
      __m128 XMM5 = _mm_loadu_ps((y)+i+ 4);     \
      __m128 XMM6 = _mm_loadu_ps((y)+i+ 8);     \
      __m128 XMM7 = _mm_loadu_ps((y)+i+12);     \
      XMM4 = _mm_mul_ps(XMM4, XMM0);            \
      XMM5 = _mm_mul_ps(XMM5, XMM1);            \
      XMM6 = _mm_mul_ps(XMM6, XMM2);            \
      XMM7 = _mm_mul_ps(XMM7, XMM3);            \
      _mm_storeu_ps((y)+i   , XMM4);            \
      _mm_storeu_ps((y)+i+ 4, XMM5);            \
      _mm_storeu_ps((y)+i+ 8, XMM6);            \
      _mm_storeu_ps((y)+i+12, XMM7);            \
    }                                           \
    long off = (n) - ((n)%16);                  \
    for (i=0; i<((n)%16); i++) {                \
      y[off+i] *= x[off+i];                     \
    }                                           \
  }

#elif defined __NEON__
// ARM NEON Assembly routine for operating on floats

#define THFloatVector_fill(x, c, n) {                   \
        float ctemp = c;                                \
        float * caddr = &ctemp;                         \
        __asm__ __volatile__ (                          \
            "mov         r0, %0           @ \n\t"       \
            "ldr         r4, [%1]         @ \n\t"       \
            "vdup.32     q12, r4          @ \n\t"       \
            "vdup.32     q13, r4          @ \n\t"       \
            "lsrs        r4, %2, #3       @ \n\t"       \
            "beq         3f               @ \n\t"       \
            "1:                           @ \n\t"       \
            "vst1.32     {d24-d27}, [r0]! @ \n\t"       \
            "subs        r4, r4, #1       @ \n\t"       \
            "bne         1b               @ \n\t"       \
            "3:                           @ \n\t"       \
            "ands        r4, %2, #7       @ \n\t"       \
            "beq         5f               @ \n\t"       \
            "4:                           @ \n\t"       \
            "subs        r4, r4, #1       @ \n\t"       \
            "vst1.32     {d24[0]}, [r0]!  @ \n\t"       \
            "bne         4b               @ \n\t"       \
            "5:                           @ "           \
            :                                           \
            :"r" (x), "r"(caddr),"r"(n)                 \
            : "cc", "r0", "r4",  "memory",              \
              "q12",                                    \
              "d24", "d25", "d26", "d27"                \
            );                                          \
    }

#define THFloatVector_diff(z, x, y, n) {                                \
        __asm__ __volatile__ (                                          \
            "mov         r0, %2           @ \n\t"                       \
            "mov         r1, %1           @ \n\t"                       \
            "mov         r2, %0           @ \n\t"                       \
            "lsrs        r4, %3, #3       @ \n\t"                       \
            "beq         3f               @ \n\t"                       \
            "vld1.32     {d16-d19}, [r1]! @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "1:                           @ \n\t"                       \
            "vsub.f32    q12, q8, q0      @ \n\t"                       \
            "vsub.f32    q13, q9, q1      @ \n\t"                       \
            "subs        r4, r4, #1       @ \n\t"                       \
            "beq         2f               @ \n\t"                       \
            "vld1.32     {d16-d19}, [r1]! @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vst1.32     {d24-d27}, [r2]! @ \n\t"                       \
            "b           1b               @ \n\t"                       \
            "2:                           @ \n\t"                       \
            "vst1.32     {d24-d27}, [r2]! @ \n\t"                       \
            "3:                           @ \n\t"                       \
            "ands        r4, %3, #7       @ \n\t"                       \
            "beq         5f               @ \n\t"                       \
            "4:                           @ \n\t"                       \
            "subs        r4, r4, #1       @ \n\t"                       \
            "vld1.32     {d16[0]}, [r1]!  @ \n\t"                       \
            "vld1.32     {d0[0]}, [r0]!   @ \n\t"                       \
            "vsub.f32    d24, d16, d0     @ \n\t"                       \
            "vst1.32     {d24[0]}, [r2]!  @ \n\t"                       \
            "bne         4b               @ \n\t"                       \
            "5:                           @ "                           \
            :                                                           \
            :"r" (z), "r" (x),"r" (y), "r"(n)                           \
            : "cc", "r0", "r1", "r2", "r4", "memory",                   \
              "q0", "q1", "q8", "q9", "q12", "q13",                     \
              "d0", "d1", "d2", "d3",                                   \
              "d16", "d17", "d18", "d19", "d24", "d25", "d26", "d27"    \
            );                                                          \
    }

#define THFloatVector_scale(y, c, n) {                                  \
        float ctemp = c;                                                \
        float * caddr = &ctemp;                                         \
        __asm__ __volatile__ (                                          \
            "mov         r0, %0           @ \n\t"                       \
            "mov         r2, r0           @ \n\t"                       \
            "ldr         r5, [%1]         @ \n\t"                       \
            "vdup.32     q14, r5          @ \n\t"                       \
            "lsrs        r5, %2, #5       @ \n\t"                       \
            "beq         3f               @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vld1.32     {d4-d7}, [r0]!   @ \n\t"                       \
            "vld1.32     {d8-d11}, [r0]!  @ \n\t"                       \
            "vld1.32     {d12-d15}, [r0]! @ \n\t"                       \
            "1:                           @ \n\t"                       \
            "vmul.f32    q0, q0, q14      @ \n\t"                       \
            "vmul.f32    q1, q1, q14      @ \n\t"                       \
            "vmul.f32    q2, q2, q14      @ \n\t"                       \
            "vmul.f32    q3, q3, q14      @ \n\t"                       \
            "vmul.f32    q4, q4, q14      @ \n\t"                       \
            "vmul.f32    q5, q5, q14      @ \n\t"                       \
            "vmul.f32    q6, q6, q14      @ \n\t"                       \
            "vmul.f32    q7, q7, q14      @ \n\t"                       \
            "subs        r5, r5, #1       @ \n\t"                       \
            "beq         2f               @ \n\t"                       \
            "vst1.32     {d0-d3}, [r2]!   @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vst1.32     {d4-d7}, [r2]!   @ \n\t"                       \
            "vld1.32     {d4-d7}, [r0]!   @ \n\t"                       \
            "vst1.32     {d8-d11}, [r2]!  @ \n\t"                       \
            "vld1.32     {d8-d11}, [r0]!  @ \n\t"                       \
            "vst1.32     {d12-d15}, [r2]! @ \n\t"                       \
            "vld1.32     {d12-d15}, [r0]! @ \n\t"                       \
            "b           1b               @ \n\t"                       \
            "2:                           @ \n\t"                       \
            "vst1.32     {d0-d3}, [r2]!   @ \n\t"                       \
            "vst1.32     {d4-d7}, [r2]!   @ \n\t"                       \
            "vst1.32     {d8-d11}, [r2]!  @ \n\t"                       \
            "vst1.32     {d12-d15}, [r2]! @ \n\t"                       \
            "3:                           @ \n\t"                       \
            "lsrs        r5, %2, #4       @ \n\t"                       \
            "ands        r5, r5, #1       @ \n\t"                       \
            "beq         4f               @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vld1.32     {d4-d7}, [r0]!   @ \n\t"                       \
            "vmul.f32    q0, q0, q14      @ \n\t"                       \
            "vmul.f32    q1, q1, q14      @ \n\t"                       \
            "vmul.f32    q2, q2, q14      @ \n\t"                       \
            "vmul.f32    q3, q3, q14      @ \n\t"                       \
            "vst1.32     {d0-d3}, [r2]!   @ \n\t"                       \
            "vst1.32     {d4-d7}, [r2]!   @ \n\t"                       \
            "4:                           @ \n\t"                       \
            "lsrs        r5, %2, #3       @ \n\t"                       \
            "ands        r5, r5, #1       @ \n\t"                       \
            "beq         5f               @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vmul.f32    q0, q0, q14      @ \n\t"                       \
            "vmul.f32    q1, q1, q14      @ \n\t"                       \
            "vst1.32     {d0-d3}, [r2]!   @ \n\t"                       \
            "5:                           @ \n\t"                       \
            "ands        r5, %2, #7       @ \n\t"                       \
            "beq         7f               @ \n\t"                       \
            "6:                           @ \n\t"                       \
            "subs        r5, r5, #1       @ \n\t"                       \
            "vld1.32     d0[0], [r0]!     @ \n\t"                       \
            "vmul.f32    d0, d0, d28      @ \n\t"                       \
            "vst1.32     d0[0], [r2]!     @ \n\t"                       \
            "bne         6b               @ \n\t"                       \
            "7:                           @ "                           \
            :                                                           \
            :"r" (y), "r"(caddr),"r"(n)                                 \
            : "cc", "r0", "r2", "r5", "memory",                         \
              "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q14",    \
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",           \
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",     \
              "d28", "d29"                                              \
            );                                                          \
    }

#define THFloatVector_mul(y, x, n) {                                    \
        __asm__ __volatile__ (                                          \
            "mov         r0, %0           @ \n\t"                       \
            "mov         r1, %1           @ \n\t"                       \
            "mov         r2, r0           @ \n\t"                       \
            "lsrs        r4, %2, #3       @ \n\t"                       \
            "beq         3f               @ \n\t"                       \
            "vld1.32     {d16-d19}, [r1]! @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "1:                           @ \n\t"                       \
            "vmul.f32    q12, q8, q0      @ \n\t"                       \
            "vmul.f32    q13, q9, q1      @ \n\t"                       \
            "subs        r4, r4, #1       @ \n\t"                       \
            "beq         2f               @ \n\t"                       \
            "vld1.32     {d16-d19}, [r1]! @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vst1.32     {d24-d27}, [r2]! @ \n\t"                       \
            "b           1b               @ \n\t"                       \
            "2:                           @ \n\t"                       \
            "vst1.32     {d24-d27}, [r2]! @ \n\t"                       \
            "3:                           @ \n\t"                       \
            "ands        r4, %2, #7       @ \n\t"                       \
            "beq         5f               @ \n\t"                       \
            "4:                           @ \n\t"                       \
            "subs        r4, r4, #1       @ \n\t"                       \
            "vld1.32     {d16[0]}, [r1]!  @ \n\t"                       \
            "vld1.32     {d0[0]}, [r0]!   @ \n\t"                       \
            "vmul.f32    q12, q8, q0      @ \n\t"                       \
            "vst1.32     {d24[0]}, [r2]!  @ \n\t"                       \
            "bne         4b               @ \n\t"                       \
            "5:                           @ "                           \
            :                                                           \
            :"r" (y),"r" (x),"r"(n)                                     \
            : "cc", "r0", "r1", "r2", "r4", "memory",                   \
              "q0", "q1", "q8", "q9", "q12", "q13",                     \
              "d0", "d1", "d2", "d3",                                   \
              "d16", "d17", "d18", "d19", "d24", "d25", "d26", "d27"    \
            );                                                          \
    }
#define THFloatVector_add(y, x, c, n) {                                 \
        float ctemp = c;                                                \
        float * caddr = &ctemp;                                         \
        __asm__ __volatile__ (                                          \
            "mov         r0, %0           @ \n\t"                       \
            "mov         r1, %1           @ \n\t"                       \
            "mov         r2, r0           @ \n\t"                       \
            "ldr         r5, [%2]         @ \n\t"                       \
            "vdup.32     q14, r5          @ \n\t"                       \
            "lsrs        r5, %3, #4       @ \n\t"                       \
            "beq         3f               @ \n\t"                       \
            "vld1.32     {d16-d19}, [r1]! @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vld1.32     {d20-d23}, [r1]! @ \n\t"                       \
            "vld1.32     {d4-d7}, [r0]!   @ \n\t"                       \
            "1:                           @ \n\t"                       \
            "vmla.f32    q0, q8, q14      @ \n\t"                       \
            "vmla.f32    q1, q9, q14      @ \n\t"                       \
            "vmla.f32    q2, q10, q14     @ \n\t"                       \
            "vmla.f32    q3, q11, q14     @ \n\t"                       \
            "subs        r5, r5, #1       @ \n\t"                       \
            "beq         2f               @ \n\t"                       \
            "vld1.32     {d16-d19}, [r1]! @ \n\t"                       \
            "vld1.32     {d20-d23}, [r1]! @ \n\t"                       \
            "vst1.32     {d0-d3}, [r2]!   @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vst1.32     {d4-d7}, [r2]!   @ \n\t"                       \
            "vld1.32     {d4-d7}, [r0]!   @ \n\t"                       \
            "b           1b               @ \n\t"                       \
            "2:                           @ \n\t"                       \
            "vst1.32     {d0-d3}, [r2]!   @ \n\t"                       \
            "vst1.32     {d4-d7}, [r2]!   @ \n\t"                       \
            "3:                           @ \n\t"                       \
            "lsrs        r5, %3, #3       @ \n\t"                       \
            "ands        r5, #1           @ \n\t"                       \
            "beq         4f               @ \n\t"                       \
            "vld1.32     {d16-d19}, [r1]! @ \n\t"                       \
            "vld1.32     {d0-d3}, [r0]!   @ \n\t"                       \
            "vmla.f32    q0, q8, q14      @ \n\t"                       \
            "vmla.f32    q1, q9, q14      @ \n\t"                       \
            "vst1.32     {d0-d3}, [r2]!   @ \n\t"                       \
            "4:                           @ \n\t"                       \
            "ands        r5, %3, #7       @ \n\t"                       \
            "beq         6f               @ \n\t"                       \
            "5:                           @ \n\t"                       \
            "subs        r5, r5, #1       @ \n\t"                       \
            "vld1.32     {d16[0]}, [r1]!  @ \n\t"                       \
            "vld1.32     {d0[0]}, [r0]!   @ \n\t"                       \
            "vmla.f32    d0, d16, d28     @ \n\t"                       \
            "vst1.32     d0[0], [r2]!     @ \n\t"                       \
            "bne         5b               @ \n\t"                       \
            "6:                           @ "                           \
            :                                                           \
            :"r" (y),"r" (x), "r"(caddr),"r"(n)                         \
            : "cc", "r0", "r1", "r2", "r5", "memory",                   \
              "q0", "q1", "q2", "q3", "q14",                            \
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",           \
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d28", "d29" \
            );                                                          \
    }


#define THFloatVector_conv1d(y, x, c, alpha, n, cn, reverse) {         \
        float xtmp[48];                                                 \
        float ytmp[16];                                                 \
        float ctmp[16];                                                 \
        float *argptr[6];                                               \
        float alphatmp = alpha;                                         \
        int i, j, csize;                                                \
        int Xsize;                                                      \
        argptr[0] = &alphatmp;                                          \
        argptr[1] = y;                                                  \
        argptr[4] = ytmp;                                               \
        argptr[5] = xtmp;                                               \
        if((n) & 15){                                                   \
            memcpy(ytmp, &y[n & 0xfffffff0], sizeof(float) * ((n) & 15)); \
        }                                                               \
        for(i = 0; i < cn; i+=16) {                                     \
            csize = (cn - i);                                           \
            if(csize > 16)                                              \
                csize = 16;                                             \
            if(reverse){                                                \
                argptr[3] = ctmp;                                       \
                for(j = 0; j < csize; j++){                             \
                  ctmp[j] = c[-j-i];                                    \
                }                                                       \
            }                                                           \
            else                                                        \
                argptr[3] = (c) + i;                                    \
            argptr[2] = x + i;                                          \
            Xsize = (n + csize - 1);                                    \
            if(Xsize >=16){                                             \
                memcpy(xtmp, &argptr[2][(Xsize & 0xfffffff0) - 16], sizeof(float) * ((Xsize & 31) + 16)); \
            }                                                           \
            else {                                                      \
                memcpy(xtmp, &argptr[2][0], sizeof(float) * Xsize);        \
            }                                                           \
            __asm__ __volatile__ (                                      \
                "@push        {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12} @ \n\t" \
                "ldr         r0, [%0]         @ \n\t"                   \
                "vld1.32     d8[0], [r0]      @ Load alpha coef \n\t"   \
                "ldr         r0, [%0, #12]    @ \n\t"                   \
                "cmp         %2, #4           @ \n\t"                   \
                "bge         18f              @ \n\t"                   \
                "vld1.32     d0[0], [r0]!     @ Load Kernel elements \n\t" \
                "cmp         %2, #2           @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d0[1], [r0]!     @ Load Kernel elements \n\t" \
                "cmp         %2, #3           @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d1[0], [r0]!     @ \n\t"                   \
                "b           30f              @ \n\t"                   \
                "18:                          @ Kernel loaded \n\t"     \
                "cmp         %2, #8           @ \n\t"                   \
                "bge         112f             @ \n\t"                   \
                "vld1.32     {d0-d1}, [r0]!   @ Load Kernel elements \n\t" \
                "cmp         %2, #5           @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d2[0], [r0]!     @ Load Kernel elements \n\t" \
                "cmp         %2, #6           @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d2[1], [r0]!     @ \n\t"                   \
                "cmp         %2, #7           @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d3[0], [r0]!     @ \n\t"                   \
                "b           30f              @ \n\t"                   \
                "112:                         @ Kernel loaded \n\t"     \
                "cmp         %2, #12          @ \n\t"                   \
                "bge         116f             @ \n\t"                   \
                "vld1.32     {d0-d3}, [r0]!   @ Load Kernel elements \n\t" \
                "cmp         %2, #9           @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d4[0], [r0]!     @ Load Kernel elements \n\t" \
                "cmp         %2, #10          @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d4[1], [r0]!     @ \n\t"                   \
                "cmp         %2, #11          @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d5[0], [r0]!     @ \n\t"                   \
                "b           30f              @ \n\t"                   \
                "116:                         @ Kernel loaded \n\t"     \
                "vld1.32     {d0-d3}, [r0]!   @ Load Kernel elements \n\t" \
                "vld1.32     {d4-d5}, [r0]!   @ Load Kernel elements \n\t" \
                "cmp         %2, #13          @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d6[0], [r0]!     @ Load Kernel elements \n\t" \
                "cmp         %2, #14          @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d6[1], [r0]!     @ \n\t"                   \
                "cmp         %2, #15          @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d7[0], [r0]!     @ \n\t"                   \
                "cmp         %2, #16          @ \n\t"                   \
                "blt         30f              @ \n\t"                   \
                "vld1.32     d7[1], [r0]!     @ \n\t"                   \
                "30:                          @ Kernel loaded \n\t"     \
                "vmul.f32    q0, q0, d8[0]    @ Multiply kernel coef by alpha \n\t" \
                "vmul.f32    q1, q1, d8[0]    @ \n\t"                   \
                "vmul.f32    q2, q2, d8[0]    @ \n\t"                   \
                "vmul.f32    q3, q3, d8[0]    @ \n\t"                   \
                "mov         r4, %1           @ Number of element in Vector X \n\t" \
                "adds        r4, %2           @ Number of element in Vector X \n\t" \
                "subs        r4, #1           @ Number of element in Vector X \n\t" \
                "lsrs        r4, r4, #4       @ Number of 16 X packets in vectors \n\t" \
                "subs        r4, #1           @ Number of Loads in Vector X \n\t" \
                "ldr         r0, [%0, #4]     @ Load Y ptr \n\t"        \
                "ldr         r1, [%0, #8]     @ Load X ptr \n\t"        \
                "mov         r3, #1           @ Flag used to know load/store remain elements \n\t" \
                "mov         r2, r0           @ \n\t"                   \
                "lsrs        r6, %1, #4       @ Number of 16 Y packets in vectors \n\t" \
                "beq         4f               @ \n\t"                   \
                "vld1.32     {d8-d11}, [r1]!  @ Preload X 32 first elements \n\t" \
                "vld1.32     {d12-d15}, [r1]! @ \n\t"                   \
                "vld1.32     {d16-d19}, [r1]! @ \n\t"                   \
                "vld1.32     {d24-d27}, [r0]! @ Preload Y 16 first elements  \n\t" \
                "vld1.32     {d28-d31}, [r0]! @ \n\t"                   \
                "vld1.32     {d20-d23}, [r1]! @ \n\t"                   \
                "subs        r1, r1, #64      @ Keep the X pointer ready for the second packet \n\t" \
                "1:                           @ \n\t"                   \
                "15913:                       @ Kernel size 1, 5, 9, 13 \n\t" \
                "vmla.f32    q12, q4, d0[0]   @ \n\t"                   \
                "vmla.f32    q13, q5, d0[0]   @ \n\t"                   \
                "vmla.f32    q14, q6, d0[0]   @ \n\t"                   \
                "vmla.f32    q15, q7, d0[0]   @ \n\t"                   \
                "cmp         %2, #5           @ \n\t"                   \
                "blt         371115f          @ \n\t"                   \
                "vmla.f32    q12, q5, d2[0]   @ \n\t"                   \
                "vmla.f32    q13, q6, d2[0]   @ \n\t"                   \
                "vmla.f32    q14, q7, d2[0]   @ \n\t"                   \
                "vmla.f32    q15, q8, d2[0]   @ \n\t"                   \
                "cmp         %2, #9           @ \n\t"                   \
                "blt         371115f          @ \n\t"                   \
                "vmla.f32    q12, q6, d4[0]   @ \n\t"                   \
                "vmla.f32    q13, q7, d4[0]   @ \n\t"                   \
                "vmla.f32    q14, q8, d4[0]   @ \n\t"                   \
                "vmla.f32    q15, q9, d4[0]   @ \n\t"                   \
                "cmp         %2, #13          @ \n\t"                   \
                "blt         371115f          @ \n\t"                   \
                "vmla.f32    q12, q7, d6[0]   @ \n\t"                   \
                "vmla.f32    q13, q8, d6[0]   @ \n\t"                   \
                "vmla.f32    q14, q9, d6[0]   @ \n\t"                   \
                "vmla.f32    q15, q10, d6[0]  @ \n\t"                   \
                "371115:                      @ Kernel size 3, 7, 11, 15 \n\t" \
                "cmp         %2, #3           @ \n\t"                   \
                "blt         9f               @ If less than 2 then epilog \n\t" \
                "vmla.f32    d24, d9, d1[0]   @ \n\t"                   \
                "vmla.f32    d25, d10, d1[0]  @ \n\t"                   \
                "vmla.f32    d26, d11, d1[0]  @ \n\t"                   \
                "vmla.f32    d27, d12, d1[0]  @ \n\t"                   \
                "vmla.f32    d28, d13, d1[0]  @ \n\t"                   \
                "vmla.f32    d29, d14, d1[0]  @ \n\t"                   \
                "vmla.f32    d30, d15, d1[0]  @ \n\t"                   \
                "vmla.f32    d31, d16, d1[0]  @ \n\t"                   \
                "cmp         %2, #7           @ \n\t"                   \
                "blt         9f               @ \n\t"                   \
                "vmla.f32    d24, d11, d3[0]  @ \n\t"                   \
                "vmla.f32    d25, d12, d3[0]  @ \n\t"                   \
                "vmla.f32    d26, d13, d3[0]  @ \n\t"                   \
                "vmla.f32    d27, d14, d3[0]  @ \n\t"                   \
                "vmla.f32    d28, d15, d3[0]  @ \n\t"                   \
                "vmla.f32    d29, d16, d3[0]  @ \n\t"                   \
                "vmla.f32    d30, d17, d3[0]  @ \n\t"                   \
                "vmla.f32    d31, d18, d3[0]  @ \n\t"                   \
                "cmp         %2, #11          @ \n\t"                   \
                "blt         9f               @ \n\t"                   \
                "vmla.f32    d24, d13, d5[0]  @ \n\t"                   \
                "vmla.f32    d25, d14, d5[0]  @ \n\t"                   \
                "vmla.f32    d26, d15, d5[0]  @ \n\t"                   \
                "vmla.f32    d27, d16, d5[0]  @ \n\t"                   \
                "vmla.f32    d28, d17, d5[0]  @ \n\t"                   \
                "vmla.f32    d29, d18, d5[0]  @ \n\t"                   \
                "vmla.f32    d30, d19, d5[0]  @ \n\t"                   \
                "vmla.f32    d31, d20, d5[0]  @ \n\t"                   \
                "cmp         %2, #15          @ \n\t"                   \
                "blt         9f               @ \n\t"                   \
                "vmla.f32    d24, d15, d7[0]  @ \n\t"                   \
                "vmla.f32    d25, d16, d7[0]  @ \n\t"                   \
                "vmla.f32    d26, d17, d7[0]  @ \n\t"                   \
                "vmla.f32    d27, d18, d7[0]  @ \n\t"                   \
                "vmla.f32    d28, d19, d7[0]  @ \n\t"                   \
                "vmla.f32    d29, d20, d7[0]  @ \n\t"                   \
                "vmla.f32    d30, d21, d7[0]  @ \n\t"                   \
                "vmla.f32    d31, d22, d7[0]  @ \n\t"                   \
                "9:                           @ Shift X NEON registers \n\t" \
                "vext.32     q4 , q4, q5, #1  @ Shift X by one element \n\t" \
                "vext.32     q5 , q5, q6, #1  @ Shift X by one element \n\t" \
                "vext.32     q6 , q6, q7, #1  @ Shift X by one element \n\t" \
                "vext.32     q7 , q7, q8, #1  @ Shift X by one element \n\t" \
                "vext.32     q8 , q8, q9, #1  @ Shift X by one element \n\t" \
                "cmp         %2, #8           @ @ quit early if small kernel\n\t" \
                "blt         261014f          @ \n\t"                   \
                "vext.32     q9 , q9, q10, #1 @ Shift X by one element \n\t" \
                "cmp         %2, #12          @ \n\t"                   \
                "blt         261014f          @ \n\t"                   \
                "vext.32     q10,q10, q11, #1 @ Shift X by one element \n\t" \
                "cmp         %2, #16          @ \n\t"                   \
                "blt         261014f          @ \n\t"                   \
                "vext.32     q11,q11, q11, #1 @ Shift X by one element \n\t" \
                "261014:                      @ Kernel size 2, 6, 10, 14 \n\t" \
                "cmp         %2, #2           @ \n\t"                   \
                "blt         481216f          @ \n\t"                   \
                "vmla.f32    q12, q4, d0[1]   @ \n\t"                   \
                "vmla.f32    q13, q5, d0[1]   @ \n\t"                   \
                "vmla.f32    q14, q6, d0[1]   @ \n\t"                   \
                "vmla.f32    q15, q7, d0[1]   @ \n\t"                   \
                "cmp         %2, #6           @ \n\t"                   \
                "blt         481216f          @ \n\t"                   \
                "vmla.f32    q12, q5, d2[1]   @ \n\t"                   \
                "vmla.f32    q13, q6, d2[1]   @ \n\t"                   \
                "vmla.f32    q14, q7, d2[1]   @ \n\t"                   \
                "vmla.f32    q15, q8, d2[1]   @ \n\t"                   \
                "cmp         %2, #10          @ \n\t"                   \
                "blt         481216f          @ \n\t"                   \
                "vmla.f32    q12, q6, d4[1]   @ \n\t"                   \
                "vmla.f32    q13, q7, d4[1]   @ \n\t"                   \
                "vmla.f32    q14, q8, d4[1]   @ \n\t"                   \
                "vmla.f32    q15, q9, d4[1]   @ \n\t"                   \
                "cmp         %2, #14          @ \n\t"                   \
                "blt         481216f          @ \n\t"                   \
                "vmla.f32    q12, q7, d6[1]   @ \n\t"                   \
                "vmla.f32    q13, q8, d6[1]   @ \n\t"                   \
                "vmla.f32    q14, q9, d6[1]   @ \n\t"                   \
                "vmla.f32    q15, q10, d6[1]  @ \n\t"                   \
                "481216:                      @ Kernel size 4, 8, 12, 16 \n\t" \
                "cmp         %2, #4           @ \n\t"                   \
                "blt         2f               @ \n\t"                   \
                "vmla.f32    d24, d9, d1[1]   @ \n\t"                   \
                "vmla.f32    d25, d10, d1[1]  @ \n\t"                   \
                "vmla.f32    d26, d11, d1[1]  @ \n\t"                   \
                "vmla.f32    d27, d12, d1[1]  @ \n\t"                   \
                "vmla.f32    d28, d13, d1[1]  @ \n\t"                   \
                "vmla.f32    d29, d14, d1[1]  @ \n\t"                   \
                "vmla.f32    d30, d15, d1[1]  @ \n\t"                   \
                "vmla.f32    d31, d16, d1[1]  @ \n\t"                   \
                "cmp         %2, #8           @ \n\t"                   \
                "blt         2f               @ \n\t"                   \
                "vmla.f32    d24, d11, d3[1]  @ \n\t"                   \
                "vmla.f32    d25, d12, d3[1]  @ \n\t"                   \
                "vmla.f32    d26, d13, d3[1]  @ \n\t"                   \
                "vmla.f32    d27, d14, d3[1]  @ \n\t"                   \
                "vmla.f32    d28, d15, d3[1]  @ \n\t"                   \
                "vmla.f32    d29, d16, d3[1]  @ \n\t"                   \
                "vmla.f32    d30, d17, d3[1]  @ \n\t"                   \
                "vmla.f32    d31, d18, d3[1]  @ \n\t"                   \
                "cmp         %2, #12          @ \n\t"                   \
                "blt         2f               @ \n\t"                   \
                "vmla.f32    d24, d13, d5[1]  @ \n\t"                   \
                "vmla.f32    d25, d14, d5[1]  @ \n\t"                   \
                "vmla.f32    d26, d15, d5[1]  @ \n\t"                   \
                "vmla.f32    d27, d16, d5[1]  @ \n\t"                   \
                "vmla.f32    d28, d17, d5[1]  @ \n\t"                   \
                "vmla.f32    d29, d18, d5[1]  @ \n\t"                   \
                "vmla.f32    d30, d19, d5[1]  @ \n\t"                   \
                "vmla.f32    d31, d20, d5[1]  @ \n\t"                   \
                "cmp         %2, #16          @ \n\t"                   \
                "blt         2f               @ \n\t"                   \
                "vmla.f32    d24, d15, d7[1]  @ \n\t"                   \
                "vmla.f32    d25, d16, d7[1]  @ \n\t"                   \
                "vmla.f32    d26, d17, d7[1]  @ \n\t"                   \
                "vmla.f32    d27, d18, d7[1]  @ \n\t"                   \
                "vmla.f32    d28, d19, d7[1]  @ \n\t"                   \
                "vmla.f32    d29, d20, d7[1]  @ \n\t"                   \
                "vmla.f32    d30, d21, d7[1]  @ \n\t"                   \
                "vmla.f32    d31, d22, d7[1]  @ \n\t"                   \
                "2:                           @ Kernel done \n\t"       \
                "subs        r6, r6, #1       @ \n\t"                   \
                "beq         3f               @ No more packet of 16 \n\t" \
                "subs        r4, r4, #1       @ Number of Loads in Vector X \n\t" \
                "bne         7f               @ No more packet of 16 in X \n\t" \
                "ldr         r1, [%0, #20]    @ Use tmp buffer for Xptr\n\t" \
                "mov         r4, #2           @ Add two more load from tmp buffer \n\t" \
                "7:                           @ \n\t"                   \
                "vld1.32     {d8-d11}, [r1]!  @ Load X packets \n\t"    \
                "vld1.32     {d12-d15}, [r1]! @ Load X packets \n\t"    \
                "vld1.32     {d16-d19}, [r1]! @ \n\t"                   \
                "vld1.32     {d20-d23}, [r1]! @ \n\t"                   \
                "subs        r1, r1, #64      @ Keep the X pointer ready for the second packet \n\t" \
                "vst1.32     {d24-d27}, [r2]! @ \n\t"                   \
                "vld1.32     {d24-d27}, [r0]! @ load Y \n\t"            \
                "vst1.32     {d28-d31}, [r2]! @ \n\t"                   \
                "vld1.32     {d28-d31}, [r0]! @ \n\t"                   \
                "b           1b               @ \n\t"                   \
                "3:                           @ \n\t"                   \
                "vst1.32     {d24-d27}, [r2]! @ Store previous elements \n\t" \
                "vst1.32     {d28-d31}, [r2]! @ \n\t"                   \
                "4:                           @ \n\t"                   \
                "cmp         r3, #1           @ \n\t"                   \
                "bne         5f               @ Go to end \n\t"         \
                "ands        r6, %1, #15      @ Is remaing element \n\t" \
                "beq         5f               @ \n\t"                   \
                "ldr         r0, [%0, #16]    @ Use tmp buffer for Yptr\n\t" \
                "mov         r3, #0           @ Set flag \n\t"          \
                "mov         r2, r0           @ \n\t"                   \
                "subs        r4, r4, #1       @ Number of Loads in Vector X \n\t" \
                "bne         8f               @ No more packet of 16 in X \n\t" \
                "ldr         r1, [%0, #20]    @ Use tmp buffer for Xptr\n\t" \
                "mov         r4, #2           @ Add two more load from tmp buffer \n\t" \
                "8:                           @ \n\t"                   \
                "vld1.32     {d8-d11}, [r1]!  @ Load X packets \n\t"    \
                "vld1.32     {d12-d15}, [r1]! @ Load X packets \n\t"    \
                "vld1.32     {d16-d19}, [r1]! @ Load a packet of 16\n\t" \
                "vld1.32     {d20-d23}, [r1]! @ \n\t"                   \
                "vld1.32     {d24-d27}, [r0]! @ \n\t"                   \
                "vld1.32     {d28-d31}, [r0]! @ \n\t"                   \
                "mov         r6, #1           @ Only for one trip \n\t" \
                "b           1b               @ \n\t"                   \
                "5:                           @ End"                    \
                "@pop         {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12} @ \n\t" \
                :                                                       \
                :"r" (argptr), "r"(n),"r"(csize)                        \
                : "cc", "r0", "r1", "r2", "r4", "r6", "r3", "memory",   \
                  "q0", "q1", "q4", "q5", "q6", "q7",                   \
                  "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", \
                  "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11",     \
                  "d12", "d13", "d14", "d15","d16", "d17", "d18", "d19", \
                  "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27","d28", "d29", "d30", "d31" \
                );                                                      \
        }                                                               \
        if(n & 15){                                                     \
            memcpy(&y[n & 0xfffffff0], ytmp, sizeof(float) * (n & 15)); \
        }                                                               \
    }


static inline void THDoubleVector_fill(double *x, const double c, const long n) {
  long i = 0;

  for(; i < n-4; i += 4)
  {
    x[i] = c;
    x[i+1] = c;
    x[i+2] = c;
    x[i+3] = c;
  }

  for(; i < n; i++)
    x[i] = c;
}

static inline void THDoubleVector_add(double *y, const double *x, const double c, const long n)
{
  long i = 0;

  for(;i < n-4; i += 4)
  {
    y[i] += c * x[i];
    y[i+1] += c * x[i+1];
    y[i+2] += c * x[i+2];
    y[i+3] += c * x[i+3];
  }

  for(; i < n; i++)
    y[i] += c * x[i];
}

static inline void THDoubleVector_diff(double *z, const double *x, const double *y, const long n)
{
  long i = 0;

  for(; i < n-4; i += 4)
  {
    z[i] = x[i] - y[i];
    z[i+1] = x[i+1] - y[i+1];
    z[i+2] = x[i+2] - y[i+2];
    z[i+3] = x[i+3] - y[i+3];
  }

  for(; i < n; i++)
    z[i] = x[i] - y[i];
}

static inline void THDoubleVector_scale(double *y, const double c, const long n)
{
  long i = 0;

  for(; i < n-4; i +=4)
  {
    y[i] *= c;
    y[i+1] *= c;
    y[i+2] *= c;
    y[i+3] *= c;
  }

  for(; i < n; i++)
    y[i] *= c;
}

static inline void THDoubleVector_mul(double *y, const double *x, const long n)
{
  long i = 0;

  for(; i < n-4; i += 4)
  {
    y[i] *= x[i];
    y[i+1] *= x[i+1];
    y[i+2] *= x[i+2];
    y[i+3] *= x[i+3];
  }

  for(; i < n; i++)
    y[i] *= x[i];
}

static inline void THDoubleVector_conv1d(double *y, double *x, double *c, double a, const long n, const long cn, unsigned char reverse){
    long i;
    if (reverse==0)
        for(i = 0; i < cn; i++)
            THDoubleVector_add(y, x + i, c[i]*a, n);
    else
        for(i = 0; i < cn; i++)
            THDoubleVector_add(y, x + i, c[-i]*a, n);
}

#else

/* If SSE2 not defined, then generate plain C operators */
#include "generic/THVector.c"
#include "THGenerateFloatTypes.h"

#endif

/* For non-float types, generate plain C operators */
#include "generic/THVector.c"
#include "THGenerateIntTypes.h"

#endif
