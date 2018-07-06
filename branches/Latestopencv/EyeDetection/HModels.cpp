#include "HModels.h"

/*
 * H3DObject methods
 */

H3DObject::H3DObject() : m_HasRTMatrix(false), m_HasRTInvMatrix(false), m_HasTranslation(false), m_HasRotation(false)
{
}

H3DObject::H3DObject( const HPoint3D &translation, const HPoint3D &rotation )
: m_Translation(translation)
, m_Rotation(rotation)
{

}

H3DObject::H3DObject( const Self &object )
: m_Translation( object.m_Translation )
, m_HasTranslation(true)
, m_Rotation( object.m_Rotation )
, m_HasRotation(true)
{
	// should compute matrix here from euler angles
}

void H3DObject::SetRT(const HHomMatrix &RT)
{
	m_RT = RT;
	m_HasRTMatrix = true;

	m_RTInv = RT.Invert();
	m_HasRTInvMatrix = true;
}

void H3DObject::Initialize()
{

}

/*
 * H3DSensor
 */

H3DSensor::H3DSensor() : H3DObject(), m_HasPMatrix(false), m_HasKMatrix(false)
{

}

H3DSensor::H3DSensor( const Self &object ) : H3DObject( object )
{

}

H3DSensor::H3DSensor( const HPoint3D &translation, const HPoint3D &rotation ) : H3DObject( translation, rotation )
{

}

/*
 * HCuboid methods
 */

HCuboid::HCuboid(const HPoint3D &translation, const HPoint3D &rotation, float width, float height, float length)
: Super(translation, rotation)
, m_fWidth(width)
, m_fHeight(height)
, m_fLength(length)
{

}

HCuboid::HCuboid(const Self &object)
: Super( object.m_Translation, object.m_Rotation )
, m_fWidth(object.m_fWidth)
, m_fHeight(object.m_fHeight)
, m_fLength(object.m_fLength)
{

}

void HCuboid::SetDimensions( const float &width, const float &height, const float &length )
{
	m_fWidth = width;
	m_fHeight = height;
	m_fLength = length;
}

void HCuboid::GetDimensions( float &width, float &height, float &length ) const
{
	width = m_fWidth;
	height = m_fHeight;
	length = m_fLength;
}
