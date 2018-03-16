#pragma once

#include "HUtilities.h"

template <int aNDims, class ACoord> class HPoint 
{

public:

    typedef HPoint<aNDims, ACoord> Self;

    typedef ACoord Coord;

    enum { kNDims = aNDims }; // store dimensionality

    //! Accesses the \a i'th coordinate.
    inline Coord & operator[] ( int i ) { return fParams[i]; }

    //! Accesses the \a i'th coordinate.
    inline const Coord & operator[] ( int i ) const { return fParams[i]; }

    //! Accesses the \a i'th coordinate.
    inline Coord & operator() ( int i ) { return fParams[i]; }

    //! Accesses the \a i'th coordinate.
    inline const Coord & operator() ( int i ) const { return fParams[i]; }

        //! Returns the coordinate-wise sum of two points.
    inline Self operator+ ( const Self &p ) const {
	    Self r;
	    for (int i = 0; i < kNDims; i++)
	        r[i] = fParams[i] + p[i];
	    return r;
    }

    //! Returns the coordinate-wise difference of two points.
    inline Self operator- ( const Self &p ) const {
	    Self r;
	    for (int i = 0; i < kNDims; i++)
	        r[i] = fParams[i] - p[i];
	    return r;
    }

    //! Returns the point multiplied by a point.
    inline Self operator* ( const Self &p ) const 
	{
	    Self r;
	    for (int i = 0; i < kNDims; i++)
		{
	        r[i] = fParams[i] * p[i];
		}
	    return r;
    }

    //! Returns the point multiplied by a scalar.
    inline Self operator* ( Coord s ) const {
	    Self r;
	    for (int i = 0; i < kNDims; i++)
	        r[i] = fParams[i] * s;
	    return r;
    }

    //! Returns the point divided by a scalar.
    inline Self operator/ ( Coord s ) const {
	    Self r;
	    for (int i = 0; i < kNDims; i++)
	        r[i] = fParams[i] / s;
	    return r;
    }

    //! assignment operator.
    inline Self& operator= ( Coord* s ) {
	    for (int i = 0; i < kNDims; i++)
	         fParams[i] = s[i];
	    return *this;
    }

    //! Returns the negation of the point.
    inline Self operator- () const {
	    Self r;
	    for (int i = 0; i < kNDims; i++)
	        r[i] = - fParams[i];
	    return r;
    }

    //! Returns true if points are equal, else false.
	inline bool operator== (const Self &src) const { 
		for (int i = 0; i < kNDims; i++) {
			if(src[i] != fParams[i]) {
				return false;
			}
		}
		return true;
	}

	  //! Returns false if points are equal, else true.
	inline bool operator!= (const Self &src) const { 
		return !(*this == src);
	}

    inline float Dot(const Self &rhs) const {
	float v = 0.0f;
	for(int i = 0; i < kNDims; i++)
	    v += fParams[i] * rhs[i];
	return v;
    }

    float L2NormSquared() const { return (*this).Dot(*this); }

    float L2Norm(void) const { return sqrtf(L2NormSquared()); }

protected:

    Coord fParams[kNDims];
};

class  HPoint2D : public HPoint<2, float> {
public:
    typedef HPoint<2, float> Super;
    typedef HPoint2D Self;
    HPoint2D() {}
    HPoint2D(const Super &s) { fParams[0] = s[0]; fParams[1] = s[1]; }
    HPoint2D(float x, float y) { fParams[0] = x; fParams[1] = y; }
    void Set(float x, float y) { fParams[0] = x; fParams[1] = y; }
    inline const float &X() const { return fParams[0]; }
    inline float & X() { return fParams[0]; }
    inline const float &Y() const { return fParams[1]; }
    inline float & Y() { return fParams[1]; }
    inline const float& operator[](int i) const { return fParams[i]; }
    inline float& operator[](int i) { return fParams[i]; }
};

class  HPoint2Di : public HPoint<2, int> {
public:
    typedef HPoint<2, int> Super;
    typedef HPoint2Di Self;
    HPoint2Di() {}
    HPoint2Di(const Super &s) { fParams[0] = s[0]; fParams[1] = s[1]; }
    HPoint2Di(int x, int y) { fParams[0] = x; fParams[1] = y; }
    void Set(int x, int y) { fParams[0] = x; fParams[1] = y; }
    inline const int &X() const { return fParams[0]; }
    inline int & X() { return fParams[0]; }
    inline const int &Y() const { return fParams[1]; }
    inline int & Y() { return fParams[1]; }
    inline const int& operator[](int i) const { return fParams[i]; }
    inline int& operator[](int i) { return fParams[i]; }

    //! assignment.
    inline Self& operator= (Self s) {
	    for (int i = 0; i < kNDims; i++)
	        (*this)[i] = s[i];
	    return *this;
    }

    //! assignment operator.
    inline Self& operator= ( long* s ) {
	    for (int i = 0; i < kNDims; i++)
	         fParams[i] = s[i];
	    return *this;
    }
};

class  HPoint3D : public HPoint<3, float> {
public:
    typedef HPoint<3, float> Super;
    typedef HPoint3D Self;
    HPoint3D() {}
    HPoint3D(const Super& point) : Super(point) {}
    HPoint3D(float x, float y, float z) { 
	    X() = x; 
	    Y() = y; 
	    Z() = z; 
    }

    HPoint3D(const HPoint2D &src) {
	    X() = src.X();
	    Y() = src.Y();
	    Z() = 1.0f;
    }

    void Set(float x, float y, float z) { 
	    fParams[0] = x;
	    fParams[1] = y;
	    fParams[2] = z;
    }

    inline const float &X() const { return fParams[0]; }
    inline float & X() { return fParams[0]; }
    inline const float &Y() const { return fParams[1]; }
    inline float & Y() { return fParams[1]; }
    inline const float &Z() const { return fParams[2]; }
    inline float & Z() { return fParams[2]; }
    inline const float& operator[](int i) const { return fParams[i]; }
    inline float& operator[](int i) { return fParams[i]; }

    inline void Cross(const Self &p, Self &dest ) const {
	    dest.X() = Y() * p.Z() - Z() * p.Y();
	    dest.Y() = Z() * p.X() - X() * p.Z();
	    dest.Z() = X() * p.Y() - Y() * p.X();
    }

    inline Self Cross(const Self &p) const {
	    Self dest;
	    Cross(p, dest);
	    return dest;
    }
};

class  HPoint3Di : public HPoint<3, int> {
public:
    typedef HPoint<3, int> Super;
    typedef HPoint3Di Self;
    HPoint3Di() {}
    HPoint3Di(const Super &s) { fParams[0] = s[0]; fParams[1] = s[1]; fParams[2] = s[2]; }
    HPoint3Di(int x, int y) { fParams[0] = x; fParams[1] = y; }
    void Set(int x, int y, int z) { fParams[0] = x; fParams[1] = y; fParams[2] = z; }
    inline const int &X() const { return fParams[0]; }
    inline int & X() { return fParams[0]; }
    inline const int &Y() const { return fParams[1]; }
    inline int & Y() { return fParams[1]; }
    inline const int &Z() const { return fParams[2]; }
    inline int & Z() { return fParams[2]; }
    inline const int& operator[](int i) const { return fParams[i]; }
    inline int& operator[](int i) { return fParams[i]; }
};

class  HPoint3Dd : public HPoint<3, double> {
public:
	typedef HPoint<3,double> Super;
	typedef HPoint3Dd Self;

	HPoint3Dd() {}
	HPoint3Dd(const Super &s) { fParams[0] = s[0]; fParams[1] = s[1]; fParams[2] = s[2]; }
	HPoint3Dd(double x, double y, double z) { fParams[0] = x; fParams[1] = y; fParams[2] = z; }
	HPoint3Dd(HPoint3D Point) { fParams[0] = (double)Point[0]; fParams[1] = (double)Point[1]; fParams[2] = (double)Point[2]; }

	void Set(double x, double y, double z) { fParams[0] = x; fParams[1] = y; fParams[2] = z; }
	inline const double &X() const { return fParams[0]; }
	inline double & X() { return fParams[0]; }
	inline const double &Y() const { return fParams[1]; }
	inline double & Y() { return fParams[1]; }
	inline const double &Z() const { return fParams[2]; }
	inline double & Z() { return fParams[2]; }

	inline const double& operator[](int i) const { return fParams[i]; }
	inline double& operator[](int i) { return fParams[i]; }
	inline const double& operator()(int i) const { return fParams[i]; }
	inline double& operator()(int i) { return fParams[i]; }

	inline double Dot(const Self &rhs) const {
		double v = 0.0f;
		for(int i = 0; i < kNDims; i++)
			v += fParams[i] * rhs[i];
		return v;
	}

	inline Self operator- ( const Self &p ) const {
		Self r;
		for (int i = 0; i < kNDims; i++)
			r[i] = fParams[i] - p[i];
		return r;
	}

	inline Self operator+ ( const Self &p ) const {
		Self r;
		for (int i = 0; i < kNDims; i++)
			r[i] = fParams[i]+p[i];
		return r;
	}

	inline Self operator* ( const double M ) const {
		Self r;
		for (int i = 0; i < kNDims; i++)
			r[i] = fParams[i]*M;
		return r;
	}


};

class  HPoint4D : public HPoint<4, float> {
  public:
      typedef HPoint<4, float> Super;
      typedef HPoint4D Self;
    HPoint4D() {}
    HPoint4D(const Super& point) : Super(point) {}
    HPoint4D(float x, float y, float z) { 
	    fParams[0] = x; 
	    fParams[1] = y; 
	    fParams[2] = z; 
	    fParams[3] = 1.0;
    }
    HPoint4D(float x, float y, float z, float w) { 
	    fParams[0] = x; 
	    fParams[1] = y; 
	    fParams[2] = z; 
	    fParams[3] = w;
    }

    void Set(float x, float y, float z, float w) { 
	    fParams[0] = x;
	    fParams[1] = y;
	    fParams[2] = z;
	    fParams[3] = w;
    }

    inline const float &X() const { return fParams[0]; }
    inline float & X() { return fParams[0]; }
    inline const float &Y() const { return fParams[1]; }
    inline float & Y() { return fParams[1]; }
    inline const float &Z() const { return fParams[2]; }
    inline float & Z() { return fParams[2]; }
    inline const float &W() const { return fParams[3]; }
    inline float & W() { return fParams[3]; }
    inline const float& operator[](int i) const { return fParams[i]; }
    inline float& operator[](int i) { return fParams[i]; }

    operator HPoint3D() const {
	return HPoint3D(X()/W(), Y()/W(), Z()/W());
    }
};

class  HPoint4Dd : public HPoint<4, double> {
public:
	typedef HPoint<4, double> Super;
	typedef HPoint4Dd Self;
	HPoint4Dd() {}
	HPoint4Dd(const Super& point) : Super(point) {}
	HPoint4Dd(double x, double y, double z) { 
		fParams[0] = x; 
		fParams[1] = y; 
		fParams[2] = z; 
		fParams[3] = 1.0;
	}
	HPoint4Dd(double x, double y, double z, double w) { 
		fParams[0] = x; 
		fParams[1] = y; 
		fParams[2] = z; 
		fParams[3] = w;
	}

	void Set(double x, double y, double z, double w) { 
		fParams[0] = x;
		fParams[1] = y;
		fParams[2] = z;
		fParams[3] = w;
	}

	inline const double &X() const { return fParams[0]; }
	inline double & X() { return fParams[0]; }
	inline const double &Y() const { return fParams[1]; }
	inline double & Y() { return fParams[1]; }
	inline const double &Z() const { return fParams[2]; }
	inline double & Z() { return fParams[2]; }
	inline const double &W() const { return fParams[3]; }
	inline double & W() { return fParams[3]; }
	inline const double& operator[](int i) const { return fParams[i]; }
	inline double& operator[](int i) { return fParams[i]; }
	inline const double& operator()(int i) const { return fParams[i]; }
	inline double& operator()(int i) { return fParams[i]; }
	operator HPoint3Dd() const {
		return HPoint3Dd(X()/W(), Y()/W(), Z()/W());
	}
};
