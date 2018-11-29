/*
 * vga_sync.cpp
 *
 *  Created on: Nov 28, 2018
 *      Author: xgallom
 */

#include "vga_sync.h"
#include "vga.h"
#include "vga_framebuffer.h"

/**
 * 1 px = 160Mhz / 40MHz = 4 APB2 T clock cycles
 *
 * HSync:
 *   Period:             1056px
 *   Pulse:              128px
 *   Pulse + Back Porch: Pulse + 88px
 *
 * VSync:
 *   Period:             628ln
 *   Pulse:              4ln
 *   Pulse + Back Porch: Pulse + 23ln
 *
 * Timers:
 *
 *                                       -------------   -------------       -------------   -------------
 *                                       |  OC1 PWM  |   |    PE09   |       |  OC1 PWM  |   |    PA00   |
 *                                   +-->|   --_--   |-->|   HSync   |   +-->|   --_--   |-->|   VSync   |
 *                                   |   |  _ 128px  |   | OUT PP AF |   |   |   _ 4ln   |   | OUT PP AF |
 *                                   |   -------------   -------------   |   -------------   -------------
 *                                   |                                   |
 *                                   |                                   |
 *                                   |                                   |
 *   -------------   -------------   |   -------------   -------------   |   -------------
 *   |   APB2 T  |   |  Timer 1  |   |   |  OC2 ---  |   |  Timer 2  |   |   |  OC2 ---  |
 *   |   160MHz  |-->|  37.9KHz  |---+-->|  37.9KHz  |-->|  60.31Hz  |---+-->|  60.31Hz  |
 *   |     4px   |   |   1056px  |   |   |    1ln    |   |   628ln   |   |   |   628ln   |
 *   -------------   -------------   |   -------------   -------------   |   -------------
 *                                   |                                   |
 *                                   |                                   |
 *                                   |                                   |
 *                                   |   -------------                   |   -------------
 *                                   |   | IRQ OC2CC |                   |   |  IRQ OC2  |
 *                                   +-->|  37.9KHz  |                   +-->|  60.31Hz  |
 *                                       | DMA HLine |                       | DMA VLine |
 *                                       -------------                       -------------
 *
 */

namespace Vga
{
	static const uint32_t px = 4;

	namespace HSync
	{
		static const uint32_t
			Period = 1056 * px,
			Pulse = 128 * px,
			PulseBackPorch = Pulse + 88 * px,
			Pin = GPIO_Pin_9,
			PinSrc = GPIO_PinSource9;

		static GPIO_TypeDef * const Port = GPIOE;
		static const uint32_t PortRcc = RCC_AHB1Periph_GPIOE;

		static TIM_TypeDef * const Timer = TIM1;
		static const uint32_t TimerRcc = RCC_APB2Periph_TIM1;

		static const TIM_TimeBaseInitTypeDef TimeBase = {
				.TIM_Prescaler = 0,
				.TIM_CounterMode = TIM_CounterMode_Up,
				.TIM_Period = Period,
				.TIM_ClockDivision = TIM_CKD_DIV1,
				.TIM_RepetitionCounter = 0
		};

		static const TIM_OCInitTypeDef OutputChannel1 = {
				.TIM_OCMode = TIM_OCMode_PWM2,
				.TIM_OutputState = TIM_OutputState_Enable,
				.TIM_OutputNState = TIM_OutputNState_Enable,
				.TIM_Pulse = Pulse,
				.TIM_OCPolarity = TIM_OCPolarity_Low,
				.TIM_OCNPolarity = TIM_OCNPolarity_High,
				.TIM_OCIdleState = TIM_OCIdleState_Reset,
				.TIM_OCNIdleState = TIM_OCNIdleState_Set
		};
		static const TIM_OCInitTypeDef OutputChannel2 = {
				.TIM_OCMode = TIM_OCMode_Inactive,
				.TIM_OutputState = TIM_OutputState_Enable,
				.TIM_OutputNState = TIM_OutputNState_Enable,
				.TIM_Pulse = PulseBackPorch,
				.TIM_OCPolarity = TIM_OCPolarity_Low,
				.TIM_OCNPolarity = TIM_OCNPolarity_High,
				.TIM_OCIdleState = TIM_OCIdleState_Reset,
				.TIM_OCNIdleState = TIM_OCNIdleState_Set
		};

		static const NVIC_InitTypeDef Irq = {
				.NVIC_IRQChannel = TIM1_CC_IRQn,
				.NVIC_IRQChannelPreemptionPriority = 1,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE
		};
		static const uint16_t IrqSrc = TIM_IT_CC2;

		static const GPIO_InitTypeDef Gpio = {
				.GPIO_Pin = Pin,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_Speed = GPIO_High_Speed,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
		};
		static const uint8_t GpioAf = GPIO_AF_TIM1;
	}

	namespace VSync
	{
		static const uint32_t
			Period = 628,
			Pulse = 4,
			PulseBackPorch = Pulse + 23,
			Pin = GPIO_Pin_0,
			PinSrc = GPIO_PinSource0;

		static GPIO_TypeDef * const Port = GPIOA;
		static const uint32_t PortRcc = RCC_AHB1Periph_GPIOA;

		static TIM_TypeDef * const Timer = TIM2;
		static const uint32_t TimerRcc = RCC_APB1Periph_TIM2;

		static const TIM_TimeBaseInitTypeDef TimeBase = {
				.TIM_Prescaler = 0,
				.TIM_CounterMode = TIM_CounterMode_Up,
				.TIM_Period = Period,
				.TIM_ClockDivision = TIM_CKD_DIV1,
				.TIM_RepetitionCounter = 0
		};

		static const TIM_OCInitTypeDef OutputChannel1 = { .TIM_OCMode = TIM_OCMode_PWM2,
				.TIM_OutputState = TIM_OutputState_Enable,
				.TIM_OutputNState = TIM_OutputNState_Enable,
				.TIM_Pulse = Pulse,
				.TIM_OCPolarity = TIM_OCPolarity_Low,
				.TIM_OCNPolarity = TIM_OCNPolarity_High,
				.TIM_OCIdleState = TIM_OCIdleState_Reset,
				.TIM_OCNIdleState = TIM_OCNIdleState_Set
		};
		static const TIM_OCInitTypeDef OutputChannel2 = {
				.TIM_OCMode = TIM_OCMode_Inactive,
				.TIM_OutputState = TIM_OutputState_Enable,
				.TIM_OutputNState = TIM_OutputNState_Enable,
				.TIM_Pulse = PulseBackPorch,
				.TIM_OCPolarity = TIM_OCPolarity_Low,
				.TIM_OCNPolarity = TIM_OCNPolarity_High,
				.TIM_OCIdleState = TIM_OCIdleState_Reset,
				.TIM_OCNIdleState = TIM_OCNIdleState_Set
		};

		static const NVIC_InitTypeDef Irq = {
				.NVIC_IRQChannel = TIM2_IRQn,
				.NVIC_IRQChannelPreemptionPriority = 1,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE
		};
		static const uint16_t IrqSrc = TIM_IT_CC2;

		static const GPIO_InitTypeDef Gpio = {
				.GPIO_Pin = Pin,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_Speed = GPIO_High_Speed,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
		};
		static const uint8_t GpioAf = GPIO_AF_TIM2;
	}

	namespace HSync
	{
		void init()
		{
			RCC_APB2PeriphClockCmd(TimerRcc, ENABLE);
			RCC_AHB1PeriphClockCmd(PortRcc, ENABLE);

			TIM_TimeBaseInit(Timer, &TimeBase);
			TIM_OC1Init(Timer, &OutputChannel1);
			TIM_OC2Init(Timer, &OutputChannel2);

			TIM_Cmd(Timer, ENABLE);

			GPIO_Init(Port, &Gpio);
			GPIO_PinAFConfig(Port, PinSrc, GpioAf);

			TIM_CtrlPWMOutputs(Timer, ENABLE);

			TIM_SelectMasterSlaveMode(Timer, TIM_MasterSlaveMode_Enable);
			TIM_SelectOutputTrigger(Timer, TIM_TRGOSource_Update);

			NVIC_Init(&Irq);
			TIM_ITConfig(Timer, IrqSrc, ENABLE);
		}
	}

	namespace VSync
	{
		void init()
		{
			RCC_APB1PeriphClockCmd(TimerRcc, ENABLE);
			RCC_AHB1PeriphClockCmd(PortRcc, ENABLE);

			TIM_TimeBaseInit(Timer, &TimeBase);
			TIM_OC1Init(Timer, &OutputChannel1);
			TIM_OC2Init(Timer, &OutputChannel2);

			TIM_Cmd(Timer, ENABLE);

			GPIO_Init(Port, &Gpio);
			GPIO_PinAFConfig(Port, PinSrc, GpioAf);

			TIM_CtrlPWMOutputs(Timer, ENABLE);

			TIM_SelectSlaveMode(Timer, TIM_SlaveMode_Gated);
			TIM_SelectInputTrigger(Timer, TIM_TS_ITR0);

			NVIC_Init(&Irq);
			TIM_ITConfig(Timer, IrqSrc, ENABLE);
		}
	}
}

extern "C" void TIM1_CC_IRQHandler(void)
{
	Vga::clearHBlank();

	if(!(Vga::state() & Vga::VBlank))
		Vga::FrameBuffer::sendRow();

	Vga::HSync::Timer->SR &= ~0x0004;
}

extern "C" void TIM2_IRQHandler(void)
{
	Vga::clearVBlank();

	Vga::VSync::Timer->SR &= ~0x0004;
}
