/*
 * vga_framebuffer.cpp
 *
 *  Created on: Nov 28, 2018
 *      Author: xgallom
 */

#include "vga_framebuffer.h"
#include "vga.h"

/**
 * Frame Buffer:
 *   Resolution: 800x600 px
 *   Depth:      1 bpp
 *   Width:      100 B
 *   Height:     600 ln
 *   Size:       60000 B (58 kB)
 *
 * Spi:
 *   Frequency: 20MHz
 *   Data size: 1 B
 *   Baud rate: 0.5 px
 *
 * Dma:
 *   s_frameBuffer --> Spi
 *
 * Half resolution mode (VGA_FB_SCALE_DOUBLE):
 *   Duplicates columns (Uses half clock speed)
 *   Duplicates rows (sends same row twice)
 *
 * Peripherals:
 *
 *   -------------
 *   | IRQ T1CC2 |
 *   |  37.9KHz  |--+                        Trigger
 *   | DMA HLine |  |            +------------------------------+
 *   -------------  | Start      |                              |
 *                  |            v                              |
 *   -------------  |      -------------                        |
 *   |    AHB1   |  +----->|  DMA2 S3  |                        |
 *   |   160MHz  |----Clk->|   160MHz  |-----+                  |
 *   |     4px   |  +----->|    CH 3   |     |                  |
 *   -------------  |      -------------     | Data             |
 *                  |                        |                  |
 *   -------------  | Data    -------------  |   -------------  |   -------------
 *   | Frame Buf |  |         |   APB2 P  |  +-->|   SPI 1   |--+   |    PA07   |
 *   |  800x600  |--+         |    80MHz  |----->|   40MHz   |----->|    RGB    |
 *   |  60000 B  |            |     2px   | Clk  |    1px    | Data | OUT PP AF |
 *   -------------            -------------      -------------      -------------
 *
 * For IRQ T1CC2 see vga_sync IRQ OC2CC
 *
 */

namespace Vga
{
	namespace FrameBuffer
	{
		static volatile uint8_t s_frameBuffer[Size] __attribute__((aligned (4)));
	}

	namespace Spi
	{
		static SPI_TypeDef * const Spi = SPI1;
		static const uint32_t SpiRcc = RCC_APB2Periph_SPI1;

#if VGA_FB_SCALE_DOUBLE
		static const uint16_t Prescaler = SPI_BaudRatePrescaler_4;
#else
		static const uint16_t Prescaler = SPI_BaudRatePrescaler_2;
#endif

		static const uint32_t
			Pin = GPIO_Pin_7,
			PinSrc = GPIO_PinSource7;

		static GPIO_TypeDef * const Port = GPIOA;
		static const uint32_t PortRcc = RCC_AHB1Periph_GPIOA;

		static const SPI_InitTypeDef Init = {
				.SPI_Direction = SPI_Direction_1Line_Tx,
				.SPI_Mode = SPI_Mode_Master,
				.SPI_DataSize = SPI_DataSize_8b,
				.SPI_CPOL = SPI_CPOL_Low,
				.SPI_CPHA = SPI_CPHA_2Edge,
				.SPI_NSS = SPI_NSS_Soft,
				.SPI_BaudRatePrescaler = Prescaler,
				.SPI_FirstBit = SPI_FirstBit_MSB,
				.SPI_CRCPolynomial = 7
		};

		static const GPIO_InitTypeDef Gpio = {
				.GPIO_Pin = Pin,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_Speed = GPIO_High_Speed,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
		};
		static const uint8_t GpioAf = GPIO_AF_SPI1;

		static void init();
	}

	namespace Dma
	{
		static DMA_TypeDef * const Dma = DMA2;
		static const uint32_t DmaRcc = RCC_AHB1Periph_DMA2;
		static DMA_Stream_TypeDef * const Stream = DMA2_Stream3;

		static const DMA_InitTypeDef Init = {
				.DMA_Channel = DMA_Channel_3,
				.DMA_PeripheralBaseAddr = reinterpret_cast<uint32_t>(&Spi::Spi->DR),
				.DMA_Memory0BaseAddr = reinterpret_cast<uint32_t>(FrameBuffer::s_frameBuffer),
				.DMA_DIR = DMA_DIR_MemoryToPeripheral,
				.DMA_BufferSize = FrameBuffer::Width,
				.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_Low,
				.DMA_FIFOMode = DMA_FIFOMode_Disable,
				.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull,
				.DMA_MemoryBurst = DMA_MemoryBurst_Single,
				.DMA_PeripheralBurst = DMA_PeripheralBurst_Single
		};

		static const NVIC_InitTypeDef Nvic = {
				.NVIC_IRQChannel = DMA2_Stream3_IRQn,
				.NVIC_IRQChannelPreemptionPriority = 0,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
		};

		static void init();
	}

	namespace FrameBuffer
	{
		void init()
		{
			Spi::init();
			Dma::init();
		}

		void sendRow()
		{
			Dma::Stream->CR |= DMA_SxCR_EN;
		}

		volatile uint8_t *buffer()
		{
			return reinterpret_cast<volatile uint8_t *>(s_frameBuffer);
		}
	}

	namespace Spi
	{
		static void init()
		{
			RCC_APB2PeriphClockCmd(SpiRcc, ENABLE);
			RCC_AHB1PeriphClockCmd(PortRcc, ENABLE);

			SPI_Cmd(Spi::Spi, DISABLE);

			GPIO_Init(Port, &Gpio);
			GPIO_PinAFConfig(Port, PinSrc, GpioAf);

			SPI_Init(Spi, &Init);

			SPI_CalculateCRC(Spi, DISABLE);

			SPI_Cmd(Spi, ENABLE);

			SPI_DMACmd(Spi, SPI_DMAReq_Tx, ENABLE);
		}
	}

	namespace Dma
	{
		static void init()
		{
			RCC_AHB1PeriphClockCmd(DmaRcc, ENABLE);

			DMA_Cmd(Stream, DISABLE);

			DMA_Init(Stream, &Init);

			NVIC_Init(&Nvic);

			DMA_ITConfig(Stream, DMA_IT_TC, ENABLE);
		}
	}
}

extern "C" void DMA2_Stream3_IRQHandler(void)
{
	static int row = 0;

	Vga::Dma::Dma->LIFCR = DMA_IT_TCIF3;
	Vga::Dma::Stream->CR &= ~DMA_SxCR_EN;
	Vga::Dma::Stream->NDTR = Vga::FrameBuffer::Width;

	Vga::setHBlank();

#if VGA_FB_SCALE_DOUBLE
	if(row & 1) {
		Vga::Dma::Stream->M0AR += Vga::FrameBuffer::WidthBytes;
	}

	if(row++ == Vga::FrameBuffer::Height << 1) {
		Vga::Dma::Stream->M0AR = reinterpret_cast<uint32_t>(Vga::FrameBuffer::s_frameBuffer);

		Vga::setVBlank();

		row = 0;
	}
#else
	if(row++ == Vga::FrameBuffer::Height) {
		Vga::Dma::Stream->M0AR = reinterpret_cast<uint32_t>(Vga::FrameBuffer::s_frameBuffer);

		Vga::setVBlank();

		row = 0;
	}
	else
		Vga::Dma::Stream->M0AR += Vga::FrameBuffer::WidthBytes;

#endif
}

