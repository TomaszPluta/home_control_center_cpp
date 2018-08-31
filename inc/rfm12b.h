
#ifdef __cplusplus
extern "C" {
#endif



/*
 * rfm12b.c
 *
 *  Created on: 30.08.2018
 *      Author: tomek
 */

#ifndef RFM12B_C_
#define RFM12B_C_


void RFM12B_GPIO_Init(void);

uint16_t SpiTransfer(uint16_t data);

uint16_t Rfm_xmit(uint16_t data);

void Rfm12bTest (void);

void Rfm12bRx (void);


void Rfm12bSend (void);



#ifdef __cplusplus
}
#endif

#endif /* RFM12B_C_ */
