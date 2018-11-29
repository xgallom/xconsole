/*
 * vga.cpp
 *
 *  Created on: Nov 28, 2018
 *      Author: xgallom
 */

#include "vga.h"
#include "vga_sync.h"
#include "vga_framebuffer.h"

/**
 * SVGA 800x600 @ 60Hz
 * Vertical refresh: 37.878787878788 kHz
 * Pixel frequency:  40 MHz
 *
 * System clock is underclocked from 180MHz to 160MHz
 *
 *   +-------------------------+------|
 *   |     Horizontal line     | Unit |
 *   |          1056           |  px  |
 *   |          26.4           |  us  |
 *   |---------+----+-----+----|      |
 *   | Visible | FP |  S  | BP |      |
 *   |   800   | 40 | 128 | 88 |  px  |
 *   |    20   |  1 | 3.2 | 2.2|  us  |
 *   +---------+----+-----+----+------|
 *
 *   +-------------------------+------|
 *   |      Vertical line      | Unit |
 *   |           628           |  ln  |
 *   |         16.5792         |  ms  |
 *   |---------+----+-----+----|      |
 *   | Visible | FP |  S  | BP |      |
 *   |   600   |  1 |  4  | 23 |  ln  |
 *   |  15.84  |.026|.1056|.607|  ms  |
 *   +---------+----+-----+----+------|
 *
 * Visible - Visible scanline area
 * FP - Front Porch
 * S  - Synchronization pulse
 * BP - Back Porch
 *
 * px - Pixel clocks
 * us - Microseconds
 * ln - Horizontal lines
 * ms - Milliseconds
 */

namespace Vga
{
	static volatile uint8_t s_state = 0x00;

	void init()
	{
		FrameBuffer::init();

		HSync::init();
		VSync::init();
	}

	volatile uint8_t state()
	{
		return s_state;
	}

	void setHBlank()
	{
		s_state |= HBlank;
	}

	void clearHBlank()
	{
		s_state &= ~HBlank;
	}

	void setVBlank()
	{
		s_state |= VBlank;
	}

	void clearVBlank()
	{
		s_state &= ~VBlank;
	}

}
