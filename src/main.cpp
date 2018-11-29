/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */

#include "vga.h"
#include "usart.h"
#include "gdi.h"
#include "text.h"

struct Position {
	int x, y;
};

void printCharAt(Position position, char c)
{

}

int main(void)
{
	Vga::init();
	Usart::init();

	Gdi::border();

	static const int BufferSize = Gdi::TextWidth * Gdi::TextHeight;
	char buffer[BufferSize];
	Position positions[BufferSize];

	int bufferPosition = 0, bufferEnd = 0, x = 0, y = 0;

	Gdi::printCharRow(x, y, '_');

	bool escape = false;

	while(bufferPosition < BufferSize) {
		if(Usart::available()) {
			char input = Usart::read();

			Gdi::printCharRow(x, y, 0);

			if(escape) {
				if(input == 0x1b) {
					bufferPosition = 0;
					x = 0;
					y = 0;

					Gdi::cls();
					Gdi::border();
					escape = false;
				}
				else if(input == '[') {
					while(!Usart::available()) {
					}

					input = Usart::read();
				}
			}
			else {
				buffer[bufferPosition] = input;

				if(input == 0x1b) {
					escape = true;
				}
				else if(input == 0x0d) {
					positions[bufferPosition] = Position{ .x = x, .y = y };

					x = 0;
					++y;

					++bufferPosition;
				}
				else if(input == '\b') {
					if(bufferPosition) {
						--bufferPosition;

						buffer[bufferPosition] = 0x00;
						x = positions[bufferPosition].x;
						y = positions[bufferPosition].y;

						Gdi::printCharRow(x, y, buffer[bufferPosition]);
					}
				}
				else {
					Gdi::printCharRow(x, y, buffer[bufferPosition]);
					positions[bufferPosition] = Position{ .x = x, .y = y };

					if(++x == Gdi::TextWidth) {
						x = 0;
						++y;
					}

					++bufferPosition;
				}
			}

			Gdi::printCharRow(x, y, '_');
		}
	}

	for (;;) {
	}
}
