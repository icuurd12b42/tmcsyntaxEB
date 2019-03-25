#include "stdafx.h"
#include "DrawingHelpers.h"


void DrawFilledRect(HDC hDC, RECT rect, COLORREF col)
{
	//Whi is this not part of winapi....
	HBRUSH hBrush = CreateSolidBrush(col);
	FillRect(hDC, &rect, hBrush);
	DeleteObject(hBrush);
}
void DrawFrameRect(HDC hDC, RECT rect, COLORREF col)
{
	//Whi is this not part of winapi....
	HBRUSH hBrush = CreateSolidBrush( col);
	FrameRect(hDC, &rect, hBrush);
	DeleteObject(hBrush);
}