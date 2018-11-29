/*
 * vga_sync.h
 *
 *  Created on: Nov 28, 2018
 *      Author: xgallom
 */

#ifndef VGA_SYNC_H_
#define VGA_SYNC_H_

namespace Vga
{
	namespace HSync
	{
		void init();
	}

	namespace VSync
	{
		void init();
	}
}

extern "C" void TIM1_CC_IRQHandler(void);
extern "C" void TIM2_IRQHandler(void);

#endif /* VGA_SYNC_H_ */
