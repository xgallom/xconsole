/*
 * gdi.h
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#ifndef GDI_H_
#define GDI_H_

#include "stm32f4xx.h"
#include "vga_framebuffer.h"

namespace Gdi
{
#if VGA_FB_SCALE_DOUBLE
	static const int
		Width = 320,
		Height= 240,
		StartX = 32,
		StartY = 30;
#else
	static const int
			Width = 640,
			Height= 480,
			StartX = 64,
			StartY = 60;
#endif

	void directWrite(int byte, int row, uint8_t value);

	void setPixel(int x, int y);
	void clrPixel(int x, int y);

	void cls();

	void hline(int x0, int x1, int y);
	void vline(int y0, int y1, int x);

	void line(int x0, int y0, int x1, int y1);

	void border(int x0 = -2, int y0 = -2, int x1 = Width + 2, int y1 = Height + 2);
}

#endif /* GDI_H_ */
