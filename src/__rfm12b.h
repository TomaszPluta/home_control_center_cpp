/*
 * __rfm12b.h
 *
 *  Created on: 02.09.2018
 *      Author: Tomasz Pluta
 */


#ifdef __cplusplus
extern "C" {
#endif


#include "stdint.h"

#ifndef RFM12B_H_
#define RFM12B_H_



#define RFM12_STATUS_RGIT 	0x8000
#define RFM12_STATUS_FFIT 	0x8000
#define RFM12_STATUS_POR 	0x4000
#define RFM12_STATUS_RGUR 	0x2000
#define RFM12_STATUS_FFOV 	0x2000
#define RFM12_STATUS_WKUP 	0x1000
#define RFM12_STATUS_EXT 	0x0800
#define RFM12_STATUS_LBD 	0x0400
#define RFM12_STATUS_FFEM 	0x0200
#define RFM12_STATUS_ATS 	0x0100
#define RFM12_STATUS_RSSI 	0x0100
#define RFM12_STATUS_DQD 	0x0080
#define RFM12_STATUS_CRL 	0x0040
#define RFM12_STATUS_ATGL	 0x0020



void Rfm12bInitNode();
void rfm12bSwitchTx(void);
void rfm12bSwitchRx(void);

void Rfm12bInit(void);
void Rfm12bSendByte(uint8_t byte);
void Rfm12bSendBuff(uint8_t *buf, uint8_t bytesNb);
void rfm12bFifoReset(void);
void rfm12bWaitForData(void);
uint8_t rfm12bRecv(void);

uint8_t rfm12bReadFifo(void);


void RF12_TXPACKET(uint8_t *buff, uint8_t bytesNb);

#ifdef __cplusplus
}
#endif

#endif /* RFM12B_H_ */
