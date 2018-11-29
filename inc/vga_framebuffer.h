/*
 * vga_framebuffer.h
 *
 *  Created on: Nov 28, 2018
 *      Author: xgallom
 */

#ifndef VGA_FRAMEBUFFER_H_
#define VGA_FRAMEBUFFER_H_

#include "stm32f4xx.h"

#define VGA_FB_SCALE_DOUBLE 1

namespace Vga
{
	namespace FrameBuffer
	{
#if VGA_FB_SCALE_DOUBLE
		static const int
			WidthOffset = 7,
			WidthBytes = 1 << WidthOffset,
			Width = 400 / 8,
			Height = 300,
			Size = WidthBytes * Height;
#else
		static const int
			WidthOffset = 7,
			WidthBytes = 1 << WidthOffset,
			Width = 800 / 8,
			Height = 600,
			Size = WidthBytes * Height;
#endif

		void init();

		void sendRow();

		volatile uint8_t *buffer();
	}
}

extern "C" void DMA2_Stream3_IRQHandler(void);

#endif /* VGA_FRAMEBUFFER_H_ */
