/*
 * usart.h
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#ifndef USART_H_
#define USART_H_

#include "stm32f4xx.h"

namespace Usart
{
	void init();

	volatile bool available();
	volatile uint8_t read();
}

extern "C" void USART1_IRQHandler(void);

#endif /* USART_H_ */
