#if !defined(HANDMADE_INTRINSICS_H)

// TODO: Conver all of these to platform-efficient versions and remove math.h

#include "math.h"

inline int32 RoundReal32ToInt32(float Real32)
{
    int32 Result = (int32)(Real32 + 0.5f);
    return(Result);
}

inline uint32 RoundReal32ToUInt32(float Real32)
{
    uint32 Result = (uint32)(Real32 + 0.5f);
    return(Result);
}

inline int32 FloorReal32ToInt32(float Real32)
{
    int32 Result = (int32)floorf(Real32);
    return(Result);
}

inline int32 TruncateReal32ToInt32(float Real32)
{
    int32 Result = (int32)Real32;
    return(Result);
}

inline float Sin(float Angle)
{
    float Result = sinf(Angle);
    return(Result);
}

inline float Cos(float Angle)
{
    float Result = cosf(Angle);
    return(Result);
}

inline float ATan2(float Y, float X)
{
    float Result = atan2f(Y, X);
    return(Result);
}

#define HANDMADE_INTRINSICS_H
#endif