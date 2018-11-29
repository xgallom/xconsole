/*
 * vga.h
 *
 *  Created on: Nov 28, 2018
 *      Author: xgallom
 */

#ifndef VGA_H_
#define VGA_H_

#include "stm32f4xx.h"

namespace Vga
{
	void init();

	static const uint8_t
		HBlank = 0x01,
		VBlank = 0x02;

	volatile uint8_t state();

	void setHBlank();
	void clearHBlank();
	void setVBlank();
	void clearVBlank();
}

#endif /* VGA_H_ */
