/*
 * usart.cpp
 *
 *  Created on: Nov 29, 2018
 *      Author: xgallom
 */

#include "usart.h"

namespace Usart
{
	static volatile bool s_available = false;
	static volatile uint8_t s_data = 0x00;

	static const uint32_t
		BaudRate = 9600;

	static USART_TypeDef * const Usart = USART1;
	static const uint32_t UsartRcc = RCC_APB2Periph_USART1;

	static GPIO_TypeDef * const Port= GPIOA;
	static const uint32_t PortRcc = RCC_AHB1Periph_GPIOA;

	static const uint32_t
		Rx = GPIO_Pin_10,
		Tx = GPIO_Pin_9,
		RxSrc = GPIO_PinSource10,
		TxSrc = GPIO_PinSource9;

	static const USART_InitTypeDef Init = {
			.USART_BaudRate = BaudRate,
			.USART_WordLength = USART_WordLength_8b,
			.USART_StopBits = USART_StopBits_1,
			.USART_Parity = USART_Parity_No,
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None
	};

	static const GPIO_InitTypeDef Gpio = {
			.GPIO_Pin = Rx | Tx,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_Speed = GPIO_High_Speed,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
	};
	static const uint8_t GpioAf = GPIO_AF_USART1;

	static const NVIC_InitTypeDef Nvic = {
			.NVIC_IRQChannel = USART1_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 0,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE
	};

	void init()
	{
		RCC_APB2PeriphClockCmd(UsartRcc, ENABLE);
		RCC_AHB1PeriphClockCmd(PortRcc, ENABLE);

		GPIO_Init(Port, &Gpio);

		GPIO_PinAFConfig(Port, RxSrc, GpioAf);
		GPIO_PinAFConfig(Port, TxSrc, GpioAf);

		USART_Init(Usart, &Init);

		USART_Cmd(Usart, ENABLE);

		USART_ITConfig(Usart, USART_IT_RXNE, ENABLE);

		NVIC_Init(&Nvic);
	}

	volatile bool available()
	{
		return s_available;
	}

	volatile uint8_t read()
	{
		s_available = false;
		return s_data;
	}
}

extern "C" void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(Usart::Usart, USART_IT_RXNE)) {
		Usart::s_available = true;
		Usart::s_data = static_cast<uint8_t>(USART_ReceiveData(Usart::Usart));

		USART_SendData(Usart::Usart, Usart::s_data);

		USART_ClearITPendingBit(Usart::Usart, USART_IT_RXNE);
	}
}
