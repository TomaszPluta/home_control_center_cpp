/*
 * __rfm12b.c
 *
 *  Created on: 02.09.2018
 *      Author: Tomasz Pluta
 */

#include "__rfm12b.h"
#include "__rfm12b_platform.h"


/* SPI CS LOW and High should be declared in platform layer file
#define NSEL_RFM12_LOW
#define NSEL_RFM12_HIGH
*/


 __attribute__((weak)) void Rfm12bSpiInit(void) {
	 ; /* Function for SPI init should be implemented in platform layer file*/
 }

 __attribute__((weak)) void WriteCmd( uint16_t CMD )
{
	 ; /* Function for sending 16 bits by SPI should be implemented in platform layer file*/
}

void Rfm12bInit() {
  Rfm12bSpiInit();
  Rfm12bWriteCmd(0x0000);
  Rfm12bWriteCmd(0x80E7); //EL,EF,868band,12.0pF
  Rfm12bWriteCmd(0x8208); //no tx/rx
  Rfm12bWriteCmd(0xA640); //frequency select
  Rfm12bWriteCmd(0xC648); //4,8kbps
  Rfm12bWriteCmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm
  Rfm12bWriteCmd(0xC2AC); //AL,!ml,DIG,DQD4
  Rfm12bWriteCmd(0xCA81); //FIFO8,SYNC,!ff,DR
  Rfm12bWriteCmd(0xCED4); //SYNC=2DD4G
  Rfm12bWriteCmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN
  Rfm12bWriteCmd(0x9850); //!mp,90kHz,MAX OUT
  Rfm12bWriteCmd(0xCC17); //OB1COB0, LPX,IddyCDDITCBW0
  Rfm12bWriteCmd(0xE000); //NOT USE
  Rfm12bWriteCmd(0xC800); //NOT USE
  Rfm12bWriteCmd(0xC040); //1.66MHz,2.2V
}



void Rfm12bSendByte(uint8_t byte)
{
	uint16_t status;
	uint16_t cmdAndData = 0xB800;
	cmdAndData |= byte;

	while(!status)// brakuje chyba tu inicjalizacji statusu
	{
		status = Rfm12bWriteCmd(0x0000);
		status = status & 0x8000;
	}

	Rfm12bWriteCmd(cmdAndData);
}




void rfSend(unsigned char data)
{
	uint16_t temp=0xB800, status=0x0000;
	temp|=data;

	while(  !status )
	{
		//status = RFM12B_RDSTATUS();
		status = Rfm12bWriteCmd(0x0000);
		status = status & 0x8000;
	}

	Rfm12bWriteCmd(temp);
}





void Rfm12bSendBuff(uint8_t *buff, uint8_t bytesNb)
{
//	dziala tylko z komenda rfSend zaimportowana z innego modulu
	WriteCmd(0x0000);
	rfSend(0xAA); // PREAMBLE
	rfSend(0xAA);
	rfSend(0xAA);
	rfSend(0x2D); // SYNC
	rfSend(0xD4);

	for (uint16_t i=0; i< bytesNb; i++)
	{
		rfSend(buff[i]);		//wyslij kolejny znak z bufora
	}
}



void RF12_SCAN(void)
{
	Rfm12bWriteCmd(0x0000);
	Rfm12bWriteCmd(0x82C9);
	Rfm12bWriteCmd(0xCA81);
	Rfm12bWriteCmd(0xCA83);
}


void RF12_TXPACKET(uint8_t *buff, uint8_t bytesNb)
{

	char i;
	WriteCmd(0x0000);
	rfSend(0x0000);//read status register
	rfSend(0x8239);//!er,!ebb,ET,ES,EX,!eb,!ew,DC
	rfSend(0xAA);//PREAMBLE
	rfSend(0xAA);//PREAMBLE
	rfSend(0xAA);//PREAMBLE
	rfSend(0x2D);//SYNC HI BYTE
	rfSend(0xD4);//SYNC LOW BYTE
	for(i = 0; i < bytesNb; i++)
	{
	//	Rfm12bSendByte(buff[i]);
		rfSend(buff[i]);
	}
	rfSend(0xAA);
	rfSend(0xAA);
	RF12_SCAN();
}







void rfm12bFifoReset() {
	Rfm12bWriteCmd(0xCA81);
	Rfm12bWriteCmd(0xCA83);
}


void rfm12bWaitForData() {
	unsigned int status;
	while(1) {
		status = Rfm12bWriteCmd(0x0000);
		if ( (status&0x8000) ) {
			return;
		}
	}
}


unsigned char rfm12bRecv() {
	uint8_t rxByte;
	while(1) {
		rxByte = Rfm12bWriteCmd(0x0000);
		if ( (rxByte&0x8000) ) {
			rxByte = Rfm12bWriteCmd(0xB000);
			return (rxByte&0x00FF);
		}
	}
}


uint8_t rfm12bReadFifo(void){
	uint8_t rxByte = Rfm12bWriteCmd(0xB000);
	return (rxByte&0x00FF);
}

void rfm12bSwitchRx(void)
{
	Rfm12bWriteCmd(0x8299);
}

void rfm12bSwitchTx(void)
{
	Rfm12bWriteCmd(0x8239);
}

