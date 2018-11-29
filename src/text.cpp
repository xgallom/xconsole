/*
 * text.cpp
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#include "text.h"
#include "gdi.h"
#include "font.h"

namespace Gdi
{
	void printChar(int x, int y, char c)
	{
		for(int n = 0; n < Font::Rows; ++n)
			directWrite(x, y + n, Font::data(c, n));
	}

	void printCharRow(int x, int row, char c)
	{
		printChar(x, rowToLine(row), c);
	}

	int rowToLine(int row)
	{
		return row * Font::Rows;
	}
}

