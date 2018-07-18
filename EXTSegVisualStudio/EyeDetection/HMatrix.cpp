#include "HMatrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>


// CMatrix

#define ERR_MEMORY "No memory left"
#define EXCEPTION(arg) exit(1)


HMatrix::HMatrix(void) :
  fRows(0),
  fColumns(0),
  fElement(NULL)
{
}

HMatrix::HMatrix(int Rows) :
  fRows(Rows),
  fColumns(1),
  fElement(new float[Rows])
{
  if (fElement == NULL)
    EXCEPTION(ERR_MEMORY);
}


HMatrix::HMatrix(int Rows, int Columns) :
  fRows(Rows),
  fColumns(Columns),
  fElement(new float[Rows*Columns])
{
  if (fElement == NULL)
    EXCEPTION(ERR_MEMORY);
}

HMatrix::HMatrix(const HMatrix& Matrix) :
  fRows(Matrix.fRows),
  fColumns(Matrix.fColumns),
  fElement(new float[fRows*fColumns])
{
  if (fElement == NULL)
    EXCEPTION(ERR_MEMORY);
  memcpy(fElement, Matrix.fElement, sizeof(float)*fRows*fColumns);
}

HMatrix::~HMatrix(void)
{
  delete [] fElement;
}

HMatrix& HMatrix::operator=(const HMatrix& Matrix)
{
  if ((Matrix.fColumns != fColumns) || (Matrix.fRows != fRows))
    EXCEPTION("Incompatible matrices for operator=");

  memcpy(fElement, Matrix.fElement, sizeof(float)*fRows*fColumns);
  return *this;
}


HMatrix HMatrix::operator*(const float scalar) const
{
	HMatrix ret(*this);
	for (int r = 0; r < fRows; r++)
	{
		for (int c = 0; c < fColumns; c++)
		{
			ret(r,c) *= scalar;
		}
	}
	return ret;
}

HMatrix HMatrix::operator+(const float scalar) const
{
	HMatrix ret(*this);
	for (int r = 0; r < fRows; r++)
	{
		for (int c = 0; c < fColumns; c++)
		{
			ret(r,c) += scalar;
		}
	}
	return ret;
}

HMatrix HMatrix::operator-(const float scalar) const
{
	HMatrix ret(*this);
	for (int r = 0; r < fRows; r++)
	{
		for (int c = 0; c < fColumns; c++)
		{
			ret(r,c) -= scalar;
		}
	}
	return ret;
}


HMatrix HMatrix::operator*(const HMatrix& Matrix) const
{
  int r, c, x;
  if ((Matrix.fRows != fColumns))
    EXCEPTION("Incompatible matrices for operator*");

  HMatrix ret(fRows,Matrix.fColumns);
  ret.SetAll(0);
  for (r=0; r<fRows; r++) {
    for (c=0; c<Matrix.fColumns; c++) {
      for (x=0; x<fColumns; x++) {
        ret(r,c) += (*this)(r,x)*Matrix(x,c);
      }
    }
  }

  return ret;
}

HMatrix HMatrix::operator+(const HMatrix& Matrix) const
{
	int r,c;

	if (Matrix.fRows != fRows && Matrix.fColumns != fColumns)
		EXCEPTION("Incompatible matrices for operator +=");

	HMatrix ret(fRows,fColumns);

	for (r = 0; r < fRows; r++)
	{
		for (c = 0; c < fColumns; c++)
		{
			ret.fElement[r*fColumns+c] = fElement[r*fColumns+c] + Matrix.fElement[r*fColumns+c];
		}
	}

	return ret;
}

HMatrix HMatrix::operator-(const HMatrix& Matrix) const
{
	int r,c;

	if (Matrix.fRows != fRows && Matrix.fColumns != fColumns)
		EXCEPTION("Incompatible matrices for operator +=");

	HMatrix ret(fRows,fColumns);

	for (r = 0; r < fRows; r++)
	{
		for (c = 0; c < fColumns; c++)
		{
			ret.fElement[r*fColumns+c] = fElement[r*fColumns+c] - Matrix.fElement[r*fColumns+c];
		}
	}

	return ret;
}


HMatrix& HMatrix::operator+=(const HMatrix& Matrix)
{
    int r,c;

    if (Matrix.fRows != fRows && Matrix.fColumns != fColumns)
	EXCEPTION("Incompatible matrices for operator +=");

    for (r = 0; r < fRows; r++)
    {
	for (c = 0; c < fColumns; c++)
	{
	    fElement[r*fColumns+c] += Matrix.fElement[r*fColumns+c];
	}
    }

    return *this;
}

HMatrix& HMatrix::operator-=(const HMatrix& Matrix)
{
    int r,c;

    if (Matrix.fRows != fRows && Matrix.fColumns != fColumns)
	EXCEPTION("Incompatible matrices for operator +=");

    for (r = 0; r < fRows; r++)
    {
	for (c = 0; c < fColumns; c++)
	{
	    fElement[r*fColumns+c] -= Matrix.fElement[r*fColumns+c];
	}
    }

    return *this;
}

HMatrix& HMatrix::operator*=(const float scalar)
{
    int r,c;

    for (r = 0; r < fRows; r++)
    {
	for (c = 0; c < fColumns; c++)
	{
	    fElement[r*fColumns+c] *= scalar;
	}
    }

    return *this;
}

HMatrix& HMatrix::operator/=(const float scalar)
{
    int r,c;

    for (r = 0; r < fRows; r++)
    {
	for (c = 0; c < fColumns; c++)
	{
	    fElement[r*fColumns+c] /= scalar;
	}
    }
    return *this;
}

void HMatrix::Multiply(const HMatrix &Right, HMatrix& Result) const
{

    int r,c,x;

    if (fColumns != Right.fRows)
	EXCEPTION("Incompatible matrices for Multiply method");

    Result.SetAll(0.0f);

    for (r = 0; r < fRows; r++)
    {
	for (c = 0; c < Right.fColumns; c++)
	{
	    for (x = 0; x < fColumns; x++)
	    {
		Result(r,c) += fElement[r*fColumns+x]*Right(x,c);
	    }
	}
    }
}

void HMatrix::Alloc(int Rows, int Columns)
{
  Free();

  fRows=Rows;
  fColumns=Columns;

  fElement = new float[fRows*fColumns];
  if (fElement == NULL)
    EXCEPTION(ERR_MEMORY);
}

void HMatrix::Free(void)
{
  delete [] fElement;
  fElement = NULL;
  fRows = 0;
  fColumns = 0;
}

HMatrix HMatrix::Transpose() const
{
  HMatrix ret(fColumns,fRows);
  int y,x;

  for (y=0; y<fRows; y++) {
    for (x=0; x<fColumns; x++) {
      ret(x,y) = fElement[y*fColumns+x];
    }
  }
  return ret;
}

void HMatrix::Print(void) const
{
  int x,y;
  printf("[");
  for (y=0; y<fRows; y++)
  {
    for (x=0; x<fColumns; x++)
	{
      printf(" %.4f",fElement[y*fColumns+x]);
    }
    if (y==fRows-1)
	{
      printf("]\n");
    }
	else
	{
      printf(";\n");
    }
  }
}


// HHomMatrix _________________________________________________________
// Matrices represented by float[4][4].

bool InvertMatrix3x3( const float (*src)[4], float (*dest)[4] )
{
    float det;

    det =  src[0][0] * (src[1][1] * src[2][2] - src[1][2] * src[2][1]);
    det += src[0][1] * (src[1][2] * src[2][0] - src[1][0] * src[2][2]);
    det += src[0][2] * (src[1][0] * src[2][1] - src[1][1] * src[2][0]);

    if (det == 0.0)
	return false;

    const float invdet = 1.0f / det;

    dest[0][0] = (src[1][1] * src[2][2] - src[1][2] * src[2][1]) * invdet;
    dest[1][0] = (src[1][2] * src[2][0] - src[1][0] * src[2][2]) * invdet;
    dest[2][0] = (src[1][0] * src[2][1] - src[1][1] * src[2][0]) * invdet;

    dest[0][1] = (src[0][2] * src[2][1] - src[0][1] * src[2][2]) * invdet;
    dest[1][1] = (src[0][0] * src[2][2] - src[0][2] * src[2][0]) * invdet;
    dest[2][1] = (src[0][1] * src[2][0] - src[0][0] * src[2][1]) * invdet;

    dest[0][2] = (src[0][1] * src[1][2] - src[0][2] * src[1][1]) * invdet;
    dest[1][2] = (src[0][2] * src[1][0] - src[0][0] * src[1][2]) * invdet;
    dest[2][2] = (src[0][0] * src[1][1] - src[0][1] * src[1][0]) * invdet;

    return true;
}

HHomMatrix::HHomMatrix(
      float xx, float xy, float xz, float xw,
      float yx, float yy, float yz, float yw,
      float zx, float zy, float zz, float zw,
      float wx, float wy, float wz, float ww)
{
  fParams[0][0] = xx;
  fParams[0][1] = xy;
  fParams[0][2] = xz;
  fParams[0][3] = xw;
  fParams[1][0] = yx;
  fParams[1][1] = yy;
  fParams[1][2] = yz;
  fParams[1][3] = yw;
  fParams[2][0] = zx;
  fParams[2][1] = zy;
  fParams[2][2] = zz;
  fParams[2][3] = zw;
  fParams[3][0] = wx;
  fParams[3][1] = wy;
  fParams[3][2] = wz;
  fParams[3][3] = ww;
}

HHomMatrix::HHomMatrix(const HHomMatrix& HomMatrix)
{
  memcpy(fParams, HomMatrix.fParams, sizeof(fParams));
}

void HHomMatrix::SetToZero(void)
{
  memset(fParams, 0, sizeof(fParams));
}

void HHomMatrix::Print(char *name, char *format) const
{
  printf("%s\n", name);
  for (int i = 0; i < 4; i++) {
	  for (int j = 0; j < 4; j++) {
		  printf(format, fParams[i][j]);
	  }
	  printf("\n");
  }
  printf("\n");
}

void HHomMatrix::SetToIdentity(void)
{
  SetToZero();
  fParams[0][0] = 1.0f;
  fParams[1][1] = 1.0f;
  fParams[2][2] = 1.0f;
  fParams[3][3] = 1.0f;
}

void HHomMatrix::SetRotation(const HPoint3D &rotation)
{
  // Rx = [ 1, 0, 0, 0, cos(psi), -sin(psi), 0, sin(psi), cos(psi) ]
  // Ry = [ cos(the), 0, sin(the), 0, 1, 0, -sin(the), 0, cos(the) ]
  // Rz = [ cos(phi), -sin(phi), 0, sin(phi), cos(phi), 0, 0, 0, 1 ]
  // R = Rx*Ry*Rz

  float cx = cosf(rotation.X()), sx = sinf(rotation.X()),
        cy = cosf(rotation.Y()), sy = sinf(rotation.Y()),
        cz = cosf(rotation.Z()), sz = sinf(rotation.Z());

  fParams[0][0] =  cy*cz;
  fParams[0][1] = -cy*sz;
  fParams[0][2] =  sy;
  fParams[1][0] =  cx*sz+sx*sy*cz;
  fParams[1][1] =  cx*cz-sx*sy*sz;
  fParams[1][2] = -sx*cy;
  fParams[2][0] =  sx*sz-cx*sy*cz;
  fParams[2][1] =  sx*cz+cx*sy*sz;
  fParams[2][2] =  cx*cy;
}

void HHomMatrix::SetRotationInv(const HPoint3D &rotation)
{
  // Rx = [ 1, 0, 0, 0, cos(psi), -sin(psi), 0, sin(psi), cos(psi) ]
  // Ry = [ cos(the), 0, sin(the), 0, 1, 0, -sin(the), 0, cos(the) ]
  // Rz = [ cos(phi), -sin(phi), 0, sin(phi), cos(phi), 0, 0, 0, 1 ]
  // inv(Rx(psi))=Rx(-psi)=Rx^T
  // inv(Rx(the))=Rx(-the)=Ry^T
  // inv(Rx(phi))=Rx(-phi)=Rz^T
  // R = Rx*Ry*Rz
  // inv(R) = inv(Rz)*inv(Ry)*inv(Rx) = Rz^T*Ry^T*Rx^T =
  //        = (Rx*Ry*Rz)^T

  float cx = cosf(rotation.X()), sx = sinf(rotation.X()),
        cy = cosf(rotation.Y()), sy = sinf(rotation.Y()),
        cz = cosf(rotation.Z()), sz = sinf(rotation.Z());

  fParams[0][0] =  cy*cz;
  fParams[1][0] = -cy*sz;
  fParams[2][0] =  sy;
  fParams[0][1] =  cx*sz+sx*sy*cz;
  fParams[1][1] =  cx*cz-sx*sy*sz;
  fParams[2][1] = -sx*cy;
  fParams[0][2] =  sx*sz-cx*sy*cz;
  fParams[1][2] =  sx*cz+cx*sy*sz;
  fParams[2][2] =  cx*cy;
}

void HHomMatrix::SetTranslation(const HPoint3D &location)
{
  fParams[0][3] = location.X();
  fParams[1][3] = location.Y();
  fParams[2][3] = location.Z();
}

void HHomMatrix::Set(const HPoint3D& translation, const HPoint3D &rotation)
{
  SetRotation(rotation);
  SetTranslation(translation);
  fParams[3][0] = 0.0f;
  fParams[3][1] = 0.0f;
  fParams[3][2] = 0.0f;
  fParams[3][3] = 1.0f;
}

void HHomMatrix::SetInv(const HPoint3D& translation, const HPoint3D &rotation)
{
  // M = (R t;0 1) ==> inv(M) = (inv(R) -inv(R)*t; 0 1)

  SetRotationInv(rotation);
  fParams[0][3] = -fParams[0][0]*translation.X()-fParams[0][1]*translation.Y()-fParams[0][2]*translation.Z();
  fParams[1][3] = -fParams[1][0]*translation.X()-fParams[1][1]*translation.Y()-fParams[1][2]*translation.Z();
  fParams[2][3] = -fParams[2][0]*translation.X()-fParams[2][1]*translation.Y()-fParams[2][2]*translation.Z();
  fParams[3][0] = 0.0f;
  fParams[3][1] = 0.0f;
  fParams[3][2] = 0.0f;
  fParams[3][3] = 1.0f;
}

void HHomMatrix::SetProjectionZ(float fx, float fy, float cx, float cy)
{
  // projection with center (0,0,0) to plane z=1

  fParams[0][0] =  fx;
  fParams[0][1] =  0.0f;
  fParams[0][2] =  cx;
  fParams[0][3] =  0.0;
  fParams[1][0] =  0.0;
  fParams[1][1] =  fy;
  fParams[1][2] =  cy;
  fParams[1][3] =  0.0;
  fParams[2][0] =  0.0;
  fParams[2][1] =  0.0;
  fParams[2][2] =  1.0;
  fParams[2][3] =  0.0;
  fParams[3][0] =  0.0;
  fParams[3][1] =  0.0;
  fParams[3][2] =  1.0;
  fParams[3][3] =  0.0;
}

void HHomMatrix::Multiply(const HPoint3D &rhs, HPoint3D &dest) const
{
  float x = rhs.X(), y=rhs.Y(), z=rhs.Z();
  float s = 1.0f / (fParams[3][0]*x+fParams[3][1]*y+fParams[3][2]*z+fParams[3][3]);
  dest[0] = s * (fParams[0][0]*x+fParams[0][1]*y+fParams[0][2]*z+fParams[0][3]);
  dest[1] = s * (fParams[1][0]*x+fParams[1][1]*y+fParams[1][2]*z+fParams[1][3]);
  dest[2] = s * (fParams[2][0]*x+fParams[2][1]*y+fParams[2][2]*z+fParams[2][3]);
}

void HHomMatrix::Multiply(const HPoint4D &rhs, HPoint3D &dest) const
{
  float x = rhs.X(), y=rhs.Y(), z=rhs.Z(), w=rhs.W();
  float s = 1.0f / (fParams[3][0]*x+fParams[3][1]*y+fParams[3][2]*z+fParams[3][3]*w);
  dest[0] = s * (fParams[0][0]*x+fParams[0][1]*y+fParams[0][2]*z+fParams[0][3]*w);
  dest[1] = s * (fParams[1][0]*x+fParams[1][1]*y+fParams[1][2]*z+fParams[1][3]*w);
  dest[2] = s * (fParams[2][0]*x+fParams[2][1]*y+fParams[2][2]*z+fParams[2][3]*w);
}

void HHomMatrix::Transpose(Self &dest) const
{
  dest.fParams[0][0] = fParams[0][0];
  dest.fParams[0][1] = fParams[1][0];
  dest.fParams[0][2] = fParams[2][0];
  dest.fParams[0][3] = fParams[3][0];
  dest.fParams[1][0] = fParams[0][1];
  dest.fParams[1][1] = fParams[1][1];
  dest.fParams[1][2] = fParams[2][1];
  dest.fParams[1][3] = fParams[3][1];
  dest.fParams[2][0] = fParams[0][2];
  dest.fParams[2][1] = fParams[1][2];
  dest.fParams[2][2] = fParams[2][2];
  dest.fParams[2][3] = fParams[3][2];
  dest.fParams[3][0] = fParams[0][3];
  dest.fParams[3][1] = fParams[1][3];
  dest.fParams[3][2] = fParams[2][3];
  dest.fParams[3][3] = fParams[3][3];
}

void HHomMatrix::Invert(Self &dest) const
{
  InvertMatrix3x3(fParams, dest.fParams);

  dest.fParams[0][3] = -(dest.fParams[0][0]*fParams[0][3]+dest.fParams[0][1]*fParams[1][3]+dest.fParams[0][2]*fParams[2][3]);
  dest.fParams[1][3] = -(dest.fParams[1][0]*fParams[0][3]+dest.fParams[1][1]*fParams[1][3]+dest.fParams[1][2]*fParams[2][3]);
  dest.fParams[2][3] = -(dest.fParams[2][0]*fParams[0][3]+dest.fParams[2][1]*fParams[1][3]+dest.fParams[2][2]*fParams[2][3]);

  dest.fParams[3][0] = 0;
  dest.fParams[3][1] = 0;
  dest.fParams[3][2] = 0;
  dest.fParams[3][3] = 1;
}

void HHomMatrix::GetTranslation(HPoint3D &translation) const
{
  float s = 1.0f / fParams[3][3];

  translation[0] = s*fParams[0][3];
  translation[1] = s*fParams[1][3];
  translation[2] = s*fParams[2][3];
}

void HHomMatrix::GetRotation(HPoint3D &rotation) const
{ // compute pitch=x, yaw=y, roll=z angles in the range [-pi/2,pi/2], [-pi,pi], [-pi,pi]
#if 0
  rotation[0] = atanf(-fParams[1][2]/fParams[2][2]);
  rotation[1] = atan2f(fParams[0][2], sqrtf(fParams[0][0]*fParams[0][0]+fParams[0][1]*fParams[0][1]));
  rotation[2] = atanf(-fParams[0][1]/fParams[0][0]);
#else
	// compute rotation angles from the matrix H
	// x,y,z are rotations in radians in the range -pi,pi
	float x,y,z;
	const float pi = 3.14159265358979f;
	// y = asin(H2) is the best choice if abs(H2) < sqrt(2)/2
	// otherwise, compute acos of the norm of elements 0 and 1
	if (fabs(fParams[0][2]) < 0.707f) {
		// in this branch, cos(y) is positive and cannot be zero
		y = asinf(fParams[0][2]);
		z = atan2f(-fParams[0][1],fParams[0][0]);
		x = atan2f(-fParams[1][2],fParams[2][2]);
	}
	else {
		// in this branch, sin(y) could be near +/-1
		// at y= +/- pi/2, the arguments of one of the atan2 functions could be 0,0
		// in the C++ library, atan2(0,0) is defined to be zero, so these cases are safe.
		// If a test were necessary, the fact that (1-sin(y))*(1+sin(y)) = cy^2
		// could be used to set sum or diff = 0, e.g. when cy < 1e-3
		float cy = sqrt(fParams[0][0]*fParams[0][0]+fParams[0][1]*fParams[0][1]);
		float sum, diff;
		y= acosf(cy);
		// check the sign of sin(y) = H2
		if (fParams[0][2]<0.0f) y = -y;
		// H5-H8 = (1+sin(y)) cos(z+x)
		// H4+H9 = (1+sin(y)) sin(z+x)
		sum = atan2f(fParams[1][0]+fParams[2][1],fParams[1][1]-fParams[2][0]);
		// H5+H8 = (1-sin(y)) cos(z-x)
		// H4-H9 = (1-sin(y)) sin(z-x)
		diff = atan2f(fParams[1][0]-fParams[2][1],fParams[1][1]+fParams[2][0]);
		z = (sum+diff)/2.0f;
		x = (sum-diff)/2.0f;
		// sum, diff, x and z are in the range -pi to pi
		// check the sign of cos(theta)
		int k = (int)(4*z/pi);
		bool flip = false;
		if (k<=-3 || k>=3) { // cz < 0
			if (fParams[0][0]>0.0f) flip = true;
		}
		else if (k<=-1) { // sz < 0
			if (fParams[0][1]<0.0f) flip = true;
		}
		else if (k>=1) { // sz > 0
			if (fParams[0][1]>0.0f) flip = true;
		}
		else { // cz > 0
			if (fParams[0][0]<0.0f) flip = true;
		}
		if (flip) {
			y = y>0.0f ? pi-y : -pi-y;
		}
	}
	if (fabsf(x)>pi/2.0f) {
		y=y<0.0 ? -y-pi: pi-y;
		x=x<0.0 ? x+pi : x-pi;
		z=z<0.0 ? z+pi : z-pi;
	}
  rotation[0] = x;
  rotation[1] = y;
  rotation[2] = z;
#endif
}

void HHomMatrix::Get(HPoint3D &translation, HPoint3D &rotation) const
{
  GetTranslation(translation);
  GetRotation(rotation);
}
