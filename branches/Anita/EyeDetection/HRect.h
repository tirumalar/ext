#pragma once

#include "HBox.h"
#include "CvInterfaceDefs.h"
#include "HPoint.h"

class HBOX_API HRect
{
public:
	HRect(int xOffset, int yOffset, int width, int height) : m_XOffset(xOffset), m_YOffset(yOffset), m_Width(width), m_Height(height) {}
	HRect() : m_XOffset(0), m_YOffset(0), m_Width(0), m_Height(0) {}

	HRect Intersect(const HRect &roi) ;

	int &GetTop() { return m_YOffset; }
	int &GetLeft() { return m_XOffset; }
	int &GetWidth() { return m_Width; }
	int &GetHeight() { return m_Height; }
	int GetRight() { return (m_XOffset + m_Width - 1); }
	int GetBottom() { return (m_YOffset + m_Height - 1); }

	const int &GetTop() const { return m_YOffset; }
	const int &GetLeft() const { return m_XOffset; }
	const int &GetWidth() const { return m_Width; }
	const int &GetHeight() const { return m_Height; }
	int GetRight() const { return (m_XOffset + m_Width - 1); }
	int GetBottom() const { return (m_YOffset + m_Height - 1); }

	bool Contains(const HPoint2Di &p) 
	{
		return (p.X() >= GetLeft() && p.X() < GetRight() && p.Y() >= GetTop() && p.Y() < GetBottom());
	}	


	HRect Scale(int scale) 
	{
		HRect rect;
		rect.m_XOffset = m_XOffset * scale;
		rect.m_YOffset = m_YOffset * scale;
		rect.m_Width = m_Width * scale;
		rect.m_Height = m_Height * scale;

		return rect;
	}	

	HRect Reduce(int level) const
	{
		HRect rect = *this;
		for(int i = 0; i < level; i++)
		{
			rect.m_XOffset = (rect.GetLeft() + 1) / 2;
			rect.m_YOffset = (rect.GetTop() + 1) / 2;
			rect.m_Width = (rect.GetWidth() + 1) / 2;
			rect.m_Height = (rect.GetHeight() + 1) / 2;
		}
		return rect;
	}	

	int m_XOffset;
	int m_YOffset;
	int m_Width;
	int m_Height;
};

void PrintROI(const char *name, const HRect &roi);
