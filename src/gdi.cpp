/*
 * gdi.cpp
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#include "gdi.h"
#include "vga.h"
#include "util.h"

namespace Gdi
{
	static inline int X(int x)
	{
		return x + StartX;
	}

	static inline int Y(int y)
	{
		return y + StartY;
	}

	static inline int getOffset(int x, int y)
	{
		return (Y(y) << Vga::FrameBuffer::WidthOffset) | (X(x) >> 3);
	}

	static inline uint8_t getPixel(int x)
	{
		return 1 << (7 - (X(x) & 7));
	}

	void directWrite(int byte, int row, uint8_t value)
	{
		const int offset = getOffset(0, row);

		Vga::FrameBuffer::buffer()[offset + byte] = value;
	}

	void setPixel(int x, int y)
	{
		const int offset = getOffset(x, y);
		const uint8_t pixel = getPixel(x);

		Vga::FrameBuffer::buffer()[offset] |= pixel;
	}

	void clrPixel(int x, int y)
	{
		const int offset = getOffset(x, y);
		const uint8_t pixel = getPixel(x);

		Vga::FrameBuffer::buffer()[offset] &= ~pixel;
	}

	void cls()
	{
		for(int n = 0; n < Vga::FrameBuffer::Size; ++n) {
			Vga::FrameBuffer::buffer()[n] = 0x00;
		}
	}

	void hline(int x0, int x1, int y)
	{
		while(x1 - x0)
			setPixel(x0++, y);
	}

	void vline(int y0, int y1, int x)
	{
		while(y1 - y0)
			setPixel(x, y0++);
	}

	void line(int x0, int y0, int x1, int y1)
	{
		if(y0 == y1)
			hline(min(x0, x1), max(x0, x1), y0);
		else if(x0 == x1)
			vline(min(y0, y1), max(y0, y1), x0);
		else {
			float
				dx = x1 - x0,
				dy = y1 - y0,
				derr = abs(dy / dx),
				err = 0.0;

			int y = y0;

			for(int x = x0; x < x1; ++x) {
				setPixel(x, y);

				err += derr;

				if(err >= 0.5) {
					y += sgn(dy);
					err -= 1.0;
				}
			}
		}
	}

	void border(int x0, int y0, int x1, int y1)
	{
		hline(x0, x1, y0);
		hline(x0, x1, y1 - 1);

		vline(y0, y1, x0);
		vline(y0, y1, x1 - 1);
	}
}

