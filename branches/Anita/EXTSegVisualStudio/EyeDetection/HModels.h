#pragma once

#include "HUtilities.h"
#include "HPoint.h"
#include "HMatrix.h"

class HBOX_API H3DObject
{
public:

	typedef H3DObject Self;

	H3DObject();
	H3DObject( const Self &object );
	H3DObject( const HPoint3D &translation, const HPoint3D &rotation );

	const HPoint3D& GetTranslation() const { return m_Translation; }
	HPoint3D& GetTranslation() { return m_Translation; }

	const HPoint3D& GetRotation() const { return m_Rotation; }
	HPoint3D& GetRotation() { return m_Rotation; }

	const HHomMatrix & GetRT() const { return m_RT; }
	HHomMatrix & GetRT() { return m_RT; }

	const HHomMatrix & GetRTInv() const { return m_RTInv; }
	HHomMatrix & GetRTInv() { return m_RTInv; }

	void SetRT( const HHomMatrix &RT );

	void Initialize();

protected:

	bool m_HasRTMatrix;
	HHomMatrix m_RT;

	bool m_HasRTInvMatrix;
	HHomMatrix m_RTInv;

	HPoint3D m_Translation;
	bool m_HasTranslation;

	HPoint3D m_Rotation;
	bool m_HasRotation;
};


/*
Extrinsic parameters:

Translation vector: Tc_ext = [ -614.664080 	 373.547297 	 3811.248450 ]
Rotation vector:   omc_ext = [ 1.589648 	 1.592714 	 -0.939808 ]
Rotation matrix:    Rc_ext = [ -0.013829 	 0.999691 	 -0.020653
                               0.501411 	 -0.010937 	 -0.865140
                               -0.865099 	 -0.022320 	 -0.501105 ]

Pixel error:           err = [ 0.59113 	 1.21347 ]

Focal Length:          fc = [ 1958.79504   1968.62130 ] � [ 10.21546   12.09922 ]
Principal point:       cc = [ 528.65725   624.46763 ] � [ 17.84715   19.03952 ]
Skew:             alpha_c = [ 0.00000 ] � [ 0.00000  ]   => angle of pixel axes = 90.00000 � 0.00000 degrees
Distortion:            kc = [ -0.34610   0.64953   0.00000   0.00000  0.00000 ] � [ 0.04436   0.35979   0.00000   0.00000  0.00000 ]
Pixel error:          err = [ 1.38372   1.33894 ]

*/

class HBOX_API H3DSensor : public H3DObject
{
public:

	typedef H3DSensor Self;

	H3DSensor();
	H3DSensor( const Self &object );
	H3DSensor( const HPoint3D &translation, const HPoint3D &rotation );

	const HHomMatrix &GetP() const { return m_P; }
	HHomMatrix &GetP() { return m_P; }

	const HHomMatrix &GetK() const { return m_K; }
	HHomMatrix &GetK() { return m_K; }

	HPoint2D & GetFocalLength() { return m_FocalLength; }
	const HPoint2D & GetFocalLength() const { return m_FocalLength; }

	HPoint2D & GetPrincipalPoint() { return m_PrincipalPoint; }
	const HPoint2D & GetPrincipalPoint() const { return m_PrincipalPoint; }

protected:

	bool m_HasPMatrix;
	HHomMatrix m_P;

	bool m_HasKMatrix;
	HHomMatrix m_K;

	HPoint2D m_FocalLength;

	HPoint2D m_PrincipalPoint;

	float m_Skew;

	float m_Distortion[5];
};

class HBOX_API HCuboid : public H3DObject
{
public:

	typedef H3DObject Super;
	typedef HCuboid Self;

	HCuboid() {}
	HCuboid(const Self &object);
	HCuboid(const HPoint3D &translation, const HPoint3D &rotation, float width, float height, float length);

	const float& GetWidth() const { return m_fWidth; }
	float& GetWidth() { return m_fWidth; }

	const float& GetHeight() const { return m_fHeight; }
	float& GetHeight() { return m_fHeight; }

	const float& GetLength() const { return m_fLength; }
	float& GetLength() { return m_fLength; }

	void SetDimensions( const float &width, const float &height, const float &length );
	void GetDimensions( float &width, float &height, float &length ) const;

protected:

	float m_fWidth;
	float m_fHeight;
	float m_fLength;
};
