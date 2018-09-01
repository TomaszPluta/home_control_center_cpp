/*
 * rfm12bAM.c
 *
 *  Created on: 01.09.2018
 *      Author: tomek
 */
#include "stdint.h"
#include "stm32f10x.h"


#define RFM12B_NSEL_L()              GPIO_ResetBits(GPIOB, GPIO_Pin_1);
#define RFM12B_NSEL_H()              GPIO_SetBits(GPIOB, GPIO_Pin_1);

#define NSEL_RFM12_DOWN				 RFM12B_NSEL_L()
#define NSEL_RFM12_UP				 RFM12B_NSEL_H()



void writeCmd( uint16_t CMD )
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




void rfInitAM() {
  writeCmd(0x80E7); //EL,EF,868band,12.0pF
  writeCmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
  writeCmd(0xA640); //frequency select
  writeCmd(0xC648); //4,8kbps
  writeCmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm
  writeCmd(0xC2AC); //AL,!ml,DIG,DQD4
  writeCmd(0xCA81); //FIFO8,SYNC,!ff,DR
  writeCmd(0xCED4); //SYNC=2DD4G
  writeCmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN
  writeCmd(0x9850); //!mp,90kHz,MAX OUT
  writeCmd(0xCC17); //OB1COB0, LPX,IddyCDDITCBW0
  writeCmd(0xE000); //NOT USE
  writeCmd(0xC800); //NOT USE
  writeCmd(0xC040); //1.66MHz,2.2V
}

void rfSend(unsigned char data)
{
	uint16_t temp=0xB800, status=0x0000;
	temp|=data;

	while(  !status )
	{
		status = RFM12B_RDSTATUS();
		status = status & 0x8000;
	}

	RFM12B_WriteCMD(temp);
}

void rfSendString(char *s)  //wyslij string
{
	int n=0;
    while (s[n])
	{
      rfInitAM();

 	   writeCmd(0x0000);
 	   rfSend(0xAA); // PREAMBLE
 	   rfSend(0xAA);
 	   rfSend(0xAA);
 	   rfSend(0x2D); // SYNC
 	   rfSend(0xD4);

   	   rfSend(s[n]);		//wyslij kolejny znak z bufora
	  //
   	   rfSend(0xAA); // DUMMY BYTES
 	   rfSend(0xAA);
       rfSend(0xAA);
	  n++;
	//  WAIT_NIRQ_LOW();///////////////////////////////

	  _delay_ms(2);

	}
}
