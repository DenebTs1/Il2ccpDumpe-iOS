//
//  Versions.h
//  Il2ccpDumper
//
//  Created by ts1 on 26/05/2026.
//


#pragma once

#if !defined(IL2CPP_VER_16)   && !defined(IL2CPP_VER_19)   && !defined(IL2CPP_VER_20)   \
 && !defined(IL2CPP_VER_21)   && !defined(IL2CPP_VER_22)   && !defined(IL2CPP_VER_23)   \
 && !defined(IL2CPP_VER_24)   && !defined(IL2CPP_VER_24_1) && !defined(IL2CPP_VER_24_2) \
 && !defined(IL2CPP_VER_24_3) && !defined(IL2CPP_VER_24_4) && !defined(IL2CPP_VER_24_5) \
 && !defined(IL2CPP_VER_27)   && !defined(IL2CPP_VER_27_1) && !defined(IL2CPP_VER_27_2) \
 && !defined(IL2CPP_VER_29)   && !defined(IL2CPP_VER_29_1) && !defined(IL2CPP_VER_31)
    #define IL2CPP_VER_31
#endif

#if defined(IL2CPP_VER_31)
    #define IL2CPP_TARGET_VERSION 31.0
#elif defined(IL2CPP_VER_29_1)
    #define IL2CPP_TARGET_VERSION 29.1
#elif defined(IL2CPP_VER_29)
    #define IL2CPP_TARGET_VERSION 29.0
#elif defined(IL2CPP_VER_27_2)
    #define IL2CPP_TARGET_VERSION 27.2
#elif defined(IL2CPP_VER_27_1)
    #define IL2CPP_TARGET_VERSION 27.1
#elif defined(IL2CPP_VER_27)
    #define IL2CPP_TARGET_VERSION 27.0
#elif defined(IL2CPP_VER_24_5)
    #define IL2CPP_TARGET_VERSION 24.5
#elif defined(IL2CPP_VER_24_4)
    #define IL2CPP_TARGET_VERSION 24.4
#elif defined(IL2CPP_VER_24_3)
    #define IL2CPP_TARGET_VERSION 24.3
#elif defined(IL2CPP_VER_24_2)
    #define IL2CPP_TARGET_VERSION 24.2
#elif defined(IL2CPP_VER_24_1)
    #define IL2CPP_TARGET_VERSION 24.1
#elif defined(IL2CPP_VER_24)
    #define IL2CPP_TARGET_VERSION 24.0
#elif defined(IL2CPP_VER_23)
    #define IL2CPP_TARGET_VERSION 23.0
#elif defined(IL2CPP_VER_22)
    #define IL2CPP_TARGET_VERSION 22.0
#elif defined(IL2CPP_VER_21)
    #define IL2CPP_TARGET_VERSION 21.0
#elif defined(IL2CPP_VER_20)
    #define IL2CPP_TARGET_VERSION 20.0
#elif defined(IL2CPP_VER_19)
    #define IL2CPP_TARGET_VERSION 19.0
#elif defined(IL2CPP_VER_16)
    #define IL2CPP_TARGET_VERSION 16.0
#endif

#if defined(IL2CPP_VER_31)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
    #define IL2CPP_VER_GE_23
    #define IL2CPP_VER_GE_24
    #define IL2CPP_VER_GE_24_1
    #define IL2CPP_VER_GE_24_2
    #define IL2CPP_VER_GE_27
    #define IL2CPP_VER_GE_29
    #define IL2CPP_VER_GE_31
#elif defined(IL2CPP_VER_29_1) || defined(IL2CPP_VER_29)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
    #define IL2CPP_VER_GE_23
    #define IL2CPP_VER_GE_24
    #define IL2CPP_VER_GE_24_1
    #define IL2CPP_VER_GE_24_2
    #define IL2CPP_VER_GE_27
    #define IL2CPP_VER_GE_29
#elif defined(IL2CPP_VER_27_2) || defined(IL2CPP_VER_27_1) || defined(IL2CPP_VER_27)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
    #define IL2CPP_VER_GE_23
    #define IL2CPP_VER_GE_24
    #define IL2CPP_VER_GE_24_1
    #define IL2CPP_VER_GE_24_2
    #define IL2CPP_VER_GE_27
#elif defined(IL2CPP_VER_24_5) || defined(IL2CPP_VER_24_4) \
   || defined(IL2CPP_VER_24_3) || defined(IL2CPP_VER_24_2)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
    #define IL2CPP_VER_GE_23
    #define IL2CPP_VER_GE_24
    #define IL2CPP_VER_GE_24_1
    #define IL2CPP_VER_GE_24_2
#elif defined(IL2CPP_VER_24_1)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
    #define IL2CPP_VER_GE_23
    #define IL2CPP_VER_GE_24
    #define IL2CPP_VER_GE_24_1
#elif defined(IL2CPP_VER_24)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
    #define IL2CPP_VER_GE_23
    #define IL2CPP_VER_GE_24
#elif defined(IL2CPP_VER_23)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
    #define IL2CPP_VER_GE_23
#elif defined(IL2CPP_VER_22)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
    #define IL2CPP_VER_GE_22
#elif defined(IL2CPP_VER_21)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
    #define IL2CPP_VER_GE_21
#elif defined(IL2CPP_VER_20)
    #define IL2CPP_VER_GE_19
    #define IL2CPP_VER_GE_20
#elif defined(IL2CPP_VER_19)
    #define IL2CPP_VER_GE_19
#endif

// "Version at most X" helpers for fields that DISAPPEARED at a release.
#if !defined(IL2CPP_VER_GE_24_1)
    #define IL2CPP_VER_LE_24
#endif
#if !defined(IL2CPP_VER_GE_24_2)
    #define IL2CPP_VER_LE_24_1
#endif
#if !defined(IL2CPP_VER_GE_27)
    #define IL2CPP_VER_LE_24_5
#endif
#if !defined(IL2CPP_VER_GE_29)
    #define IL2CPP_VER_LE_27_2
#endif
#if !defined(IL2CPP_VER_GE_23)
    #define IL2CPP_VER_LE_22
#endif
#if defined(IL2CPP_VER_21) || defined(IL2CPP_VER_22)
    #define IL2CPP_VER_IN_21_22
#endif

// Version 24.3 cutoff (SAssemblyNameDefinition.hashValueIndex).
#if defined(IL2CPP_VER_24_3) || defined(IL2CPP_VER_24_2) || defined(IL2CPP_VER_24_1) \
 || defined(IL2CPP_VER_24)   || defined(IL2CPP_VER_23)   || defined(IL2CPP_VER_22)   \
 || defined(IL2CPP_VER_21)   || defined(IL2CPP_VER_20)   || defined(IL2CPP_VER_19)   \
 || defined(IL2CPP_VER_16)
    #define IL2CPP_VER_LE_24_3
#endif

#define IL2CPP_VER_GE(v, t)      ((v) >= (t))
#define IL2CPP_VER_GT(v, t)      ((v) >  (t))
#define IL2CPP_VER_LE(v, t)      ((v) <= (t))
#define IL2CPP_VER_LT(v, t)      ((v) <  (t))
#define IL2CPP_VER_EQ(v, t)      ((v) == (t))
#define IL2CPP_VER_IN(v, mn, mx) ((v) >= (mn) && (v) <= (mx))
