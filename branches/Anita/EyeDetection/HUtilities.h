#pragma once

#include "math.h" // for  double exp(double);
#include <cstddef>

#ifdef M_PI
#undef M_PI
#define M_PI	      3.1415926536f
#endif

template <class NUMBER> inline NUMBER power(NUMBER x, int y)
{
  NUMBER z = (NUMBER) 1;
  for(int i=0; i<y; i++) z *= x;
  return z;
}

template <class NUMBER> inline NUMBER clip(NUMBER x, NUMBER l, NUMBER u)
{
  return ((x < l) ? l : ((x > u) ? u : x));
}

template <class Type> inline Type HMin(const Type &x, const Type &y)
{
    return (x < y) ? x : y;
}

template <class Type> inline Type HMax(const Type &x, const Type &y)
{
    return (x > y) ? x : y;
}

template <class Type> inline Type HClip(const Type &x, const Type &min, const Type &max)
{
    return HMin(HMax(x, min), max);
}

template <class T>
inline T HMin(const T val[], size_t n)
{
    T min = val[0];
    for(size_t i = 1; i < n; i++)
        if(val[i] < min)
            min = val[i];
    return min;
}

template <class T>
inline T HMax(const T val[], size_t n)
{
    T max = val[0];
    for(size_t i = 1; i < n; i++)
        if(val[i] > max)
            max = val[i];
    return max;
}


template <class Type> inline Type HAbs(const Type &x)
{
    return (x < 0) ? -x : x;
}

template <class AType> inline AType HSqr(const AType &x)
{
    return x * x;
}

template <class X> inline void swap(X &x1, X &x2)
{
  X t = x1; x1 = x2; x2 = t;
}

template <class Type> inline int HSgn(const Type &x)
{
    int tmp = (x>0)<<1;
    return (tmp-1);
}

inline float HGauss(float x, float sigma)
{
	return float(exp(-HSqr(x/sigma)/2.f));
}

inline double sqr(double x) { return x * x; }

inline float sqrf(float x) { return x * x; }

/*
#if BigEndian_
	#define iexp_				0
	#define iman_				1
#else
	#define iexp_				1
	#define iman_				0
#endif //BigEndian_

// This is a fast floor() function
const double _double2fixmagic = 68719476736.0*1.5;     //2^36 * 1.5,  (52-_shiftamt=36) uses limited precisicion to floor
const int _shiftamt       = 16;                    //16.16 fixed point representation,
inline int Real2Int(double val)
{
	val		= val + _double2fixmagic;
	return ((int*)&val)[iman_] >> _shiftamt;
}

inline int Real2Int(float val) { return Real2Int ((double)val); }

// This is a fast round() function
inline int Round(double d0) // REAL => int  (faster than a (cast)).
{
   static double intScale = 6755399441055744.0;
   static double d1;
   static int *pInt = (int*) &d1;
   d1 = d0 + intScale;
   return *pInt;
}

inline int Round(float d0) { return Round((double)d0); }

// 8.8 fixed point format
inline float Float(short val) { return (float(val)/256.0f); }
inline short Fixed(float val) { return short(Real2Int(val*256)); }
inline short Floor(short val) { return (short)(val >> 8); }
inline short Rem(short val) { return (short)(val & 0x00ff); }

// 24.8 fixed point format
inline float Float32(int val) { return (float(val)/256.0f); }
inline int Fixed32(float val) { return int(Real2Int(val*256)); }
inline int Floor32(int val) { return (val >> 8); }
inline int Rem32(int val) { return (val & 0x000000ff); }

*/
