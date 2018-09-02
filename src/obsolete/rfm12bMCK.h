/*
 * rfm12bMCK.h
 *
 *  Created on: 31.08.2018
 *      Author: tomek
 */

#ifndef RFM12BMCK_H_
#define RFM12BMCK_H_


#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>
#include "stm32f10x.h"


#define RFM12B_NSEL_L()              GPIO_ResetBits(GPIOB, GPIO_Pin_1);
#define RFM12B_NSEL_H()              GPIO_SetBits(GPIOB, GPIO_Pin_1);

#define NSEL_RFM12_DOWN				 RFM12B_NSEL_L()
#define NSEL_RFM12_UP				 RFM12B_NSEL_H()




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





uint16_t RFM12B_RDSTATUS(void);


void RFM12B_WriteCMD( uint16_t CMD );

void RFM12B_TXInit( void );

void RFM12B_WriteFSKbyte( uint8_t DATA );

void RFM12B_SendData(uint8_t *TXbuf, uint8_t nr_of_bytes);
void RFM12B_RXInit( void );

uint8_t RFM12B_RDFIFO(void);


uint16_t RFM12B_RDSTATUS(void);

void RFM12B_SWITCH_TO_RX(void);

void RFM12B_SWITCH_TO_TX(void);


#ifdef __cplusplus
}
#endif

#endif /* RFM12BMCK_H_ */
