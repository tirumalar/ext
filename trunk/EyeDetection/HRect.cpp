#include "HRect.h"
#include <stdio.h>
#include <algorithm>
using std::max;
using std::min;
HRect HRect::Intersect(const HRect &roi)
{
	int xOffset = max(m_XOffset, roi.GetLeft());
	int yOffset = max(m_YOffset, roi.GetTop());
	int right = min(GetRight(), roi.GetRight());
	int bottom = min(GetBottom(), roi.GetBottom());
	return HRect(xOffset, yOffset, (right - xOffset + 1), (bottom - yOffset + 1));
}
void PrintROI(const char *name, const HRect &roi)
{
	printf("%s: %d %d %d %d\n", name, roi.GetTop(), roi.GetLeft(), roi.GetWidth(), roi.GetHeight());
}
