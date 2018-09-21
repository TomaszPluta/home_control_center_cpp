/*
 * __rfm12b.c
 *
 *  Created on: 02.09.2018
 *      Author: Tomasz Pluta
 */

#include "__rfm12b.h"
#include "__rfm12b_platform.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"


/* SPI CS LOW and High should be declared in platform layer file
#define NSEL_RFM12_LOW
#define NSEL_RFM12_HIGH
*/

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

void Rrm12bObjInit (volatile rfm12bObj_t * rfm12bObj, uint8_t module_addr){
	memset(rfm12bObj, 0, sizeof(rfm12bObj_t));
	rfm12bObj->module_addr = module_addr;
	rfm12bObj->state = receive;
}

void Rfm12bSendByte(uint8_t byte)
{
	uint16_t status;
	uint16_t cmdAndData = 0xB800;
	cmdAndData |= byte;

	while(!status)//
	{
		status = Rfm12bWriteCmd(0x0000);
		status = status & 0x8000;
	}

	Rfm12bWriteCmd(cmdAndData);
}




static void rfSend(uint8_t data)
{
	uint16_t temp=0xB800;
	uint16_t status=0x0000;
	temp|=data;

	if (  !status )
	{
		//status = RFM12B_RDSTATUS();
		status = Rfm12bWriteCmd(0x0000);
		status = status & 0x8000;
	}

	Rfm12bWriteCmd(temp);
}



//void Rfm12bClearBuff (rfm12bBuff_t * rfm12bBuff){
//	memset(rfm12bBuff, 0, sizeof (rfm12bBuff_t));
//}

void Rfm12bStartSending (volatile rfm12bObj_t * rfm12b, uint8_t *data, uint8_t dataNb, uint8_t toAddr){

	rfm12b->txBuff.data[0] = 0xAA;
	rfm12b->txBuff.data[1] = 0x2D;
	rfm12b->txBuff.data[2] = 0xD4;
	rfm12b->txBuff.data[3] = dataNb;
	rfm12b->txBuff.data[4] = toAddr;
	rfm12b->txBuff.data[5] = rfm12b->module_addr;
	memcpy((void*)&rfm12b->txBuff.data[HEADER_SIZE], data, dataNb);
	rfm12b->txBuff.pos =0;
	rfm12b->txBuff.dataNb = dataNb + RFM12_PREMBLE_LEN;
    rfm12bSwitchTx();
    rfm12b->state = transmit;
	rfSend(0xAA);
}



static void Rfm12bTranssmitSeqByte(volatile rfm12bBuff_t * txBuff){
	uint16_t cmd = 0xB800;
	uint8_t data = txBuff->data[txBuff->pos++];
	Rfm12bWriteCmd(cmd | data);
}

void Rfm12bMantainSending(volatile rfm12bObj_t * rfm12b){
	 if (rfm12b->txBuff.pos < rfm12b->txBuff.dataNb){
		 Rfm12bTranssmitSeqByte(&rfm12b->txBuff);
	 } else{
		 rfm12b->state = receive;
		 rfm12bSwitchRx();
	 }
}

static void Rfm12bMoveDataToCompletedBuff(volatile rfm12bObj_t * rfm12b){
	memset(&rfm12b->completedRxBuff, 0, sizeof (rfm12bBuff_t));
	memcpy(rfm12b->completedRxBuff.data,  &rfm12b->rxBuff.data[HEADER_SIZE], rfm12b->rxBuff.dataNb);
	rfm12b->completedRxBuff.dataNb = rfm12b->rxBuff.dataNb;
	rfm12b->rxTOAddr = rfm12b->rxBuff.data[ADDR_TO_POS];
	rfm12b->rxFromAddr = rfm12b->rxBuff.data[ADDR_FROM_POS];
	memset(&rfm12b->rxBuff, 0, sizeof (rfm12bBuff_t));
}

static void Rfm12bresetRx(volatile rfm12bObj_t * rfm12b){
	rfm12b->rxBuff.pos = 0;
	rfm12bFifoReset();
}


/*+++++++++++++++++++++Frame++++++++++++++++++++++++++*/
/* [BYTE_NUMBER] [ADDR_RECEIVER] [ADDR_SENDER] [DATA] */
/*+++++++++++++++++++++Frame++++++++++++++++++++++++++*/

void Rfm12bMantainreceiving(volatile rfm12bObj_t * rfm12b){
	uint8_t rxByte = rfm12bReadFifo();
	if (rfm12b->rxBuff.pos < RFM12_MAX_FRAME_SIZE){
		rfm12b->rxBuff.data[rfm12b->rxBuff.pos++] = rxByte;
		rfm12b->rxBuff.dataNb =  rfm12b->rxBuff.data[BYTE_NB_POS];
		if (rfm12b->rxBuff.pos == rfm12b->rxBuff.dataNb){
			Rfm12bMoveDataToCompletedBuff(rfm12b);
			Rfm12bresetRx(rfm12b);
			GPIOC->ODR ^= GPIO_Pin_13;//
		}
	} else {
		Rfm12bresetRx(rfm12b);
	}
}


void Rfm12bIrqCallback (volatile rfm12bObj_t * rfm12b){
	uint16_t status = Rfm12bWriteCmd(0x0000);

	if (status & RFM12_STATUS_FFIT ){
		if (rfm12b->state == transmit){
			Rfm12bMantainSending(rfm12b);
		}
		else{
			Rfm12bMantainreceiving(rfm12b);
		}
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

