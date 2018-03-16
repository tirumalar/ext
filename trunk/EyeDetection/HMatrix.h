#pragma once

#include "HBox.h"
#include "HUtilities.h"
#include "HPoint.h"
#include <math.h> // for  double exp(double);

// Matrices ___________________________________________________________

class HBOX_API HMatrix
{
public:
  HMatrix(void);
  HMatrix(int Rows);
  HMatrix(int Rows, int Columns);
  HMatrix(const HMatrix& Matrix);
  ~HMatrix(void);

  int GetRows() const { return fRows; }
  int GetCols() const { return fColumns; }

  HMatrix& operator=(const HMatrix& Matrix);

  HMatrix operator*(const float scalar) const;
  HMatrix operator+(const float scalar) const;
  HMatrix operator-(const float scalar) const;

  HMatrix operator*(const HMatrix& Matrix) const;
  HMatrix operator+(const HMatrix& Matrix) const;
  HMatrix operator-(const HMatrix& Matrix) const;

  HMatrix& operator+=(const HMatrix& Matrix);
  HMatrix& operator-=(const HMatrix& Matrix);

  HMatrix& operator*=(const float scalar);
  HMatrix& operator /=(const float scalar);

  void Alloc(int Rows, int Columns);
  void Free(void);

  void Print(void) const;
  //HMatrix Inverse() const;
  HMatrix Transpose() const;

  inline const float& operator()(int Row, int Column) const {return fElement[Row*fColumns+Column];}
  inline float& operator()(int Row, int Column) {return fElement[Row*fColumns+Column];}

  inline float* operator[](int Row) const {return &fElement[Row*fColumns];}
  inline const float& operator()(int Row) const {return fElement[Row];}
  inline float& operator()(int Row) {return fElement[Row];}

  void Multiply(const HMatrix &Right, HMatrix &Result) const;
  void SetAll(float Value)
  {
	  for(int y = 0; y < fRows; y++)
	  {
		  for(int x = 0; x < fColumns; x++)
		  {
			  (*this)(y, x) = Value;
		  }
	  }
  }

  void SetToIdentity(void)
  {
	  SetAll(0);
	  int dim = HMin(fRows, fColumns);
	  for(int i = 0; i < dim; i++)
	  {
		 (*this)(i, i) = 1.0;
	  }
  }


public:
  int	      fRows;
  int	      fColumns;
  float*      fElement;
};


class HBOX_API HHomMatrix
{
  typedef HHomMatrix Self;

  public:

    HHomMatrix() {}
    HHomMatrix(
      float xx, float xy, float xz, float xw,
      float yx, float yy, float yz, float yw,
      float zx, float zy, float zz, float zw,
      float wx, float wy, float wz, float ww);

    HHomMatrix(const HHomMatrix& HomMatrix);

	inline float& operator()(int i, int j) { return fParams[i][j]; }
	inline const float& operator()(int i, int j) const { return fParams[i][j]; }

	void SetToZero(void);
	void SetToIdentity(void);
	void SetRotation(const HPoint3D &rotation);
	void SetRotationInv(const HPoint3D &rotation);
	void SetTranslation(const HPoint3D &translation);
	void Set(const HPoint3D &translation, const HPoint3D &rotation);
	void SetInv(const HPoint3D &translation, const HPoint3D &rotation);

	void GetRotation(HPoint3D &rotation) const;
	void GetTranslation(HPoint3D &translation) const;
	void Get(HPoint3D &translation, HPoint3D &rotation) const;

	void SetProjectionZ(float fx, float fy, float cx, float cy);

	void Multiply(const Self &rhs, Self &dest) const
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				dest(i, j) = 0.0f;
				for(int k = 0; k < 4; k++)
				{
					dest(i, j) += fParams[i][k] * rhs(k, j);
				}
			}
		}
	}

	HHomMatrix& operator=(const HHomMatrix& Matrix)
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				fParams[i][j] = Matrix.fParams[i][j];
			}
		}
		return *this;
	}

	bool operator==(const HHomMatrix& Matrix) const
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				if(fParams[i][j] != Matrix.fParams[i][j])
				{
					return false;
				}
			}
		}
		return true;
	}

	HHomMatrix operator*(const HHomMatrix &rhs) const
	{
		HHomMatrix dest;
		this->Multiply(rhs, dest);
		return dest;
	}

	void Multiply(const HPoint3D &rhs, HPoint3D &dest) const;

	HPoint3D operator*(const HPoint3D &rhs) const
	{
		HPoint3D dest;
		this->Multiply(rhs, dest);
		return dest;
	}

	void Multiply(const HPoint4D &rhs, HPoint3D &dest) const;
	void Multiply(const HPoint4D &rhs, HPoint4D &dest) const
	{
		for(int i = 0; i < 4; i++)
		{
			dest[i] = 0.0f;
			for(int j = 0; j < 4; j++)
			{
				dest[i] += fParams[i][j] * rhs[j];
			}
		}
	}

	HPoint3D operator*(const HPoint4D &rhs) const
	{
		HPoint4D dest;
		this->Multiply(rhs, dest);
		return dest;
	}

	void Transpose(Self &dest) const;

	HHomMatrix Transpose() const
	{
		HHomMatrix dest;
		this->Transpose(dest);
		return dest;
	}

	void Invert(Self &dest) const;

	HHomMatrix Invert() const
	{
		HHomMatrix dest;
		this->Invert(dest);
		return dest;
	}

	void Print(char *name, char *format) const;

public:

	float fParams[4][4];
};
