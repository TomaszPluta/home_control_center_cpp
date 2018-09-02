/*
 * rfm12bMCK.c
 *
 *  Created on: 31.08.2018
 *      Author: tomek
 */

#include <stdint.h>
#include "stm32f10x.h"


#define RFM12B_NSEL_L()              GPIO_ResetBits(GPIOB, GPIO_Pin_1);
#define RFM12B_NSEL_H()              GPIO_SetBits(GPIOB, GPIO_Pin_1);

#define NSEL_RFM12_DOWN				 RFM12B_NSEL_L()
#define NSEL_RFM12_UP				 RFM12B_NSEL_H()


uint16_t RFM12B_RDSTATUS(void);


void RFM12B_WriteCMD( uint16_t CMD )
{


	NSEL_RFM12_DOWN;
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, CMD);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	SPI_I2S_ReceiveData(SPI1);

	NSEL_RFM12_UP;

	return;

}

void RFM12B_TXInit( void )
{
	RFM12B_WriteCMD(0x80D8);//enable register,433MHz,12.5pF
	RFM12B_WriteCMD(0x8208);//Turn on crystal,!PA
	RFM12B_WriteCMD(0xA640);//
	RFM12B_WriteCMD(0xC647);//
	RFM12B_WriteCMD(0XCC77);//
	RFM12B_WriteCMD(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
	RFM12B_WriteCMD(0xC2AC);
	RFM12B_WriteCMD(0xCA80);
	RFM12B_WriteCMD(0xCA83);//FIFO8,SYNC,
	RFM12B_WriteCMD(0xC49B);
	RFM12B_WriteCMD(0x9850);//!mp,9810=30kHz,MAX OUT
	RFM12B_WriteCMD(0xE000);//NOT USE
	RFM12B_WriteCMD(0xC80E);//NOT USE
	RFM12B_WriteCMD(0xC000);//1.0MHz,2.2V

	RFM12B_WriteCMD(0x8228); //OPEN PA
	_delay_ms( 1 );
	RFM12B_WriteCMD(0x8238);

}

void RFM12B_WriteFSKbyte( uint8_t DATA )
{

	uint16_t temp=0xB800, status=0x0000;
	temp|=DATA;

	while(  !status )
	{
		status = RFM12B_RDSTATUS();
		status = status & 0x8000;
	}

	RFM12B_WriteCMD(temp);


}

void RFM12B_SendData(uint8_t *TXbuf, uint8_t nr_of_bytes)
{
	uint8_t i =0;

	RFM12B_WriteFSKbyte( 0xAA ); // send Preamble
	RFM12B_WriteFSKbyte( 0xAA ); // send preamble
	RFM12B_WriteFSKbyte( 0xAA ); // send preamble
	RFM12B_WriteFSKbyte( 0x2D ); // send sync word
	RFM12B_WriteFSKbyte( 0xD4 ); // send sync word

	RFM12B_WriteFSKbyte( nr_of_bytes );

	for (i = 0; i < nr_of_bytes; i++)
	{
		RFM12B_WriteFSKbyte( TXbuf[i] );
	}
	RFM12B_WriteFSKbyte( 0xAA ); // dummy

}

void RFM12B_RXInit( void )
{
	RFM12B_WriteCMD(0x80D8);//enable register,433MHz,12.5pF
	RFM12B_WriteCMD(0x82D8);//enable receive,!PA
	RFM12B_WriteCMD(0xA640);//
	RFM12B_WriteCMD(0xC647);//
	RFM12B_WriteCMD(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
	RFM12B_WriteCMD(0xC2AC);
	RFM12B_WriteCMD(0XCC77);//
	RFM12B_WriteCMD(0xCA80);
	RFM12B_WriteCMD(0xCA83);//FIFO8,SYNC,
	RFM12B_WriteCMD(0xC49B);
	RFM12B_WriteCMD(0x9850);//!mp,9810=30kHz,MAX OUT
	RFM12B_WriteCMD(0xE000);//NOT USE
	RFM12B_WriteCMD(0xC800);//NOT USE
	RFM12B_WriteCMD(0xC000);//1.0MHz,2.2V

}

uint8_t RFM12B_RDFIFO(void)
{
	uint8_t i, Result;

	NSEL_RFM12_DOWN;

	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, 0xB000);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	Result = SPI_I2S_ReceiveData(SPI1);


	NSEL_RFM12_UP;

	return(Result & 0xff);

}




uint16_t RFM12B_RDSTATUS(void)
{

	uint16_t Result;

	NSEL_RFM12_DOWN;

	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, 0x0000);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	Result = SPI_I2S_ReceiveData(SPI1);

	NSEL_RFM12_UP;

	return(Result);
}



void RFM12B_SWITCH_TO_RX(void)
{
	//RFM12B_WriteCMD(0x8208);
	RFM12B_WriteCMD(0x82D8);//enable receive,!PA
}

void RFM12B_SWITCH_TO_TX(void)
{
	RFM12B_WriteCMD(0x8228); //OPEN PA
	_delay_ms( 1 );
	RFM12B_WriteCMD(0x8238);
}
