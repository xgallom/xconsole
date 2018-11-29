/*
 * text.h
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#ifndef TEXT_H_
#define TEXT_H_

#include "gdi.h"

namespace Gdi
{
	static const int
		TextWidth = Gdi::Width / 8,
		TextHeight = Gdi::Height / 8;

	void printChar(int x, int y, char c);
	void printCharRow(int x, int row, char c);

	int rowToLine(int row);
}

#endif /* TEXT_H_ */
