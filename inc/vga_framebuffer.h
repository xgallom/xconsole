/*
 * vga_framebuffer.h
 *
 *  Created on: Nov 28, 2018
 *      Author: xgallom
 */

#ifndef VGA_FRAMEBUFFER_H_
#define VGA_FRAMEBUFFER_H_

#include "stm32f4xx.h"

namespace Vga
{
	namespace FrameBuffer
	{
		static const int
			FrameBufferWidth = 800 / 8,
			FrameBufferHeight = 600,
			Size = FrameBufferWidth * FrameBufferHeight;

		void init();

		void sendRow();

		volatile uint8_t *buffer();
	}
}

extern "C" void DMA2_Stream3_IRQHandler(void);

#endif /* VGA_FRAMEBUFFER_H_ */
