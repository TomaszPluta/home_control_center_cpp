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


/* SPI CS LOW and High should be declared in platform layer file */
//#define NSEL_RFM12_LOW
//#define NSEL_RFM12_HIGH


 __attribute__((weak)) void Rfm12bSpiInit(void);
 __attribute__((weak)) uint16_t Rfm12bWriteCmd( uint16_t cmd );

void Rfm12bInit(void);

void Rfm12bSendByte(uint8_t byte);
void Rfm12bSendBuff(uint8_t *buf, uint8_t bytesNb);
void rfm12bFifoReset(void);
void rfm12bWaitForData(void);
uint8_t rfm12bRecv(void);


#ifdef __cplusplus
}
#endif

#endif /* RFM12B_H_ */
