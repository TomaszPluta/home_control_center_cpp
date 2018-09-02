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
#include "stm32f10x.h"

#ifndef RFM12B_H_
#define RFM12B_H_


#define NSEL_RFM12_LOW              GPIO_ResetBits(GPIOB, GPIO_Pin_1);
#define NSEL_RFM12_HIGH             GPIO_SetBits(GPIOB, GPIO_Pin_1);


void Rfm12bSpiInit(void);
uint16_t Rfm12bWriteCmd( uint16_t cmd );



#ifdef __cplusplus
}
#endif

#endif /* RFM12B_H_ */
