/*
 * font.h
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#ifndef FONT_H_
#define FONT_H_

#include "stm32f4xx.h"

namespace Font
{
	static const int
		Chars = 0x100,
		Rows = 8;

	uint8_t data(char c, int row);
}

#endif /* FONT_H_ */
